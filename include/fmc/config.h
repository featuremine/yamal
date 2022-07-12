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
  FMC_TYPE_POSITIVE_FIXNUM, /*  0 */
  FMC_TYPE_FIXMAP,          /*  1 */
  FMC_TYPE_FIXARRAY,        /*  2 */
  FMC_TYPE_FIXSTR,          /*  3 */
  FMC_TYPE_NIL,             /*  4 */
  FMC_TYPE_BOOLEAN,         /*  5 */
  FMC_TYPE_BIN8,            /*  6 */
  FMC_TYPE_BIN16,           /*  7 */
  FMC_TYPE_BIN32,           /*  8 */
  FMC_TYPE_EXT8,            /*  9 */
  FMC_TYPE_EXT16,           /* 10 */
  FMC_TYPE_EXT32,           /* 11 */
  FMC_TYPE_FLOAT,           /* 12 */
  FMC_TYPE_DOUBLE,          /* 13 */
  FMC_TYPE_UINT8,           /* 14 */
  FMC_TYPE_UINT16,          /* 15 */
  FMC_TYPE_UINT32,          /* 16 */
  FMC_TYPE_UINT64,          /* 17 */
  FMC_TYPE_SINT8,           /* 18 */
  FMC_TYPE_SINT16,          /* 19 */
  FMC_TYPE_SINT32,          /* 20 */
  FMC_TYPE_SINT64,          /* 21 */
  FMC_TYPE_FIXEXT1,         /* 22 */
  FMC_TYPE_FIXEXT2,         /* 23 */
  FMC_TYPE_FIXEXT4,         /* 24 */
  FMC_TYPE_FIXEXT8,         /* 25 */
  FMC_TYPE_FIXEXT16,        /* 26 */
  FMC_TYPE_STR8,            /* 27 */
  FMC_TYPE_STR16,           /* 28 */
  FMC_TYPE_STR32,           /* 29 */
  FMC_TYPE_ARRAY16,         /* 30 */
  FMC_TYPE_ARRAY32,         /* 31 */
  FMC_TYPE_MAP16,           /* 32 */
  FMC_TYPE_MAP32,           /* 33 */
  FMC_TYPE_NEGATIVE_FIXNUM  /* 34 */
} FMCCFGTYPE;


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
   FMCCFGTYPE type; // type of the array. if section=FMCCFGSECT.
   fmc_cfg_node_spec *node; // only for sections, NULL for arrays
};

// (Key,value) of [Array, section or single value]
struct fmc_cfg_node_spec {
   const char *name; // Key
   const char *descr;
   FMCCFGTYPE type; // array, section, single value // TODO: only FMCCFGARRAY(FMCCFGMULTIVALUE), or FMCCFGARRAY/FMCCFGSECT ??
   bool required;
   fmc_cfg_arr_spec *array; // Array or section. NULL in case of single value
};

// value
struct fmc_cfg_node {
   union {
      const char *str;
      double real;
      fmc_cfg_sect* sect;
      fmc_cfg_arr *arr;
   } value;
   FMCCFGTYPE type;   
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