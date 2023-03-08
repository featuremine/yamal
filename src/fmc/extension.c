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
 * @file extension.c
 * @date 17 Jun 2021
 * @brief File contains C implementation of fmc ext loading
 *
 * This file contains C implementation of fmc ext loading.
 * @see http://www.featuremine.com
 */
#include <fmc/extension.h>
#include <fmc/files.h>
#include <fmc/string.h>

#include <uthash/utarray.h>
#include <uthash/utheap.h>
#include <uthash/utlist.h>

#include <string.h>

#if defined(FMC_SYS_WIN)
#include <windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#if defined(FMC_SYS_UNIX)
#define FMC_MOD_SEARCHPATH_CUR "."
#define FMC_MOD_SEARCHPATH_USRLOCAL ".local"
#define FMC_MOD_SEARCHPATH_SYSLOCAL "/usr/local"
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

fmc_ext_t fmc_ext_open(const char *path, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_UNIX)
  dlerror(); // Clear any existing error
  fmc_ext_t handle = dlopen(path, RTLD_NOW);
  if (!handle) {
    FMC_ERROR_REPORT(error, dlerror());
  }
  return handle;
#else
#error "Unsupported operating system"
#endif
}

void *fmc_ext_sym(fmc_ext_t handle, const char *sym, fmc_error_t **error) {
#if defined(FMC_SYS_UNIX)
  dlerror();                      // Clear any existing error
  void *ret = dlsym(handle, sym); // ret could be NULL and it is OK
  char *dlerrorstr = dlerror();
  if (dlerrorstr) {
    FMC_ERROR_REPORT(error, dlerrorstr);
  }
  return ret;
#else
#error "Unsupported operating system"
#endif
}

void fmc_ext_close(fmc_ext_t handle) {
#if defined(FMC_SYS_UNIX)
  if (handle) {
    dlclose(handle);
  }
#else
#error "Unsupported operating system"
#endif
}

struct mod_load_result {
  struct fmc_ext_mod_t mod;
  bool not_found;
};

static struct mod_load_result mod_load(const char *dir, const char *relpath,
                                       const char *fnname,
                                       fmc_error_t **error) {
  fmc_error_clear(error);

  size_t pathsz = fmc_path_join(NULL, 0, dir, relpath) + 1;
  char path[pathsz];
  fmc_path_join(path, pathsz, dir, relpath);

  struct mod_load_result res;
  res.mod.path = NULL;
  res.mod.handle = fmc_ext_open(path, error);
  res.not_found = false;
  if (*error) {
    fmc_error_set(error, "module not found");
    res.not_found = true;
    goto cleanup;
  }

  // Check if init function is available
  res.mod.func = fmc_ext_sym(res.mod.handle, fnname, error);
  if (*error) {
    fmc_error_set(error, "init function not found");
    res.not_found = true;
    goto cleanup;
  }

  res.mod.path = fmc_cstr_new(path, error);
  if (*error) {
    goto cleanup;
  }

  return res;

cleanup:
  fmc_ext_mod_destroy(&res.mod);
  res.mod.handle = NULL;
  return res;
}

struct fmc_ext_mod_t fmc_ext_mod_load(const char *modname,
                                      const char *initfunc_prefix,
                                      struct fmc_ext_searchpath_t *search_paths,
                                      fmc_error_t **error) {
  fmc_error_clear(error);

  size_t modsz = strlen(modname);
  size_t relpath1sz = modsz + strlen(FMC_LIB_SUFFIX) + 1;
  char relpath1[modsz + strlen(FMC_LIB_SUFFIX) + 1];
  snprintf(relpath1, relpath1sz, "%s%s", modname, FMC_LIB_SUFFIX);

  size_t relpath2sz = fmc_path_join(NULL, 0, modname, relpath1) + 1;
  char relpath2[relpath2sz];
  fmc_path_join(relpath2, relpath2sz, modname, relpath1);

  size_t modfuncsz = strlen(initfunc_prefix) + strlen(modname) + 1;
  char modfunc[modfuncsz];
  snprintf(modfunc, modfuncsz, "%s%s", initfunc_prefix, modname);

  struct fmc_ext_searchpath_t *head = search_paths;
  struct fmc_ext_searchpath_t *item;
  struct mod_load_result res;
  res.mod.handle = NULL;
  res.mod.path = NULL;
  res.not_found = true;
  DL_FOREACH(head, item) {
    res = mod_load(item->path, relpath1, modfunc, error);
    if (!res.not_found) {
      break;
    }
    res = mod_load(item->path, relpath2, modfunc, error);
    if (!res.not_found) {
      break;
    }
  }
  if (res.not_found) {
    fmc_ext_mod_destroy(&res.mod);
    res.mod.handle = NULL;
    res.mod.path = NULL;
    fmc_error_set(error, "component module %s was not found", modname);
  }
  return res.mod;
}

void fmc_ext_mod_destroy(struct fmc_ext_mod_t *mod) {
  fmc_ext_close(mod->handle);
  free((char *)mod->path);
}

void fmc_ext_searchpath_del(struct fmc_ext_searchpath_t **phead) {
  if (!*phead)
    return;
  struct fmc_ext_searchpath_t *p;
  struct fmc_ext_searchpath_t *ptmp;
  DL_FOREACH_SAFE(*phead, p, ptmp) {
    DL_DELETE(*phead, p);
    free(p);
  }
}

void fmc_ext_searchpath_add(struct fmc_ext_searchpath_t **phead,
                            const char *path, fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_ext_searchpath_t *p =
      (struct fmc_ext_searchpath_t *)calloc(1, sizeof(*p) + strlen(path) + 1);
  if (p) {
    strcpy(p->path, path);
    DL_APPEND(*phead, p);
  } else {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return;
  }
}

void fmc_ext_searchpath_set(struct fmc_ext_searchpath_t **head,
                            const char **paths, fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_ext_searchpath_t *tmp = NULL;
  for (unsigned int i = 0; paths && paths[i]; ++i) {
    fmc_ext_searchpath_add(&tmp, paths[i], error);
    if (*error) {
      fmc_ext_searchpath_del(&tmp);
      return;
    }
  }
  fmc_ext_searchpath_del(head);
  *head = tmp;
}

FMMODFUNC void
fmc_ext_searchpath_set_default(struct fmc_ext_searchpath_t **head,
                               const char *defaultpath, const char *pathenvname,
                               fmc_error_t **error) {
  fmc_error_clear(error);

  const char *homedir = getenv("HOME");
  int homepath1sz =
      fmc_path_join(NULL, 0, homedir, FMC_MOD_SEARCHPATH_USRLOCAL) + 1;
  char homepath1[homepath1sz];
  fmc_path_join(homepath1, homepath1sz, homedir, FMC_MOD_SEARCHPATH_USRLOCAL);

  int homepath2sz = fmc_path_join(NULL, 0, homepath1, defaultpath) + 1;
  char homepath2[homepath2sz];
  fmc_path_join(homepath2, homepath2sz, homepath1, defaultpath);

  int syspathsz =
      fmc_path_join(NULL, 0, FMC_MOD_SEARCHPATH_SYSLOCAL, defaultpath) + 1;
  char syspath[syspathsz];
  fmc_path_join(syspath, syspathsz, FMC_MOD_SEARCHPATH_SYSLOCAL, defaultpath);

  const char *defaults[] = {FMC_MOD_SEARCHPATH_CUR, homepath2, syspath, NULL};

  struct fmc_ext_searchpath_t *tmpls = NULL;
  fmc_ext_searchpath_set(&tmpls, defaults, error);
  if (*error) {
    goto cleanup;
  }

  char *pathenv = getenv(pathenvname);
  if (pathenv) {
    char ycpaths[strlen(pathenv) + 1];
    strcpy(ycpaths, pathenv);
    pathenv = ycpaths;
    char *found;
    while ((found = strsep(&pathenv, FMC_MOD_SEARCHPATH_ENV_SEP))) {
      fmc_ext_searchpath_add(&tmpls, found, error);
      if (*error) {
        goto cleanup;
      }
    }
  }
  struct fmc_ext_searchpath_t *tmpls2 = *head;
  *head = tmpls;
  tmpls = tmpls2;
  return;

cleanup:
  fmc_ext_searchpath_del(&tmpls);
}
