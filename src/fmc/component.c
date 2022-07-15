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

void fmc_component_sys_paths_set(struct fmc_component_sys *sys, const char **paths, fmc_error_t **error) {
  unsigned int cnt = 0;
  for(; paths[cnt]; ++cnt) {}
  sys->search_paths = (char **)calloc(cnt+1, sizeof(*(sys->search_paths)));
  if(!sys->search_paths) {
    *error = fmc_error_inst(); // TODO: check this
    fmc_error_init(*error, FMC_ERROR_MEMORY, NULL);
    return;
  }

  for(cnt = 0; paths[cnt]; ++cnt) {
    sys->search_paths[cnt] = (char *)calloc(strlen(paths[cnt])+1, sizeof(**(sys->search_paths)));
    if(!sys->search_paths[cnt]) {
      for(unsigned int i = 0; i < cnt; ++i) {
        free(sys->search_paths[i]);
      }
      free(sys->search_paths);
      sys->search_paths = NULL;
      *error = fmc_error_inst(); // TODO: check this
      fmc_error_init(*error, FMC_ERROR_MEMORY, NULL);
      return;
    }
    strcpy(sys->search_paths[cnt], paths[cnt]);
  }
  sys->search_paths[cnt] = NULL;
}

const char **fmc_component_sys_paths_get(struct fmc_component_sys *sys) {
  return (const char **)sys->search_paths;
}

#if defined(FMC_SYS_UNIX)
struct fmc_component_module *mod_load_recursive(struct fmc_component_sys *sys, const char *dir, const char *mod, const char *mod_lib, fmc_error_t **error) {
  struct fmc_component_module *ret = NULL;
  fmc_ext_t ext = NULL;
  DIR *d = opendir(dir);
  if (!d) {
    return NULL;
  }
  struct dirent *entry;
  char *path;
  while ((entry = readdir(d)) != NULL) {
    if(entry->d_type == DT_LNK) {
      // TODO: check if symbolic link is a file or a directory
    }
    else if(entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
      // Recursive find
      size_t path_len = strlen(dir) + strlen(entry->d_name) + 1;
      path = (char *)calloc(path_len+1, sizeof(*path));
      if(!path) {
        *error = fmc_error_inst(); // TODO: check this
        fmc_error_init(*error, FMC_ERROR_MEMORY, NULL);
        break;
      }
      sprintf(path, "%s/%s", dir, entry->d_name);
      ret = mod_load_recursive(sys, path, mod, mod_lib, error);
      free(path);
      if(*error || ret) {
        // Only error possible is type FMC_ERROR_NONE
        break;
      }
    }
    else if(entry->d_type == DT_REG && !strcmp(entry->d_name, mod_lib)) {
      // module file found
      size_t path_len = strlen(dir) + strlen(entry->d_name) + 1;
      path = (char *)calloc(path_len+1, sizeof(*path));
      if(!path) {
        *error = fmc_error_inst(); // TODO: check this
        fmc_error_init(*error, FMC_ERROR_MEMORY, NULL);
        break;
      }
      sprintf(path, "%s/%s", dir, entry->d_name);
      ext = fmc_ext_open(path, error);
      if(*error) {
        // If it is an error don't abort. Keep searching.
        free(path);
        fmc_error_destroy(*error);
        fmc_error_clear(error);
      }
      else if(ext) {
        // Check if init function is available
        char *comp_init_function = (char *)calloc(strlen("FMCompInit_")+strlen(mod)+1, sizeof(*comp_init_function));
        if(!comp_init_function) {
          fmc_ext_close(ext);
          free(path);
          *error = fmc_error_inst(); // TODO: check this
          fmc_error_init(*error, FMC_ERROR_MEMORY, NULL);
          break;
        }
        sprintf(comp_init_function, "FMCompInit_%s", mod);
        FMCOMPINITFUNC components_type_init = (FMCOMPINITFUNC)fmc_ext_sym(ext, comp_init_function, error);
        free(comp_init_function);
        if(*error) {
          // Function is not there. Keep searching.
          fmc_ext_close(ext);
          free(path);
          fmc_error_destroy(*error);
          fmc_error_clear(error);
        }
        else if(components_type_init) {
          // append the module to the system
          list_fmc_component_module_t *module = (list_fmc_component_module_t *)calloc(1, sizeof(*module));
          if(!module) {
            fmc_ext_close(ext);
            free(path);
            *error = fmc_error_inst(); // TODO: check this
            fmc_error_init(*error, FMC_ERROR_MEMORY, NULL);
            break;
          }
          module->mod.handle = ext;
          module->mod.name = (char *)calloc(strlen(mod)+1, sizeof(*(module->mod.name)));
          if(!module->mod.name) {
            free(module);
            fmc_ext_close(ext);
            free(path);
            *error = fmc_error_inst(); // TODO: check this
            fmc_error_init(*error, FMC_ERROR_MEMORY, NULL);
            break;
          }
          strcpy(module->mod.name, mod);
          module->mod.path = path;
          module->mod.components_type = components_type_init(sys);
          module->mod.components = NULL; // empty components for now
          list_fmc_component_module_t *moduleshead = sys->modules;
          DL_APPEND(moduleshead, module);
          ret = &(module->mod);
          break; // ext is the valid module. Don't free() path.
        }
      }
      free(path);
    }
  }
  closedir(d);
  return ret;
}
#else
#error "Unsupported operating system"
#endif

struct fmc_component_module *fmc_component_module_load(struct fmc_component_sys *sys, const char *mod, fmc_error_t **error) {
  // search for the module (mod.so; e.g oms.so) in the search_paths
  // Look for the function FMCompInit_mod (e.g.FMCompInit_oms)
  // if it does not have this function, keep looking

  struct fmc_component_module *ret = NULL;
  char *mod_lib = (char *)calloc(strlen(mod)+strlen(FMC_LIB_SUFFIX)+1, sizeof(*mod_lib));
  if(!mod_lib) {
    *error = fmc_error_inst(); // TODO: check this
    fmc_error_init(*error, FMC_ERROR_MEMORY, NULL);
    return false;
  }
  sprintf(mod_lib, "%s%s", mod, FMC_LIB_SUFFIX);
#if defined(FMC_SYS_UNIX)
  if(sys->search_paths) {
    for(unsigned int i = 0; sys->search_paths[i] && !ret && !(*error); ++i) {
      ret = mod_load_recursive(sys, sys->search_paths[i], mod, mod_lib, error);
    }
  }
#else
#error "Unsupported operating system"
#endif
  free(mod_lib);
  return ret;
}

static void sys_comp_del(list_fmc_component_t **comp) {
  // TODO: destroy (*comp)->comp (struct fmc_component) ??
  // e.g struct fmc_comp_type *_vt, struct fmc_error _err
  free(*comp);
  *comp = NULL;
}

static void sys_mod_del(list_fmc_component_module_t **mod) {
  struct fmc_component_module *m = &((*mod)->mod);
  // delete module
  fmc_ext_close(m->handle);
  free(m->name);
  free(m->path);
  m->components_type = NULL;
  // delete all components in the module
  list_fmc_component_t *comphead = m->components;
  list_fmc_component_t *comp;
  list_fmc_component_t *comptmp;
  DL_FOREACH_SAFE(comphead,comp,comptmp) {
    DL_DELETE(comphead,comp);
    sys_comp_del(&comp);
  }
  m->components = NULL;
  free(*mod);
  *mod = NULL;
}

static list_fmc_component_module_t *find_mod(struct fmc_component_sys *sys, struct fmc_component_module *mod) {
  list_fmc_component_module_t *mhead = sys->modules;
  list_fmc_component_module_t *m = NULL;
  DL_FOREACH(mhead,m) {
    if( &(m->mod) == mod) {
      return m;
    }
  }
  return NULL;
}

void fmc_component_module_unload(struct fmc_component_module *mod) {
  struct fmc_component_sys *sys = mod->sys;
  list_fmc_component_module_t *m = find_mod(sys, mod);
  if(m) {
    DL_DELETE(sys->modules,m);
    sys_mod_del(&m);
  }
}

struct fmc_component *fmc_component_new(struct fmc_component_module *mod, const char *comp, struct fmc_cfg_sect_item *cfg, fmc_error_t **error) {
  for(unsigned int i = 0; mod->components_type[i].size; ++i) {
    if(!strcmp(mod->components_type[i].name, comp)) {
      // component was found.
      // TODO: validate cfg against mod.components_type[i].cfgspec
      // TODO: allocate with the actual size of the component with mod.components_type[i].size
      list_fmc_component_t *newc = (list_fmc_component_t *)calloc(1, sizeof(*newc));
      if(newc) {
        // TODO: initialize vt, fmc_error, and the component struct?
        DL_APPEND(mod->components, newc);
      }
      return &(newc->comp);
    }
  }
  return NULL;
}

void fmc_component_destroy(struct fmc_component *comp) {
  struct fmc_component_module *m = comp->_mod;
  list_fmc_component_t *chead = m->components;
  list_fmc_component_t *c;
  DL_FOREACH(chead,c) {
    if( (&(c->comp)) == comp ) {
      DL_DELETE(m->components,c);
      sys_comp_del(&c);
      break;
    }
  }
}

void fmc_component_sys_destroy(struct fmc_component_sys *sys) {
  for(unsigned int cnt = 0; sys->search_paths[cnt]; ++cnt) {
    free(sys->search_paths[cnt]);
  }
  free(sys->search_paths);
  sys->search_paths = NULL;

  // destroy modules: also destroys components of the module
  list_fmc_component_module_t *modhead = sys->modules;
  list_fmc_component_module_t *mod;
  list_fmc_component_module_t *modtmp;
  DL_FOREACH_SAFE(modhead,mod,modtmp) {
    DL_DELETE(modhead,mod);
    sys_mod_del(&mod);
  }
  sys->modules = NULL;
}