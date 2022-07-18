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
#include <fmc/uthash/utlist.h>
#include <stdlib.h> // calloc()

#if defined(FMC_SYS_UNIX)
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#else
#error "Unsupported operating system"
#endif

void fmc_component_sys_init(struct fmc_component_sys *sys) {
  sys->search_paths = NULL;
  sys->modules = NULL; // important- initialize lists to NULL
}

void component_path_list_del(fmc_component_path_list_t *phead) {
    if (!phead) return;
    fmc_component_path_list_t *p;
    fmc_component_path_list_t *ptmp;
    DL_FOREACH_SAFE(phead,p,ptmp) {
      DL_DELETE(phead,p);
      free(p);
    }
}

void component_path_list_add(fmc_component_path_list_t *phead, const char *path, fmc_error_t **error) {
  fmc_error_clear(error);
  fmc_component_path_list_t *p = (fmc_component_path_list_t *)calloc(1, sizeof(*p)+strlen(path)+1);
  if(p) {
    strcpy(p->path, path);
    DL_APPEND(phead, p);
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
    component_path_list_add(tmpls, paths[i], error);
    if (*error) {
      component_path_list_del(tmpls);
      return;
    }
  }
  component_path_list_del(sys->search_paths);
  sys->search_paths = tmpls;
}

void fmc_component_sys_paths_add(struct fmc_component_sys *sys, const char *path, fmc_error_t **error) {
  fmc_error_clear(*error);
  if(path) {
    component_path_list_add(sys->search_paths, path, error);
  }
}

fmc_component_path_list_t *fmc_component_sys_paths_get(struct fmc_component_sys *sys) {
  return sys->search_paths;
}

// Possible library paths: <path>/mod.so or <path>/mod/mod.so
// TODO(max): I have some comments here. This functions can be improved 
#if defined(FMC_SYS_UNIX)
static struct fmc_component_module *mod_load(struct fmc_component_sys *sys, const char *dir, const char *mod, const char *mod_lib, fmc_error_t **error) {
  struct fmc_component_module *ret = NULL;
  char *lib_path;
  fmc_path_join(&lib_path, dir, mod_lib, error);
  if(*error) {
    goto error_0;
  }
  fmc_ext_t ext = fmc_ext_open(lib_path, error);
  if(*error) {
    // If it is an error don't abort. Keep searching.
    fmc_error_destroy(*error);
    fmc_error_clear(error);
    goto error_1;
  }
  if(!ext) {
    goto error_1;
  }
  // Check if init function is available
  char *comp_init_function = (char *)calloc(strlen(fmc_comp_INIT_FUNCT_PREFIX)+strlen(mod)+1, sizeof(*comp_init_function));
  if(!comp_init_function) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    goto error_2;
  }
  sprintf(comp_init_function, fmc_comp_INIT_FUNCT_PREFIX "%s", mod);
  FMCOMPINITFUNC components_type_init = (FMCOMPINITFUNC)fmc_ext_sym(ext, comp_init_function, error);
  free(comp_init_function);
  if(*error) {
    // Function is not there. Keep searching.
    fmc_error_destroy(*error);
    fmc_error_clear(error);
    goto error_2;
  }
  if(!components_type_init) {
    goto error_2;
  }
  // append the module to the system
  fmc_component_module_list_t *module = (fmc_component_module_list_t *)calloc(1, sizeof(*module));
  if(!module) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    goto error_2;
  }
  module->mod.sys = sys;
  module->mod.handle = ext;
  module->mod.name = (char *)calloc(strlen(mod)+1, sizeof(*(module->mod.name)));
  if(!module->mod.name) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    goto error_3;
  }
  strcpy(module->mod.name, mod);
  module->mod.path = lib_path;
  module->mod.components_type = components_type_init();
  module->mod.components = NULL; // empty components for now
  DL_APPEND(sys->modules, module);
  ret = &(module->mod);

error_3:
  free(module);
error_2:
  fmc_ext_close(ext);
error_1:
  if(!ret) {
    free(lib_path);
  }
error_0:
  return ret;
}
#else
#error "Unsupported operating system"
#endif

struct fmc_component_module *fmc_component_module_new(struct fmc_component_sys *sys, const char *mod, fmc_error_t **error) {
  // search for the module (mod.so; e.g oms.so) in the search_paths
  // Look for the function FMCompInit_mod (e.g.FMCompInit_oms)
  // if it does not have this function, keep looking

  struct fmc_component_module *ret = NULL;
  char mod_lib[strlen(mod)+strlen(FMC_LIB_SUFFIX)+1];
  sprintf(mod_lib, "%s%s", mod, FMC_LIB_SUFFIX);
  char mod_lib_2[strlen(mod)+1+strlen(mod_lib)+1];
  fmc_path_join((char **)&mod_lib_2, mod, mod_lib, error);
#if defined(FMC_SYS_UNIX)
  fmc_component_path_list_t *phead = sys->search_paths;
  fmc_component_path_list_t *p;
  DL_FOREACH(phead,p) {
    ret = mod_load(sys, p->path, mod, mod_lib, error);
    if(!ret && !(*error)) {
      ret = mod_load(sys, p->path, mod, mod_lib_2, error);
    }
    if(ret || *error) {
      break;
    }
  }
#else
#error "Unsupported operating system"
#endif
  return ret;
}

static void comp_del(fmc_component_list_t **comp) {
  (*comp)->comp->_vt->del((*comp)->comp);
  free(*comp);
  *comp = NULL;
}

static void mod_del(fmc_component_module_list_t **mod) {
  struct fmc_component_module *m = &((*mod)->mod);
  // delete module
  m->sys = NULL;
  fmc_ext_close(m->handle);
  free(m->name);
  free(m->path);
  m->components_type = NULL;
  // delete all components in the module
  fmc_component_list_t *comphead = m->components;
  fmc_component_list_t *comp;
  fmc_component_list_t *comptmp;
  DL_FOREACH_SAFE(comphead,comp,comptmp) {
    DL_DELETE(comphead,comp);
    comp_del(&comp);
  }
  m->components = NULL;
  free(*mod);
  *mod = NULL;
}

static fmc_component_module_list_t *find_mod(struct fmc_component_sys *sys, struct fmc_component_module *mod) {
  fmc_component_module_list_t *mhead = sys->modules;
  fmc_component_module_list_t *m = NULL;
  DL_FOREACH(mhead,m) {
    if( &(m->mod) == mod) {
      return m;
    }
  }
  return NULL;
}

void fmc_component_module_destroy(struct fmc_component_module *mod) {
  struct fmc_component_sys *sys = mod->sys;
  fmc_component_module_list_t *m = find_mod(sys, mod);
  if(m) {
    DL_DELETE(sys->modules,m);
    mod_del(&m);
  }
}

struct fmc_component *fmc_component_new(struct fmc_component_module *mod, const char *comp, struct fmc_cfg_sect_item *cfg, fmc_error_t **error) {
  struct fmc_component *ret = NULL;
  for(unsigned int i = 0; mod->components_type[i].size; ++i) {
    if(!strcmp(mod->components_type[i].name, comp)) {
      // component was found.
      // TODO: validate cfg against mod.components_type[i].cfgspec
      fmc_component_list_t *newc = (fmc_component_list_t *)calloc(1, sizeof(*newc));
      if(newc) {
        ret = mod->components_type[i].new(cfg, error);
        if(*error) {
          ret = NULL;
          free(newc);
        }
        else {
          ret->_mod = mod;
          ret->_vt = &mod->components_type[i];
          fmc_error_init_none(&ret->_err);
          newc->comp = ret;
          DL_APPEND(mod->components, newc);
        }
      }
      else {
        fmc_error_set2(error, FMC_ERROR_MEMORY);
      }
      break;
    }
  }
  return ret;
}

void fmc_component_destroy(struct fmc_component *comp) {
  struct fmc_component_module *m = comp->_mod;
  fmc_component_list_t *chead = m->components;
  fmc_component_list_t *c;
  DL_FOREACH(chead,c) {
    if( c->comp == comp ) {
      DL_DELETE(m->components,c);
      comp_del(&c);
      break;
    }
  }
}

void fmc_component_sys_destroy(struct fmc_component_sys *sys) {
  fmc_component_path_list_t *phead = sys->search_paths;
  fmc_component_path_list_t *p;
  fmc_component_path_list_t *ptmp;
  DL_FOREACH_SAFE(phead,p,ptmp) {
    DL_DELETE(phead,p);
    free(p);
  }
  sys->search_paths = NULL;

  // destroy modules: also destroys components of the module
  fmc_component_module_list_t *modhead = sys->modules;
  fmc_component_module_list_t *mod;
  fmc_component_module_list_t *modtmp;
  DL_FOREACH_SAFE(modhead,mod,modtmp) {
    DL_DELETE(modhead,mod);
    mod_del(&mod);
  }
  sys->modules = NULL;
}