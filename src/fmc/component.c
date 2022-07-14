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

//TODO: check for callocs return != NULL

void fmc_component_sys_init(struct fmc_component_sys *sys) {
  sys->search_paths = NULL;
  sys->modules = NULL; // important- initialize lists to NULL
}

void fmc_component_sys_paths_set(struct fmc_component_sys *sys, const char **paths) {
  unsigned int cnt = 0;
  for(; paths[cnt]; ++cnt) {}
  sys->search_paths = (char **)calloc(cnt+1, sizeof(*(sys->search_paths)));

  for(cnt = 0; paths[cnt]; ++cnt) {
    sys->search_paths[cnt] = (char *)calloc(strlen(paths[cnt])+1, sizeof(**(sys->search_paths)));
    strcpy(sys->search_paths[cnt], paths[cnt]);
  }
  sys->search_paths[cnt] = NULL;
}

const char **fmc_component_sys_paths_get(struct fmc_component_sys *sys) {
  return (const char **)sys->search_paths;
}

static bool mod_load_recursive(struct fmc_component_sys *sys, const char *dir, const char *mod, fmc_error_t **error) {
  fmc_ext_t ext = NULL;
  DIR *d = opendir(dir);
  if (d == NULL) {
    FMC_ERROR_REPORT(error, strerror(errno));
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
      sprintf(path, "%s/%s", dir, entry->d_name);
      bool found = mod_load_recursive(sys, path, mod, error);
      free(path);
      if(*error || found) {
        // TODO: if error should I abort? Maybe it is a file called mod.so or a broken dir?
        break;
      }
    }
    else if(entry->d_type == DT_REG && !strcmp(entry->d_name, mod)) {
      // module file found
      size_t path_len = strlen(dir) + strlen(entry->d_name) + 1;
      path = (char *)calloc(path_len+1, sizeof(*path));
      sprintf(path, "%s/%s", dir, entry->d_name);
      ext = fmc_ext_open(path, error);
      if(*error) {
        // TODO: if error should I abort? Maybe it is a file called mod.so
        fmc_error_destroy(*error);
      }
      else if(ext) {
        // Check if init function is available
        char *comp_init_function = (char *)calloc(strlen("FMCompInit_")+strlen(mod)+1, sizeof(*comp_init_function));
        sprintf(comp_init_function, "FMCompInit_%s", mod);
        FMCOMPINITFUNC components_type_init = (FMCOMPINITFUNC)fmc_ext_sym(ext, comp_init_function, error);
        free(comp_init_function);
        if(*error) {
          // Function is not there => try on other files
          // Reset the error because it is not an error
          fmc_error_destroy(*error);
        }
        else if(components_type_init) {
          // append the module to the system
          list_fmc_component_module_t *module = (list_fmc_component_module_t *)calloc(1, sizeof(*module));
          module->mod.handle = ext;
          module->mod.name = (char *)calloc(strlen(mod)+1, sizeof(*(module->mod.name)));
          strcpy(module->mod.name, mod);
          module->mod.path = path;
          module->mod.components_type = components_type_init(sys);
          module->mod.components = NULL; // empty components for now
          DL_APPEND(sys->modules, module);
          return true; // ext is the valid module. Don't free() path.
        }
      }
      free(path);
    }
  }
  return false;
}

bool fmc_component_sys_mod_load(struct fmc_component_sys *sys, const char *mod) {
  // search for the module (mod.so; e.g oms.so) in the search_paths
  // Look for the function FMCompInit_mod (e.g.FMCompInit_oms)
  // if it does not have this function, keep looking

  // TODO: if it fails error?
  fmc_error_t *error = NULL;
  bool found = false;
#if defined(FMC_SYS_UNIX)
  if(sys->search_paths) {
    for(unsigned int i = 0; sys->search_paths[i] && !found && !error; ++i) {
      found = mod_load_recursive(sys, sys->search_paths[i], mod, &error);
    }
  }
#else
#error "Unsupported operating system"
#endif
  return found;
}

static void sys_comp_del(list_fmc_component_t **comp) {
  // TODO: destroy (*comp)->comp (struct fmc_component)
  // e.g struct fmc_comp_type *_vt, struct fmc_error _err
  free(*comp);
  *comp = NULL;
}

static void sys_mod_del(list_fmc_component_module_t **mod) {
  // delete module
  fmc_ext_close((*mod)->mod.handle);
  free((*mod)->mod.name);
  free((*mod)->mod.path);
  (*mod)->mod.components_type = NULL;
  // delete all components in the module
  list_fmc_component_t *comp;
  list_fmc_component_t *comptmp;
  DL_FOREACH_SAFE((*mod)->mod.components,comp,comptmp) {
    DL_DELETE((*mod)->mod.components,comp);
    sys_comp_del(&comp);
  }
  (*mod)->mod.components = NULL;
  free(*mod);
  *mod = NULL;
}

void fmc_component_sys_mod_unload(struct fmc_component_sys *sys, const char *mod) {
  list_fmc_component_module_t *modunload = NULL;
  DL_FOREACH(sys->modules,modunload) {
    if(!strcmp(modunload->mod.name, mod)) {
      break;
    }
  }
  DL_DELETE(sys->modules,modunload);
  sys_mod_del(&modunload);
}

struct fmc_component *fmc_component_sys_comp_new(struct fmc_component_sys *sys, const char *mod, const char *comp, struct fmc_cfg_sect_item *cfg) {

}

void fmc_component_sys_comp_destroy(struct fmc_component_sys *sys, const char *mod, struct fmc_component *comp) {
  bool found = false;
  list_fmc_component_module_t *modcomp = NULL;
  DL_FOREACH(sys->modules,modcomp) {
    if(!strcmp(modcomp->mod.name, mod)) {
      found = true;
      break;
    }
  }
  if(found) {
    found = false;
    list_fmc_component_t *compit;
    DL_FOREACH(modcomp->mod.components,compit) {
      if( (&(compit->comp)) == comp ) {
        found = true;
        break;
      }
    }
    if(found) {
      DL_DELETE(modcomp->mod.components,compit);
      sys_comp_del(&compit);
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
  list_fmc_component_module_t *mod;
  list_fmc_component_module_t *modtmp;
  DL_FOREACH_SAFE(sys->modules,mod,modtmp) {
    DL_DELETE(sys->modules,mod);
    sys_mod_del(&mod);
  }
}