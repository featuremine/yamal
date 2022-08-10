/******************************************************************************

        COPYRIGHT (c) 2022 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

/**
 * @file component.c
 * @date 14 Jul 2022
 * @brief File contains C implementation of fmc component
 * @see http://www.featuremine.com
 */

#include <fmc/component.h>
#include <fmc/error.h>
#include <fmc/extension.h>
#include <fmc/files.h>
#include <fmc/math.h>
#include <fmc/platform.h>
#include <fmc/reactor.h>
#include <fmc/string.h>
#include <stdlib.h> // calloc() getenv()
#include <string.h> // memcpy() strtok()
#include <uthash/utheap.h>
#include <uthash/utlist.h>
#include <stdarg.h>

#if defined(FMC_SYS_UNIX)
#define FMC_MOD_SEARCHPATH_CUR ""
#define FMC_MOD_SEARCHPATH_USRLOCAL ".local/lib/yamal/modules"
#define FMC_MOD_SEARCHPATH_SYSLOCAL "/usr/local/lib/yamal/modules"
#define FMC_MOD_SEARCHPATH_ENV "YAMALCOMPPATH"
#define FMC_MOD_SEARCHPATH_ENV_SEP ":"
#if defined(FMC_SYS_LINUX)
#define FMC_LIB_SUFFIX ".so"
#elif defined(FMC_SYS_MACH)
#define FMC_LIB_SUFFIX ".dylib"
#endif
#else
#define FMC_MOD_SEARCHPATH_ENV_SEP ";"
#error "Unsupported operating system"
#endif

static void components_del(struct fmc_component_list **comps) {
  struct fmc_component_list *head = *comps;
  struct fmc_component_list *item;
  struct fmc_component_list *tmp;
  DL_FOREACH_SAFE(head, item, tmp) {
    DL_DELETE(head, item);
    item->comp->_vt->tp_del(item->comp);
    free(item);
  }
  *comps = NULL;
}

static void component_types_del(struct fmc_component_type **types) {
  struct fmc_component_type *head = *types;
  struct fmc_component_type *item;
  struct fmc_component_type *tmp;
  DL_FOREACH_SAFE(head, item, tmp) {
    DL_DELETE(head, item);
    components_del(&item->comps);
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

static int size_t_less(const void *a, const void *b) {
  return FMC_SIZE_T_PTR_LESS(a, b);
}

static int sched_item_less(const void *a, const void *b) {
  struct sched_item *_a = (struct sched_item *)a;
  struct sched_item *_b = (struct sched_item *)b;
  return (int)fmc_time64_less(_a->t, _b->t);
}

static void reactor_queue_v1(struct fmc_reactor_ctx *ctx) {
  utheap_push(&ctx->reactor->toqueue, &ctx->idx, size_t_less);
}

static void reactor_schedule_v1(struct fmc_reactor_ctx *ctx,
                                fmc_time64_t time) {
  struct sched_item item = {.idx = ctx->idx, .t = time};
  utheap_push(&ctx->reactor->sched, &item, sched_item_less);
}

static void reactor_on_exec_v1(struct fmc_reactor_ctx *ctx,
                               fmc_reactor_exec_clbck cl) {
  ctx->exec = cl;
}

void reactor_set_error_v1(struct fmc_reactor_ctx *ctx, const char *fmt, ...) {
  va_list _args1;
  va_start(_args1, fmt);
  if (fmt) {
    va_list _args2;
    va_copy(_args2, _args1);
    int _size = vsnprintf(NULL, 0, fmt, _args1) + 1;
    char _buf[_size];
    vsnprintf(_buf, _size, fmt, _args2);
    va_end(_args2);
    fmc_error_init(&ctx->err, FMC_ERROR_CUSTOM, _buf);
  } else {
    fmc_error_init(&ctx->err, va_arg(_args1, FMC_ERROR_CODE), NULL);
  }
  va_end(_args1);
}

void reactor_on_shutdown_v1(struct fmc_reactor_ctx *ctx, fmc_reactor_shutdown_clbck cl) {
  ctx->shutdown = cl;
}

void reactor_finished_v1(struct fmc_reactor_ctx *ctx) {
  if (!ctx->finishing) return;
  ctx->finishing = false;
  --ctx->reactor->finishing;
}

void reactor_on_dep_v1(struct fmc_reactor_ctx *ctx, fmc_reactor_dep_clbck cl) {
  ctx->dep_upd = cl;
}

void reactor_add_output_v1(struct fmc_reactor_ctx *ctx, const char *type, const char *name) {
  char **tmp = (char**)realloc(ctx->out_tps, ctx->nouts + 1 * sizeof(*tmp));
  if (!tmp) goto cleanup;
  tmp[ctx->nouts++] = strdup(type);
  ctx->out_tps = tmp;
cleanup:
  reactor_set_error_v1(ctx, NULL, FMC_ERROR_MEMORY);
}

void reactor_notify_v1(struct fmc_reactor_ctx *ctx, int idx, struct fmc_shmem mem) {
  //TODO: implement
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
    .notify = reactor_notify_v1
};

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

static void component_path_list_del(fmc_component_path_list_t **phead) {
  if (!*phead)
    return;
  fmc_component_path_list_t *p;
  fmc_component_path_list_t *ptmp;
  DL_FOREACH_SAFE(*phead, p, ptmp) {
    DL_DELETE(*phead, p);
    free(p);
  }
}

static void component_path_list_add(fmc_component_path_list_t **phead,
                                    const char *path, fmc_error_t **error) {
  fmc_component_path_list_t *p =
      (fmc_component_path_list_t *)calloc(1, sizeof(*p) + strlen(path) + 1);
  if (p) {
    strcpy(p->path, path);
    DL_APPEND(*phead, p);
  } else {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return;
  }
}

static void component_path_list_set(fmc_component_path_list_t **head,
                                    const char **paths, fmc_error_t **error) {
  for (unsigned int i = 0; paths && paths[i]; ++i) {
    component_path_list_add(head, paths[i], error);
    if (*error) {
      component_path_list_del(head);
      return;
    }
  }
}

void fmc_component_sys_paths_set(struct fmc_component_sys *sys,
                                 const char **paths, fmc_error_t **error) {
  fmc_error_clear(error);
  fmc_component_path_list_t *tmpls = NULL;
  component_path_list_set(&tmpls, paths, error);
  if (!*error) {
    fmc_component_path_list_t *tmpls2 = sys->search_paths;
    sys->search_paths = tmpls;
    tmpls = tmpls2;
  }
  component_path_list_del(&tmpls);
}

void fmc_component_sys_paths_set_default(struct fmc_component_sys *sys,
                                         fmc_error_t **error) {
  fmc_error_clear(error);
  fmc_component_path_list_t *tmpls = NULL;

  char *tmp = getenv("HOME");
  int psz = fmc_path_join(NULL, 0, tmp, FMC_MOD_SEARCHPATH_USRLOCAL) + 1;
  char home_path[psz];
  fmc_path_join(home_path, psz, tmp, FMC_MOD_SEARCHPATH_USRLOCAL);

  const char *defaults[] = {FMC_MOD_SEARCHPATH_CUR, home_path,
                            FMC_MOD_SEARCHPATH_SYSLOCAL, NULL};

  component_path_list_set(&tmpls, defaults, error);
  if (*error)
    goto cleanup;

  tmp = getenv(FMC_MOD_SEARCHPATH_ENV);
  if (tmp) {
    char ycpaths[strlen(tmp) + 1];
    strcpy(ycpaths, tmp);
    char *found;
    tmp = ycpaths;
    while ((found = strsep(&tmp, FMC_MOD_SEARCHPATH_ENV_SEP))) {
      component_path_list_add(&tmpls, found, error);
      if (*error)
        goto cleanup;
    }
  }
  fmc_component_path_list_t *tmpls2 = sys->search_paths;
  sys->search_paths = tmpls;
  tmpls = tmpls2;
cleanup:
  component_path_list_del(&tmpls);
}

void fmc_component_sys_paths_add(struct fmc_component_sys *sys,
                                 const char *path, fmc_error_t **error) {
  fmc_error_clear(error);
  if (path) {
    component_path_list_add(&sys->search_paths, path, error);
  }
}

fmc_component_path_list_t *
fmc_component_sys_paths_get(struct fmc_component_sys *sys) {
  return sys->search_paths;
}

void fmc_component_module_destroy(struct fmc_component_module *mod) {
  if (mod->name)
    free(mod->name);
  if (mod->file)
    free(mod->file);
  if (mod->handle)
    fmc_ext_close(mod->handle);
  fmc_error_destroy(&mod->error);
  component_types_del(&mod->types);
}

static struct fmc_component_module *
mod_load(struct fmc_component_sys *sys, const char *dir, const char *modstr,
         const char *mod_lib, const char *mod_func, fmc_error_t **error) {
  fmc_error_t *err = NULL;
  int psz = fmc_path_join(NULL, 0, dir, mod_lib) + 1;
  char lib_path[psz];
  fmc_path_join(lib_path, psz, dir, mod_lib);

  struct fmc_component_module mod;
  memset(&mod, 0, sizeof(mod));

  mod.handle = fmc_ext_open(lib_path, &err);
  if (err)
    goto cleanup;

  // Check if init function is available
  fmc_component_module_init_func mod_init =
      (fmc_component_module_init_func)fmc_ext_sym(mod.handle, mod_func, &err);
  if (err)
    goto cleanup;

  // append the mod to the system
  fmc_error_init_none(&mod.error);
  mod.sys = sys;
  mod.name = fmc_cstr_new(modstr, error);
  if (*error)
    goto cleanup;
  mod.file = fmc_cstr_new(lib_path, error);
  if (*error)
    goto cleanup;

  fmc_error_reset_none(&mod.error);
  mod_init(&api, &mod);
  if (fmc_error_has(&mod.error)) {
    fmc_error_set(error, "failed to load components %s with error: %s", modstr,
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

struct fmc_component_module *
fmc_component_module_get(struct fmc_component_sys *sys, const char *mod,
                         fmc_error_t **error) {
  fmc_error_clear(error);

  // If the module exists, get it
  struct fmc_component_module *mhead = sys->modules;
  struct fmc_component_module *mitem;
  DL_FOREACH(mhead, mitem) {
    if (!strcmp(mitem->name, mod)) {
      return mitem;
    }
  }

  struct fmc_component_module *ret = NULL;
  char mod_lib[strlen(mod) + strlen(FMC_LIB_SUFFIX) + 1];
  sprintf(mod_lib, "%s%s", mod, FMC_LIB_SUFFIX);

  int pathlen = fmc_path_join(NULL, 0, mod, mod_lib) + 1;
  char mod_lib_2[pathlen];
  fmc_path_join(mod_lib_2, pathlen, mod, mod_lib);

  char mod_func[strlen(FMC_COMPONENT_INIT_FUNC_PREFIX) + strlen(mod) + 1];
  sprintf(mod_func, "%s%s", FMC_COMPONENT_INIT_FUNC_PREFIX, mod);
  fmc_component_path_list_t *head = sys->search_paths;
  fmc_component_path_list_t *item;
  DL_FOREACH(head, item) {
    ret = mod_load(sys, item->path, mod, mod_lib, mod_func, error);
    if (!ret && !(*error)) {
      ret = mod_load(sys, item->path, mod, mod_lib_2, mod_func, error);
    }
    if (ret || *error) {
      break;
    }
  }
  if (!ret && !(*error)) {
    fmc_error_set(error, "component module %s was not found", mod);
  }
  return ret;
}

void fmc_component_module_del(struct fmc_component_module *mod) {
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

struct fmc_component *fmc_component_new(struct fmc_reactor *reactor,
                                        struct fmc_component_type *tp,
                                        struct fmc_cfg_sect_item *cfg,
                                        struct fmc_component_input *inps,
                                        fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_component_list *item = NULL;
  unsigned int in_sz = 0;
  for (; inps && inps[in_sz].comp; ++in_sz) {
  }
  char *in_names[in_sz];

  fmc_cfg_node_spec_check(tp->tp_cfgspec, cfg, error);
  if (*error)
    goto cleanup;

  item = (struct fmc_component_list *)calloc(1, sizeof(*item));
  if (!item)
    goto cleanup;

  // fmc_component_input to array of component names
  for (unsigned int i = 0; i < in_sz; ++i) {
    if (inps[i].comp->_ctx->reactor != reactor) {
      fmc_error_set(
          error, "input component %d of type %s does not have the same reactor",
          i, inps[i].comp->_vt->tp_name);
      goto cleanup;
    }
    if (!inps[i].comp->_ctx->out_tps) {
      fmc_error_set(
          error, "the outputs of the input component %d of type %s are not set",
          i, inps[i].comp->_vt->tp_name);
      goto cleanup;
    }
    unsigned int out_sz = 0;
    for (; inps[i].comp->_ctx->out_tps && inps[i].comp->_ctx->out_tps[out_sz];
         ++out_sz) {
    }
    if (inps[i].idx < 0 || out_sz <= inps[i].idx) {
      fmc_error_set(error, "invalid output index %d of type %s", inps[i].idx,
                    inps[i].comp->_vt->tp_name);
      goto cleanup;
    }
    in_names[i] = inps[i].comp->_ctx->out_tps[inps[i].idx];
  }
  in_names[in_sz] = NULL;

  struct fmc_reactor_ctx ctx;
  fmc_reactor_ctx_init(reactor, &ctx);
  item->comp = tp->tp_new(cfg, &ctx, in_names, error);
  if (*error)
    goto cleanup;
  ctx.comp = item->comp;
  fmc_reactor_ctx_push(&ctx, inps, error); // copy the context
  if (*error)
    goto cleanup;
  item->comp->_vt = tp;
  item->comp->_ctx = reactor->ctxs[reactor->size - 1];
  DL_APPEND(tp->comps, item);
  return item->comp;
cleanup:
  if (!*error)
    fmc_error_set2(error, FMC_ERROR_MEMORY);
  if (item) {
    free(item);
    if (item->comp)
      item->comp->_vt->tp_del(item->comp);
  }
  return NULL;
}

void fmc_component_del(struct fmc_component *comp) {
  struct fmc_component_list *head = comp->_vt->comps;
  struct fmc_component_list *item;
  DL_FOREACH(head, item) {
    if (item->comp == comp) {
      DL_DELETE(comp->_vt->comps, item);
      free(item);
      break;
    }
  }
  comp->_vt->tp_del(comp);
}

void fmc_component_sys_destroy(struct fmc_component_sys *sys) {
  fmc_component_path_list_t *phead = sys->search_paths;
  fmc_component_path_list_t *p;
  fmc_component_path_list_t *ptmp;
  DL_FOREACH_SAFE(phead, p, ptmp) {
    DL_DELETE(phead, p);
    free(p);
  }
  sys->search_paths = NULL;

  // destroy modules: also destroys components of the module
  struct fmc_component_module *modhead = sys->modules;
  struct fmc_component_module *mod;
  struct fmc_component_module *modtmp;
  DL_FOREACH_SAFE(modhead, mod, modtmp) { fmc_component_module_del(mod); }
  sys->modules = NULL;
}
