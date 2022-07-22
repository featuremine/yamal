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
#include <fmc/platform.h>
#include <fmc/string.h>
#include <fmc/uthash/utlist.h>
#include <stdlib.h> // calloc()
#include <string.h> // memcpy()

#if defined(FMC_SYS_LINUX)
#define FMC_LIB_SUFFIX ".so"
#elif defined(FMC_SYS_MACH)
#define FMC_LIB_SUFFIX ".dylib"
#else
#error "Unsupported operating system"
#endif

static void components_del(struct fmc_component_list **comps) {
  struct fmc_component_list *head = *comps;
  struct fmc_component_list *item;
  struct fmc_component_list *tmp;
  DL_FOREACH_SAFE(head, item, tmp) {
    DL_DELETE(head, item);
    fmc_error_destroy(&item->comp->_err);
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
    memcpy(tp, d, sizeof(*d));
    tp->comps = NULL;
    DL_APPEND(mod->types, tp);
  }
}

static void incompatible(struct fmc_component_module *mod, void *unused) {
  fmc_error_reset_sprintf(
      &mod->error, "component API version is higher than the system version");
}

struct fmc_component_api api = {
    .components_add_v1 = components_add_v1,
    .components_add_v2 = incompatible,
    .components_add_v3 = incompatible,
    .components_add_v4 = incompatible,
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

void fmc_component_sys_paths_set(struct fmc_component_sys *sys,
                                 const char **paths, fmc_error_t **error) {
  fmc_error_clear(error);
  fmc_component_path_list_t *tmpls = NULL;
  for (unsigned int i = 0; paths && paths[i]; ++i) {
    component_path_list_add(&tmpls, paths[i], error);
    if (*error) {
      component_path_list_del(&tmpls);
      return;
    }
  }
  component_path_list_del(&sys->search_paths);
  sys->search_paths = tmpls;
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

struct fmc_component *fmc_component_new(struct fmc_component_type *tp,
                                        struct fmc_cfg_sect_item *cfg,
                                        fmc_error_t **error) {
  fmc_error_clear(error);
  fmc_cfg_node_spec_check(tp->tp_cfgspec, cfg, error);
  if (*error)
    return NULL;

  struct fmc_component_list *item =
      (struct fmc_component_list *)calloc(1, sizeof(*item));
  if (item) {
    item->comp = tp->tp_new(cfg, error);
    if (*error) {
      free(item);
    } else {
      item->comp->_vt = tp;
      fmc_error_init_none(&item->comp->_err);
      DL_APPEND(tp->comps, item);
      return item->comp;
    }
  } else {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
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
  fmc_error_destroy(&comp->_err);
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
