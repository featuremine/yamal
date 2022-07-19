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
#include <fmc/extension.h>
#include <fmc/error.h>
#include <fmc/platform.h>
#include <fmc/files.h>
#include <fmc/string.h>
#include <fmc/uthash/utlist.h>
#include <stdlib.h> // calloc()

#if defined(FMC_SYS_UNIX)
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#if defined(FMC_SYS_LINUX)
#define FMC_LIB_SUFFIX ".so"
#elif defined(FMC_SYS_MACH)
#define FMC_LIB_SUFFIX ".dylib"
#endif
#else
#error "Unsupported operating system"
#endif


void fmc_component_sys_init(struct fmc_component_sys *sys) {
  sys->search_paths = NULL;
  sys->modules = NULL; // important- initialize lists to NULL
}

static void component_path_list_del(fmc_component_path_list_t **phead) {
  if (!*phead) return;
  fmc_component_path_list_t *p;
  fmc_component_path_list_t *ptmp;
  DL_FOREACH_SAFE(*phead,p,ptmp) {
    DL_DELETE(*phead,p);
    free(p);
  }
}

static void component_path_list_add(fmc_component_path_list_t **phead, const char *path, fmc_error_t **error) {
  fmc_component_path_list_t *p = (fmc_component_path_list_t *)calloc(1, sizeof(*p)+strlen(path)+1);
  if(p) {
    strcpy(p->path, path);
    DL_APPEND(*phead, p);
  }
  else {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return;
  }
}

void fmc_component_sys_paths_set(struct fmc_component_sys *sys, const char **paths, fmc_error_t **error) {
  fmc_error_clear(error);
  fmc_component_path_list_t *tmpls = NULL;
  for(unsigned int i = 0; paths && paths[i]; ++i) {
    component_path_list_add(&tmpls, paths[i], error);
    if (*error) {
      component_path_list_del(&tmpls);
      return;
    }
  }
  component_path_list_del(&sys->search_paths);
  sys->search_paths = tmpls;
}

void fmc_component_sys_paths_add(struct fmc_component_sys *sys, const char *path, fmc_error_t **error) {
  fmc_error_clear(error);
  if(path) {
    component_path_list_add(&sys->search_paths, path, error);
  }
}

fmc_component_path_list_t *fmc_component_sys_paths_get(struct fmc_component_sys *sys) {
  return sys->search_paths;
}

void fmc_component_module_destroy(struct fmc_component_module *mod) {
  if (mod->name) free(mod->name);
  if (mod->file) free(mod->file);
  if (mod->handle) fmc_ext_close(mod->handle);
  fmc_component_list_t *head = mod->components;
  fmc_component_list_t *item;
  fmc_component_list_t *tmp;
  DL_FOREACH_SAFE(head, item, tmp) {
    item->comp->_vt->del(item->comp);
    DL_DELETE(head, item);
  }
  mod->components = NULL;
}

static struct fmc_component_module *mod_load(struct fmc_component_sys *sys, const char *dir,
                                             const char *modstr, const char *mod_lib, const char *mod_func,
                                             fmc_error_t **error) {
  fmc_error_t *err = NULL;
  int psz = fmc_path_join_len(dir, mod_lib);
  char lib_path[psz];
  fmc_path_join(lib_path, psz, dir, mod_lib);

  struct fmc_component_module mod;
  memset(&mod, 0, sizeof(mod));

  mod.handle = fmc_ext_open(lib_path, &err);
  if(err) goto error_1;

  // Check if init function is available
  fmc_comp_mod_init_func mod_init = (fmc_comp_mod_init_func)fmc_ext_sym(mod.handle, mod_func, &err);
  if(err) goto error_1;

  // append the mod to the system
  mod.sys = sys;
  mod.name = fmc_cstr_new(modstr, error);
  if(*error) goto error_1;
  mod.file = fmc_cstr_new(lib_path, error);
  if(*error) goto error_1;
  mod.components_type = mod_init();

  struct fmc_component_module *m = (struct fmc_component_module *)calloc(1, sizeof(mod));
  if(!m) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    goto error_1;
  }
  memcpy(m, &mod, sizeof(mod));
  DL_APPEND(sys->modules, m);

  return m;
error_1:
  fmc_component_module_destroy(&mod);
  return NULL;
}

struct fmc_component_module *fmc_component_module_new(struct fmc_component_sys *sys, const char *mod, fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_component_module *ret = NULL;
  
  char mod_lib[strlen(mod) + strlen(FMC_LIB_SUFFIX) + 1];
  sprintf(mod_lib, "%s%s", mod, FMC_LIB_SUFFIX);
  
  int pathlen = fmc_path_join_len(mod, mod_lib);
  char mod_lib_2[pathlen];
  fmc_path_join(mod_lib_2, pathlen, mod, mod_lib);

  char mod_func[strlen(fmc_comp_INIT_FUNCT_PREFIX) + strlen(mod) + 1];
  sprintf(mod_func, "%s%s", fmc_comp_INIT_FUNCT_PREFIX, mod);
  fmc_component_path_list_t *head = sys->search_paths;
  fmc_component_path_list_t *item;
  DL_FOREACH(head,item) {
    ret = mod_load(sys, item->path, mod, mod_lib, mod_func, error);
    if(!ret && !(*error)) {
      ret = mod_load(sys, item->path, mod, mod_lib_2, mod_func, error);
    }
    if(ret || *error) {
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

struct fmc_component *fmc_component_new(struct fmc_component_module *mod, const char *comp,
                                        struct fmc_cfg_sect_item *cfg, fmc_error_t **error) {
  fmc_error_clear(error);
  for(unsigned int i = 0; mod->components_type && mod->components_type[i].name; ++i) {
    struct fmc_component_type *tp = &mod->components_type[i];
    if(!strcmp(tp->name, comp)) {
      fmc_cfg_node_spec_check(tp->cfgspec, cfg, error);
      if (*error) return NULL;

      fmc_component_list_t *item = (fmc_component_list_t *)calloc(1, sizeof(*item));
      if(item) {
        struct fmc_component *ret = tp->new_(cfg, error);
        if(*error) {
          free(item);
          return NULL;
        } else {
          ret->_mod = mod;
          ret->_vt = tp;
          fmc_error_init_none(&ret->_err);
          item->comp = ret;
          DL_APPEND(mod->components, item);
          return ret;
        }
      } else {
        fmc_error_set2(error, FMC_ERROR_MEMORY);
        return NULL;
      }
      break;
    }
  }
  fmc_error_set(error, "could not find computation %s in module %s", comp, mod->name);
  return NULL;
}

void fmc_component_del(struct fmc_component *comp) {
  struct fmc_component_module *m = comp->_mod;
  fmc_component_list_t *head = m->components;
  fmc_component_list_t *item;
  DL_FOREACH(head, item) {
    if (item->comp == comp) {
      DL_DELETE(m->components, item);
      free(item);
      break;
    }
  }
  comp->_vt->del(comp);
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
  DL_FOREACH_SAFE(modhead, mod, modtmp) {
    fmc_component_module_del(mod);
  }
  sys->modules = NULL;
}