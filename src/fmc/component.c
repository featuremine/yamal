/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file component.c
 * @date 14 Jul 2022
 * @brief File contains C implementation of fmc component
 * @see http://www.featuremine.com
 */

#define utarray_oom()                                                          \
  do {                                                                         \
    fmc_error_reset(error, FMC_ERROR_MEMORY, NULL);                            \
    goto cleanup;                                                              \
  } while (0)

#include <fmc/component.h>
#include <fmc/error.h>
#include <fmc/extension.h>
#include <fmc/math.h>
#include <fmc/platform.h>
#include <fmc/reactor.h>
#include <fmc/string.h>
#include <stdarg.h>
#include <stdlib.h> // calloc() getenv()
#include <string.h> // memcpy() strtok()
#include <uthash/utarray.h>
#include <uthash/utheap.h>
#include <uthash/utlist.h>

#define FMC_MOD_SEARCHPATH_ENV "YAMALCOMPPATH"

#if defined(FMC_SYS_UNIX)
#define FMC_MOD_SEARCHPATH_DEFAULTPATH "lib/yamal/modules"
#else
#error "Unsupported operating system"
#endif

static void component_types_del(struct fmc_component_type **types) {
  struct fmc_component_type *head = *types;
  struct fmc_component_type *item;
  struct fmc_component_type *tmp;
  DL_FOREACH_SAFE(head, item, tmp) {
    DL_DELETE(head, item);
    free(item);
  }
  *types = NULL;
}

static void components_add_v1(struct fmc_component_module *mod,
                              struct fmc_component_def_v1 *d) {
  for (int i = 0; d && d[i].tp_name; ++i) {
    struct fmc_component_type *tp =
        (struct fmc_component_type *)calloc(1, sizeof(*tp));
    if (!tp) {
      component_types_del(&mod->types);
      fmc_error_reset(&mod->error, FMC_ERROR_MEMORY, NULL);
      break;
    }
    memcpy(tp, &d[i], sizeof(d[0]));
    DL_APPEND(mod->types, tp);
  }
}

static void incompatible(struct fmc_component_module *mod, void *unused) {
  fmc_error_reset_sprintf(
      &mod->error, "component API version is higher than the system version");
}

static void reactor_queue_v1(struct fmc_reactor_ctx *ctx) {
  fmc_error_t *error = &ctx->reactor->err;
  utheap_push(&ctx->reactor->toqueue, &ctx->idx, FMC_SIZE_T_PTR_LESS);
cleanup:
  return;
}

static void reactor_schedule_v1(struct fmc_reactor_ctx *ctx,
                                fmc_time64_t time) {
  fmc_error_t *error = &ctx->reactor->err;
  struct sched_item item = {.idx = ctx->idx, .t = time};
  utheap_push(&ctx->reactor->sched, &item, FMC_INT64_T_PTR_LESS);
cleanup:
  return;
}

static void reactor_on_exec_v1(struct fmc_reactor_ctx *ctx,
                               fmc_reactor_exec_clbck cl) {
  ctx->exec = cl;
}

void reactor_set_error_v1(struct fmc_reactor_ctx *ctx, const char *fmt, ...) {
  if (fmt) {
    fmc_error_destroy(&ctx->err);
    FMC_ERROR_FORMAT(&ctx->err, fmt);
  } else {
    va_list _args1;
    va_start(_args1, fmt);
    fmc_error_reset(&ctx->err, va_arg(_args1, FMC_ERROR_CODE), NULL);
    va_end(_args1);
  }
}

#define find_context(lhs, ctx)                                                 \
  ({                                                                           \
    struct fmc_reactor_stop_item *_lhs =                                       \
        ((struct fmc_reactor_stop_item *)(lhs));                               \
    _lhs->idx == (ctx)->idx;                                                   \
  })

void reactor_on_shutdown_v1(struct fmc_reactor_ctx *ctx,
                            fmc_reactor_shutdown_clbck cl) {

  if (!ctx->shutdown && cl) {
    struct fmc_reactor_stop_item *item = calloc(1, sizeof(*item));
    if (!item) {
      reactor_set_error_v1(ctx, NULL, FMC_ERROR_MEMORY);
      goto cleanup;
    }
    item->idx = ctx->idx;
    DL_APPEND(ctx->reactor->stop_list, item);
  } else if (ctx->shutdown && !cl) {
    struct fmc_reactor_stop_item *item = NULL;
    DL_SEARCH(ctx->reactor->stop_list, item, ctx, find_context);
    if (item)
      DL_DELETE(ctx->reactor->stop_list, item);
  }
  ctx->shutdown = cl;
cleanup:
  return;
}

void reactor_finished_v1(struct fmc_reactor_ctx *ctx) {
  ctx->reactor->finishing -= ctx->finishing;
  ctx->finishing = false;
}

void reactor_on_dep_v1(struct fmc_reactor_ctx *ctx, fmc_reactor_dep_clbck cl) {
  ctx->dep_upd = cl;
}

void reactor_add_output_v1(struct fmc_reactor_ctx *ctx, const char *type,
                           const char *name) {
  fmc_error_t *error = &ctx->reactor->err;
  struct fmc_reactor_ctx_out *item =
      (struct fmc_reactor_ctx_out *)calloc(1, sizeof(*item));
  if (!item)
    goto cleanup;
  item->type = strdup(type);
  if (!item->type)
    goto cleanup;
  if (name) {
    item->name = strdup(name);
    if (!item->name)
      goto cleanup;
  }
  DL_APPEND(ctx->out_tps, item);
  utarray_extend_back(&ctx->deps);
  return;
cleanup:
  // TODO: Move this code to a function to avoid repeated code,
  // used also in reactor
  if (item) {
    if (item->type)
      free(item->type);
    if (item->name)
      free(item->name);
    free(item);
  }
  reactor_set_error_v1(ctx, NULL, FMC_ERROR_MEMORY);
}

void reactor_notify_v1(struct fmc_reactor_ctx *ctx, size_t idx,
                       struct fmc_shmem mem) {
  fmc_error_t *error = &ctx->reactor->err;
  UT_array *deps = (UT_array *)utarray_eltptr(&ctx->deps, idx);
  if (!deps) {
    fmc_error_reset_sprintf(
        error,
        "component type %s updating output using incorrect output index %lu",
        ctx->comp->_vt->tp_name, idx);
    goto cleanup;
  }
  size_t ndeps = utarray_len(deps);
  for (size_t i = 0; i < ndeps; ++i) {
    struct fmc_reactor_ctx_dep *dep = utarray_eltptr(deps, i);
    struct fmc_reactor_ctx *dep_ctx = ctx->reactor->ctxs[dep->idx];
    if (dep_ctx->dep_upd) {
      dep_ctx->dep_upd(dep_ctx->comp, dep->inp_idx, mem);
    }
    utheap_push(&ctx->reactor->queued, &dep->idx, FMC_SIZE_T_PTR_LESS);
  }
cleanup:
  return;
}

struct fmc_pool *reactor_get_pool_v1(struct fmc_reactor_ctx *ctx) {
  return &ctx->reactor->pool;
}

static struct fmc_reactor_api_v1 reactor_v1 = {
    .queue = reactor_queue_v1,
    .schedule = reactor_schedule_v1,
    .set_error = reactor_set_error_v1,
    .on_shutdown = reactor_on_shutdown_v1,
    .finished = reactor_finished_v1,
    .on_exec = reactor_on_exec_v1,
    .on_dep = reactor_on_dep_v1,
    .add_output = reactor_add_output_v1,
    .notify = reactor_notify_v1,
    .get_pool = reactor_get_pool_v1};

static struct fmc_component_api api = {
    .reactor_v1 = &reactor_v1,
    .components_add_v1 = components_add_v1,
    .reactor_v2 = NULL,
    .components_add_v2 = incompatible,
    .reactor_v3 = NULL,
    .components_add_v3 = incompatible,
    .reactor_v4 = NULL,
    .components_add_v4 = incompatible,
    .reactor_v5 = NULL,
    .components_add_v5 = incompatible,
    ._zeros = {NULL},
};

void fmc_component_sys_init(struct fmc_component_sys *sys) {
  // important: initialize lists to NULL
  sys->search_paths = NULL;
  sys->modules = NULL;
}

void fmc_component_sys_paths_set_default(struct fmc_component_sys *sys,
                                         fmc_error_t **error) {
  fmc_ext_searchpath_set_default(&sys->search_paths,
                                 FMC_MOD_SEARCHPATH_DEFAULTPATH,
                                 FMC_MOD_SEARCHPATH_ENV, error);
}

void fmc_component_sys_paths_set(struct fmc_component_sys *sys,
                                 const char **paths, fmc_error_t **error) {
  fmc_ext_searchpath_set(&sys->search_paths, paths, error);
}

void fmc_component_sys_paths_add(struct fmc_component_sys *sys,
                                 const char *path, fmc_error_t **error) {
  fmc_error_clear(error);
  if (path) {
    fmc_ext_searchpath_add(&sys->search_paths, path, error);
  }
}

struct fmc_ext_searchpath_t *
fmc_component_sys_paths_get(struct fmc_component_sys *sys) {
  return sys->search_paths;
}

void fmc_component_module_destroy(struct fmc_component_module *mod) {
  free(mod->name);
  fmc_ext_mod_destroy(&mod->mod);
  fmc_error_destroy(&mod->error);
  component_types_del(&mod->types);
}

struct fmc_component_module *
fmc_component_module_get(struct fmc_component_sys *sys, const char *modname,
                         fmc_error_t **error) {
  fmc_error_clear(error);

  // If the module exists, get it
  struct fmc_component_module *mhead = sys->modules;
  struct fmc_component_module *mitem;
  DL_FOREACH(mhead, mitem) {
    if (!strcmp(mitem->name, modname)) {
      return mitem;
    }
  }

  struct fmc_component_module mod;
  mod.sys = sys;
  fmc_error_init_none(&mod.error);
  mod.name = NULL;
  mod.types = NULL;
  mod.prev = NULL;
  mod.next = NULL;

  mod.mod = fmc_ext_mod_load(modname, FMC_COMPONENT_INIT_FUNC_PREFIX,
                             sys->search_paths, error);
  if (*error) {
    goto cleanup;
  }

  mod.name = fmc_cstr_new(modname, error);
  if (*error) {
    goto cleanup;
  }

  ((fmc_component_module_init_func)mod.mod.func)(&api, &mod);
  if (fmc_error_has(&mod.error)) {
    fmc_error_set(error, "failed to load components %s with error: %s", modname,
                  fmc_error_msg(&mod.error));
    goto cleanup;
  }

  struct fmc_component_module *m =
      (struct fmc_component_module *)calloc(1, sizeof(mod));
  if (!m) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    goto cleanup;
  }
  memcpy(m, &mod, sizeof(mod));
  DL_APPEND(sys->modules, m);
  return m;

cleanup:
  fmc_component_module_destroy(&mod);
  return NULL;
}

FMMODFUNC const char *
fmc_component_module_file(struct fmc_component_module *mod) {
  return mod->mod.path;
}

void fmc_component_module_del(struct fmc_component_module *mod) {
  if (!mod)
    return;
  DL_DELETE(mod->sys->modules, mod);
  fmc_component_module_destroy(mod);
  free(mod);
}

struct fmc_component_type *
fmc_component_module_type_get(struct fmc_component_module *mod,
                              const char *comp, fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_component_type *head = mod->types;
  struct fmc_component_type *item;
  DL_FOREACH(head, item) {
    if (!strcmp(item->tp_name, comp)) {
      return item;
    }
  }
  FMC_ERROR_REPORT(error, "Could not find the component type");
  return NULL;
}

#define DL_GET_ELEM(head, idx)                                                 \
  ({                                                                           \
    __typeof__(idx) _count = (idx);                                            \
    __typeof__(head) _el = NULL;                                               \
    DL_FOREACH(head, _el) {                                                    \
      if (!_count--)                                                           \
        break;                                                                 \
    }                                                                          \
    _el;                                                                       \
  })

int inequal_cmp(const void *a, const void *b) {
  return *(const size_t *)a - *(const size_t *)b;
}

int inequal_idx_cmp(const void *a, const void *b) {
  return *(const size_t *)a - ((const struct sched_item *)b)->idx;
}

struct fmc_component *fmc_component_new(struct fmc_reactor *reactor,
                                        struct fmc_component_type *tp,
                                        struct fmc_cfg_sect_item *cfg,
                                        struct fmc_component_input *inps,
                                        fmc_error_t **usr_error) {
  fmc_error_clear(usr_error);
  size_t size_t_curridx = reactor->size;
  fmc_error_t *error = &reactor->err;
  struct fmc_component *comp = NULL;
  unsigned int in_sz = 0;
  for (; inps && inps[in_sz].comp; ++in_sz) {
  }
  char *in_types[in_sz + 1];
  UT_array *updated_deps[in_sz + 1];
  memset(updated_deps, 0, sizeof(updated_deps));
  struct fmc_reactor_ctx *ctx = fmc_reactor_ctx_new(reactor, usr_error);
  if (!ctx)
    goto cleanup;

  fmc_cfg_node_spec_check(tp->tp_cfgspec, cfg, usr_error);
  if (*usr_error)
    goto cleanup;

  // fmc_component_input to array of component names
  for (unsigned int i = 0; i < in_sz; ++i) {
    if (inps[i].comp->_ctx->reactor != reactor) {
      fmc_error_set(
          usr_error,
          "input component %d of type %s does not have the same reactor", i,
          inps[i].comp->_vt->tp_name);
      goto cleanup;
    }
    if (!inps[i].comp->_ctx->out_tps) {
      fmc_error_set(
          usr_error,
          "the outputs of the input component %d of type %s are not set", i,
          inps[i].comp->_vt->tp_name);
      goto cleanup;
    }

    struct fmc_reactor_ctx_out *elem =
        DL_GET_ELEM(inps[i].comp->_ctx->out_tps, inps[i].idx);
    if (!elem) {
      fmc_error_set(usr_error, "invalid output index %d of type %s",
                    inps[i].idx, inps[i].comp->_vt->tp_name);
      goto cleanup;
    }
    in_types[i] = elem->type;
  }
  in_types[in_sz] = NULL;

  comp = tp->tp_new(cfg, ctx, in_types);
  if (fmc_error_has(&ctx->err)) {
    fmc_error_set(usr_error,
                  "failed to create new component of type %s with error: %s",
                  tp->tp_name, fmc_error_msg(&ctx->err));
    goto cleanup;
  }
  comp->_vt = tp;
  comp->_ctx = ctx;
  ctx->comp = comp;
  fmc_reactor_ctx_take(ctx, inps, usr_error); // copy the context
  if (*usr_error)
    goto cleanup;
  for (unsigned int i = 0; i < in_sz; ++i) {
    struct fmc_reactor_ctx *inp_ctx = inps[i].comp->_ctx;
    UT_array *deps = (UT_array *)utarray_eltptr(&inp_ctx->deps, inps[i].idx);
    assert(deps);
    struct fmc_reactor_ctx_dep new_dep;
    new_dep.idx = comp->_ctx->idx;
    new_dep.inp_idx = i;
    utarray_push_back(deps, &new_dep);
    updated_deps[i] = deps;
  }
  return comp;
cleanup : {
  void *val = NULL;
  do {
    val = utarray_find(&ctx->reactor->queued, (const void *)&size_t_curridx,
                       inequal_cmp);
    if (!val)
      break;
    utheap_erase(&ctx->reactor->queued,
                 utarray_eltidx(&ctx->reactor->queued, val),
                 FMC_SIZE_T_PTR_LESS);
  } while (true);
  do {
    val = utarray_find(&ctx->reactor->toqueue, (const void *)&size_t_curridx,
                       inequal_cmp);
    if (!val)
      break;
    utheap_erase(&ctx->reactor->toqueue,
                 utarray_eltidx(&ctx->reactor->toqueue, val),
                 FMC_SIZE_T_PTR_LESS);
  } while (true);
  do {
    val = utarray_find(&ctx->reactor->sched, (const void *)&size_t_curridx,
                       inequal_idx_cmp);
    if (!val)
      break;
    utheap_erase(&ctx->reactor->sched,
                 utarray_eltidx(&ctx->reactor->sched, val),
                 FMC_INT64_T_PTR_LESS);
  } while (true);
}
  if (fmc_error_has(error))
    fmc_error_set(usr_error, fmc_error_msg(error));
  if (comp) {
    for (unsigned int i = 0; updated_deps[i]; ++i) {
      utarray_pop_back(updated_deps[i]);
    }
  }
  fmc_reactor_ctx_del(ctx);
  return NULL;
}

size_t fmc_component_out_idx(struct fmc_component *comp, const char *name,
                             fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_reactor_ctx_out *head = comp->_ctx->out_tps;
  struct fmc_reactor_ctx_out *item;
  size_t idx = 0;
  DL_FOREACH(head, item) {
    if (strcmp(name, item->name) == 0) {
      return idx;
    }
    ++idx;
  }
  fmc_error_set(error, "unable to find output with name %s in component", name);
  return 0;
}

size_t fmc_component_out_sz(struct fmc_component *comp) {
  struct fmc_reactor_ctx_out *item;
  size_t counter = 0;
  DL_COUNT(comp->_ctx->out_tps, item, counter);
  return counter;
}

void fmc_component_sys_destroy(struct fmc_component_sys *sys) {
  fmc_ext_searchpath_del(&sys->search_paths);

  // destroy modules: also destroys components of the module
  struct fmc_component_module *modhead = sys->modules;
  struct fmc_component_module *mod;
  struct fmc_component_module *modtmp;
  DL_FOREACH_SAFE(modhead, mod, modtmp) { fmc_component_module_del(mod); }
  sys->modules = NULL;
}
