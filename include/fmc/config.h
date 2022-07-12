/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
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
 * @author Ivan Gonzalez
 * @date 11 Jul 2022
 * @brief Configuration for components
 *
 * @see http://www.featuremine.com
 */

// TODO : pragma?
#ifndef CONFIG_H__
#define CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define fmc_comp_HEAD  \
   fmc_comp_type *_vt; \
   fmc_error _err

// int32_t
typedef enum {
  FMC_CFG_NONE,             /*  0 */
  FMC_CFG_BOOLEAN,          /*  1 */
  FMC_CFG_INT64,            /*  2 */
  FMC_CFG_FLOAT64,          /*  3 */
  FMC_CFG_STR,              /*  4 */
  FMC_CFG_SECT,             /*  5 */
  FMC_CFG_ARR               /*  6 */
} FMC_CFG_TYPE;


typedef struct fmc_cfg_arr_spec fmc_cfg_arr_spec;
typedef struct fmc_cfg_node_spec fmc_cfg_node_spec;
typedef struct fmc_cfg_node fmc_cfg_node;
typedef struct fmc_cfg_sect fmc_cfg_sect;
typedef struct fmc_cfg_arr fmc_cfg_arr;

/*
fmc_cfg_node_spec session_cfg_spec[] = {
   {"channel", "YTP channel of the session", FMCCFGSTRING, true, NULL},
   {NULL}
}

fmc_cfg_arr_spec sessions_cfg_sect_spec = {FMCCFGSECT, &session_cfg_spec};
fmc_cfg_arr_spec sessions_cfg_arr_spec = {FMCCFGSTRING, NULL};

fmc_cfg_node_spec gateway_cfg_spec[] = {
   {"sessions", "Array of individual session configuration", FMCCFGARRAY, true, &sessions_cfg_spec},
   {NULL}
};
*/

// Array or section
struct fmc_cfg_arr_spec {
   FMC_CFG_TYPE type; // type of the array. if section=FMCCFGSECT.
   fmc_cfg_node_spec *node; // only for sections, NULL for arrays
};

// (Key,value) of [Array, section or single value]
struct fmc_cfg_node_spec {
   const char *name; // Key
   const char *descr;
   FMC_CFG_TYPE type; // array(FMCCFGARRAY), section(FMCCFGSECT) or single value
   bool required;
   union {
    fmc_cfg_arr_spec *array; // Array
    fmc_cfg_node_spec *node; // section
   } subnode;
};

// value
struct fmc_cfg_node {
   union {
      bool boolean;
      int64_t int64;
      double float64;
      const char *str;
      fmc_cfg_sect *sect;
      fmc_cfg_arr *arr;
   } value;
   FMC_CFG_TYPE type;  // could be NONE
};

// Top level
struct fmc_cfg_sect {
   const char *name; // key
   fmc_cfg_node *node; // value
   fmc_cfg_sect *next;
};

// Array values
struct fmc_cfg_arr {
   fmc_cfg_node *node; // value
   fmc_cfg_sect *next;
};



#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H__ */