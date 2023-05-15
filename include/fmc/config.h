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
 * @file config.h
 * @date 11 Jul 2022
 * @brief Yet another configuration API
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/error.h>
#include <fmc/files.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  FMC_CFG_NONE,    /*  0 */
  FMC_CFG_BOOLEAN, /*  1 */
  FMC_CFG_INT64,   /*  2 */
  FMC_CFG_FLOAT64, /*  3 */
  FMC_CFG_STR,     /*  4 */
  FMC_CFG_SECT,    /*  5 */
  FMC_CFG_ARR      /*  6 */
} FMC_CFG_TYPE;

/*
This is an example of how to implement a configuration specification:

fmc_cfg_node_spec session_cfg_spec[] = {
   {"channel", "YTP channel of the session", FMC_CFG_STR, true, NULL},
   {NULL}
}

fmc_cfg_arr_spec sessions_cfg_spec = {FMC_CFG_SECT, &session_cfg_spec};

fmc_cfg_node_spec gateway_cfg_spec[] = {
   {"sessions", "Array of individual session configuration", FMC_CFG_ARR,
       true,&sessions_cfg_spec},
   {NULL}
};
*/

// Specification of the configuration type
struct fmc_cfg_type {
  FMC_CFG_TYPE type; // type of the array element
  union {
    struct fmc_cfg_type *array; // Pointer to an array item type
    struct fmc_cfg_node_spec
        *node; // Pointer to null terminated array of node specs
  } spec;
};

// Configuration node specification
struct fmc_cfg_node_spec {
  const char *key; // Key of the configuration node
  const char *descr;
  bool required;
  struct fmc_cfg_type type;
};

struct fmc_cfg_item {
  union {
    bool boolean;
    int64_t int64;
    double float64;
    char *str;
    struct fmc_cfg_sect_item *sect;
    struct fmc_cfg_arr_item *arr;
  } value;
  FMC_CFG_TYPE type;
};

struct fmc_cfg_sect_item {
  const char *key;
  struct fmc_cfg_item node;
  struct fmc_cfg_sect_item *next;
};

// Array values
struct fmc_cfg_arr_item {
  struct fmc_cfg_item item; // value
  struct fmc_cfg_arr_item *next;
};

FMMODFUNC void fmc_cfg_sect_del(struct fmc_cfg_sect_item *);
FMMODFUNC struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_none(struct fmc_cfg_sect_item *, const char *,
                           fmc_error_t **err);
FMMODFUNC struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_boolean(struct fmc_cfg_sect_item *, const char *, bool,
                              fmc_error_t **err);
FMMODFUNC struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_int64(struct fmc_cfg_sect_item *, const char *, int64_t,
                            fmc_error_t **err);
FMMODFUNC struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_float64(struct fmc_cfg_sect_item *, const char *, double,
                              fmc_error_t **err);
FMMODFUNC struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_str(struct fmc_cfg_sect_item *, const char *,
                          const char *, fmc_error_t **err);
FMMODFUNC struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_sect(struct fmc_cfg_sect_item *, const char *,
                           struct fmc_cfg_sect_item *, fmc_error_t **err);
FMMODFUNC struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_arr(struct fmc_cfg_sect_item *, const char *,
                          struct fmc_cfg_arr_item *, fmc_error_t **err);
FMMODFUNC void fmc_cfg_arr_del(struct fmc_cfg_arr_item *);
FMMODFUNC struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_none(struct fmc_cfg_arr_item *, fmc_error_t **err);
FMMODFUNC struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_boolean(struct fmc_cfg_arr_item *, bool,
                             fmc_error_t **err);
FMMODFUNC struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_int64(struct fmc_cfg_arr_item *, int64_t,
                           fmc_error_t **err);
FMMODFUNC struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_float64(struct fmc_cfg_arr_item *, double,
                             fmc_error_t **err);
FMMODFUNC struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_str(struct fmc_cfg_arr_item *, const char *,
                         fmc_error_t **err);
FMMODFUNC struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_sect(struct fmc_cfg_arr_item *, struct fmc_cfg_sect_item *,
                          fmc_error_t **err);
FMMODFUNC struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_arr(struct fmc_cfg_arr_item *, struct fmc_cfg_arr_item *,
                         fmc_error_t **err);

FMMODFUNC struct fmc_cfg_sect_item *
fmc_cfg_sect_parse_ini_file(struct fmc_cfg_node_spec *spec, fmc_fd fd,
                            const char *root_key, fmc_error_t **err);
FMMODFUNC struct fmc_cfg_sect_item *
fmc_cfg_sect_parse_json(struct fmc_cfg_node_spec *spec, const char *buffer,
                        size_t sz, fmc_error_t **err);
FMMODFUNC void fmc_cfg_node_spec_check(struct fmc_cfg_node_spec *spec,
                                       struct fmc_cfg_sect_item *cfg,
                                       fmc_error_t **err);

FMMODFUNC const char *fmc_cfg_type_name(FMC_CFG_TYPE type);

FMMODFUNC struct fmc_cfg_sect_item *
fmc_cfg_sect_item_get(struct fmc_cfg_sect_item *cfg, const char *key);

FMMODFUNC struct fmc_cfg_arr_item *fmc_cfg_arr_item_new(fmc_error_t **err);

FMMODFUNC struct fmc_cfg_sect_item *fmc_cfg_sect_item_new(fmc_error_t **err);

#ifdef __cplusplus
}
#endif
