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
 * @file config.cpp
 * @date 13 Jul 2022
 * @brief Implementation of the fmc configuration API
 *
 * @see http://www.featuremine.com
 */

#include <fmc/config.h>
#include <fmc/string.h>
#include <uthash/utlist.h>

#include <stdlib.h>
#include <string.h>

#define INI_PARSER_BUFF_SIZE 8192

static void fmc_cfg_item_destroy(struct fmc_cfg_item *node) {
  switch (node->type) {
  case FMC_CFG_SECT:
    fmc_cfg_sect_del(node->value.sect);
    break;
  case FMC_CFG_ARR:
    fmc_cfg_arr_del(node->value.arr);
    break;
  case FMC_CFG_STR:
    free((void *)node->value.str);
    break;
  case FMC_CFG_NONE:
  case FMC_CFG_BOOLEAN:
  case FMC_CFG_INT64:
  case FMC_CFG_FLOAT64:
    break;
  }
}

void fmc_cfg_sect_del(struct fmc_cfg_sect_item *head) {
  while (head) {
    fmc_cfg_item_destroy(&head->node);
    struct fmc_cfg_sect_item *next = head->next;
    free((void *)head->key);
    free(head);
    head = next;
  }
}

static struct fmc_cfg_sect_item *fmc_cfg_sect_item_new(fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *ret =
      (struct fmc_cfg_sect_item *)calloc(1, sizeof(struct fmc_cfg_sect_item));
  if (!ret) {
    fmc_error_set2(err, FMC_ERROR_MEMORY);
  }
  ret->node.type = FMC_CFG_NONE;
  return ret;
}

struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_none(struct fmc_cfg_sect_item *tail, const char *key,
                           fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->key = fmc_cstr_new(key, err);
  if (*err) {
    goto do_cleanup;
  }
  item->node.type = FMC_CFG_NONE;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_sect_del(item);
  return NULL;
}

struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_boolean(struct fmc_cfg_sect_item *tail, const char *key,
                              bool new_value, fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->key = fmc_cstr_new(key, err);
  if (*err) {
    goto do_cleanup;
  }
  item->node.type = FMC_CFG_BOOLEAN;
  item->node.value.boolean = new_value;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_sect_del(item);
  return NULL;
}
struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_int64(struct fmc_cfg_sect_item *tail, const char *key,
                            int64_t new_value, fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->key = fmc_cstr_new(key, err);
  if (*err) {
    goto do_cleanup;
  }
  item->node.type = FMC_CFG_INT64;
  item->node.value.int64 = new_value;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_sect_del(item);
  return NULL;
}
struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_float64(struct fmc_cfg_sect_item *tail, const char *key,
                              double new_value, fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->key = fmc_cstr_new(key, err);
  if (*err) {
    goto do_cleanup;
  }
  item->node.type = FMC_CFG_FLOAT64;
  item->node.value.float64 = new_value;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_sect_del(item);
  return NULL;
}
struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_str(struct fmc_cfg_sect_item *tail, const char *key,
                          const char *new_value, fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->key = fmc_cstr_new(key, err);
  if (*err) {
    goto do_cleanup;
  }
  item->node.type = FMC_CFG_STR;
  item->node.value.str = fmc_cstr_new(new_value, err);
  if (*err) {
    goto do_cleanup;
  }
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_sect_del(item);
  return NULL;
}
struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_sect(struct fmc_cfg_sect_item *tail, const char *key,
                           struct fmc_cfg_sect_item *new_value,
                           fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->key = fmc_cstr_new(key, err);
  if (*err) {
    goto do_cleanup;
  }
  item->node.type = FMC_CFG_SECT;
  item->node.value.sect = new_value;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_sect_del(item);
  return NULL;
}
struct fmc_cfg_sect_item *
fmc_cfg_sect_item_add_arr(struct fmc_cfg_sect_item *tail, const char *key,
                          struct fmc_cfg_arr_item *new_value,
                          fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->key = fmc_cstr_new(key, err);
  if (*err) {
    goto do_cleanup;
  }
  item->node.type = FMC_CFG_ARR;
  item->node.value.arr = new_value;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_sect_del(item);
  return NULL;
}

void fmc_cfg_arr_del(struct fmc_cfg_arr_item *head) {
  while (head) {
    fmc_cfg_item_destroy(&head->item);
    struct fmc_cfg_arr_item *next = head->next;
    free(head);
    head = next;
  }
}

static struct fmc_cfg_arr_item *fmc_cfg_arr_item_new(fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_arr_item *ret =
      (struct fmc_cfg_arr_item *)calloc(1, sizeof(struct fmc_cfg_arr_item));
  if (!ret) {
    fmc_error_set2(err, FMC_ERROR_MEMORY);
  }
  ret->item.type = FMC_CFG_NONE;
  return ret;
}

struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_none(struct fmc_cfg_arr_item *tail, fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_arr_item *item = fmc_cfg_arr_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->item.type = FMC_CFG_NONE;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_arr_del(item);
  return NULL;
}
struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_boolean(struct fmc_cfg_arr_item *tail, bool new_value,
                             fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_arr_item *item = fmc_cfg_arr_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->item.type = FMC_CFG_BOOLEAN;
  item->item.value.boolean = new_value;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_arr_del(item);
  return NULL;
}
struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_int64(struct fmc_cfg_arr_item *tail, int64_t new_value,
                           fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_arr_item *item = fmc_cfg_arr_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->item.type = FMC_CFG_INT64;
  item->item.value.int64 = new_value;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_arr_del(item);
  return NULL;
}
struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_float64(struct fmc_cfg_arr_item *tail, double new_value,
                             fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_arr_item *item = fmc_cfg_arr_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->item.type = FMC_CFG_FLOAT64;
  item->item.value.float64 = new_value;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_arr_del(item);
  return NULL;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_str(struct fmc_cfg_arr_item *tail,
                                                  const char *new_value,
                                                  fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_arr_item *item = fmc_cfg_arr_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->item.type = FMC_CFG_STR;
  item->item.value.str = fmc_cstr_new(new_value, err);
  if (*err) {
    goto do_cleanup;
  }
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_arr_del(item);
  return NULL;
}
struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_sect(struct fmc_cfg_arr_item *tail,
                          struct fmc_cfg_sect_item *new_value,
                          fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_arr_item *item = fmc_cfg_arr_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->item.type = FMC_CFG_SECT;
  item->item.value.sect = new_value;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_arr_del(item);
  return NULL;
}
struct fmc_cfg_arr_item *
fmc_cfg_arr_item_add_arr(struct fmc_cfg_arr_item *tail,
                         struct fmc_cfg_arr_item *new_value,
                         fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_arr_item *item = fmc_cfg_arr_item_new(err);
  if (*err) {
    goto do_cleanup;
  }
  item->item.type = FMC_CFG_ARR;
  item->item.value.arr = new_value;
  item->next = tail;
  return item;

do_cleanup:
  fmc_cfg_arr_del(item);
  return NULL;
}

struct ini_field {
  char *key;
  char *val;
  size_t line;
  bool used;
  struct ini_field *next;
};

static struct ini_field *ini_field_new(fmc_error_t **err) {
  fmc_error_clear(err);
  struct ini_field *ret =
      (struct ini_field *)calloc(1, sizeof(struct ini_field));
  if (!ret) {
    fmc_error_set2(err, FMC_ERROR_MEMORY);
  }
  return ret;
}

static void ini_field_del(struct ini_field *field) {
  while (field) {
    struct ini_field *next = field->next;
    free(field->key);
    free(field->val);
    free(field);
    field = next;
  }
}

struct ini_sect {
  char *key;
  struct ini_field *fields;
  size_t line;
  bool used;
  struct ini_sect *next;
};

static struct ini_sect *ini_sect_new(fmc_error_t **err) {
  fmc_error_clear(err);

  struct ini_sect *ret = (struct ini_sect *)calloc(1, sizeof(struct ini_sect));
  if (!ret) {
    fmc_error_set2(err, FMC_ERROR_MEMORY);
  }
  return ret;
}

static void ini_sect_del(struct ini_sect *sect) {
  while (sect) {
    struct ini_sect *next = sect->next;
    ini_field_del(sect->fields);
    free(sect->key);
    free(sect);
    sect = next;
  }
}

static struct fmc_cfg_arr_item *
parse_array_unwrapped(struct ini_sect *ini, struct fmc_cfg_type *spec,
                      char **str, char *end, size_t line, fmc_error_t **err);
static struct fmc_cfg_arr_item *parse_array(struct ini_sect *ini,
                                            struct fmc_cfg_type *spec,
                                            char **str, char *end, size_t line,
                                            fmc_error_t **err);
static struct fmc_cfg_sect_item *parse_section(struct ini_sect *ini,
                                               struct fmc_cfg_node_spec *spec,
                                               char *name, size_t len,
                                               size_t line, fmc_error_t **err);
static void parse_value(struct ini_sect *ini, struct fmc_cfg_type *spec,
                        char **str, char *end, size_t line,
                        struct fmc_cfg_item *out, fmc_error_t **err);

static void parse_value(struct ini_sect *ini, struct fmc_cfg_type *spec,
                        char **str, char *end, size_t line,
                        struct fmc_cfg_item *out, fmc_error_t **err) {
  fmc_error_clear(err);

  out->type = FMC_CFG_NONE;
  switch (spec->type) {
  case FMC_CFG_NONE: {
    if (memcmp(*str, "none", 4) == 0) {
      out->type = FMC_CFG_NONE;
      *str += 4;
    } else {
      fmc_error_set(err, "config error: unable to parse none (line %zu)", line);
      return;
    }
  } break;
  case FMC_CFG_BOOLEAN: {
    if (memcmp(*str, "false", 5) == 0) {
      *str += 5;
      out->type = FMC_CFG_BOOLEAN;
      out->value.boolean = false;
    } else if (memcmp(*str, "true", 4) == 0) {
      *str += 4;
      out->type = FMC_CFG_BOOLEAN;
      out->value.boolean = true;
    } else {
      fmc_error_set(err, "config error: unable to parse boolean (line %zu)",
                    line);
      return;
    }
  } break;
  case FMC_CFG_INT64: {
    char *endptr;
    int64_t value = strtoll(*str, &endptr, 10);
    if (endptr != *str) {
      *str = endptr;
      out->type = FMC_CFG_INT64;
      out->value.int64 = value;
    } else {
      fmc_error_set(err, "config error: unable to parse int64 (line %zu)",
                    line);
      return;
    }
  } break;
  case FMC_CFG_FLOAT64: {
    char *endptr;
    double value = strtod(*str, &endptr);
    if (endptr != *str) {
      *str = endptr;
      out->type = FMC_CFG_FLOAT64;
      out->value.float64 = value;
    } else {
      fmc_error_set(err, "config error: unable to parse float64 (line %zu)",
                    line);
      return;
    }
  } break;
  case FMC_CFG_STR: {
    if (**str != '"') {
      fmc_error_set(err, "config error: unable to parse string (line %zu)",
                    line);
      return;
    }
    ++*str;
    char *endptr = *str;
    while (endptr < end && *endptr != '"') {
      ++endptr;
    }
    if (endptr == end) {
      fmc_error_set(
          err,
          "config error: unable to find closing quotes for string (line %zu)",
          line);
      return;
    }
    out->type = FMC_CFG_STR;
    out->value.str = fmc_cstr_new2(*str, endptr - *str, err);
    if (*err) {
      return;
    }
    *str = endptr + 1;
  } break;
  case FMC_CFG_SECT: {
    char *endptr = *str;
    while (endptr < end && *endptr != ',' && *endptr != ']') {
      ++endptr;
    }
    struct fmc_cfg_sect_item *sect =
        parse_section(ini, spec->spec.node, *str, endptr - *str, line, err);
    if (*err) {
      return;
    }
    *str = endptr;
    out->type = FMC_CFG_SECT;
    out->value.sect = sect;
  } break;
  case FMC_CFG_ARR: {
    struct fmc_cfg_arr_item *subarr =
        parse_array(ini, spec->spec.array, str, end, line, err);
    if (*err) {
      return;
    }
    out->type = FMC_CFG_ARR;
    out->value.arr = subarr;
  } break;
  }
}

static struct fmc_cfg_arr_item *
parse_array_unwrapped(struct ini_sect *ini, struct fmc_cfg_type *spec,
                      char **str, char *end, size_t line, fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_arr_item *arr = NULL;
  if (**str == ',') {
    ++*str;
    return arr;
  } else if (**str == ']') {
    return arr;
  }

  while (*str < end) {
    struct fmc_cfg_arr_item *item = fmc_cfg_arr_item_new(err);
    if (*err) {
      goto do_cleanup;
    }
    LL_PREPEND(arr, item);
    parse_value(ini, spec, str, end, line, &item->item, err);
    if (*err) {
      goto do_cleanup;
    }
    if (*str == end) {
      break;
    } else if (**str == ',') {
      ++*str;
    } else if (**str == ']') {
      break;
    } else {
      fmc_error_set(err, "config error: comma was expected in array (line %zu)",
                    line);
      goto do_cleanup;
    }
  }

  if (arr) {
    struct fmc_cfg_arr_item *prev = NULL;
    while (arr->next) {
      struct fmc_cfg_arr_item *next = arr->next;
      arr->next = prev;
      prev = arr;
      arr = next;
    }
    arr->next = prev;
  }
  return arr;

do_cleanup:
  fmc_cfg_arr_del(arr);
  return NULL;
}

static struct fmc_cfg_arr_item *parse_array(struct ini_sect *ini,
                                            struct fmc_cfg_type *spec,
                                            char **str, char *end, size_t line,
                                            fmc_error_t **err) {
  fmc_error_clear(err);

  if (**str == '[') {
    ++*str;
    struct fmc_cfg_arr_item *arr =
        parse_array_unwrapped(ini, spec, str, end, line, err);
    if (*err) {
      return NULL;
    }
    if (**str == ']') {
      ++*str;
      return arr;
    } else {
      fmc_error_set(
          err, "config error: closing bracket was expected in array (line %zu)",
          line);
      fmc_cfg_arr_del(arr);
      return NULL;
    }
  } else {
    return parse_array_unwrapped(ini, spec, str, end, line, err);
  }
}

static struct fmc_cfg_sect_item *parse_section(struct ini_sect *ini,
                                               struct fmc_cfg_node_spec *spec,
                                               char *name, size_t len,
                                               size_t line, fmc_error_t **err) {
  fmc_error_clear(err);

  struct ini_sect *isect;
  struct fmc_cfg_sect_item *sect = NULL;
  for (isect = ini; isect; isect = isect->next) {
    if (memcmp(isect->key, name, len) == 0 && isect->key[len] == '\0') {
      break;
    }
  }

  if (!isect || isect->used) {
    char prev = name[len];
    name[len] = '\0';
    fmc_error_set(err, "config error: section %s not found (line %zu)", name,
                  line);
    name[len] = prev;
    return NULL;
  }
  isect->used = true;

  for (struct fmc_cfg_node_spec *spec_item = spec; spec_item->key;
       ++spec_item) {
    struct ini_field *item = NULL;
    for (item = isect->fields; item; item = item->next) {
      if (strcmp(spec_item->key, item->key) == 0) {
        break;
      }
    }
    if (!item || item->used) {
      if (spec_item->required) {
        fmc_error_set(err, "config error: missing required field %s (line %zu)",
                      spec_item->key, isect->line);
        goto do_cleanup;
      } else {
        continue;
      }
    }
    item->used = true;

    struct fmc_cfg_sect_item *sitem = fmc_cfg_sect_item_new(err);
    if (*err) {
      goto do_cleanup;
    }
    LL_PREPEND(sect, sitem);
    sitem->key = fmc_cstr_new(item->key, err);
    if (*err) {
      goto do_cleanup;
    }
    char *str = item->val;
    char *end = item->val + strlen(item->val);
    parse_value(ini, &spec_item->type, &str, end, item->line, &sitem->node,
                err);
    if (*err) {
      goto do_cleanup;
    }
    if (str != end) {
      fmc_error_set(err, "config error: unable to parse field %s (line %zu)",
                    item->key, item->line);
      goto do_cleanup;
    }
  }

  for (struct ini_field *item = isect->fields; item; item = item->next) {
    if (!item->used) {
      fmc_error_set(err, "config error: unknown field %s (line %zu)", item->key,
                    item->line);
      goto do_cleanup;
    }
  }

  return sect;

do_cleanup:
  fmc_cfg_sect_del(sect);
  return NULL;
}

struct parser_state_t {
  struct ini_sect *sections;
  size_t line_n;
};

static void ini_line_parse(struct parser_state_t *state, char *line, size_t sz,
                           fmc_error_t **err) {
  fmc_error_clear(err);

  ++state->line_n;
  if (sz == 0) {
    return;
  }

  char *key = NULL;
  char *value = NULL;

  if (line[0] == '[' && line[sz - 1] == ']') {
    key = fmc_cstr_new2(&line[1], sz - 2, err);
    if (*err) {
      goto do_cleanup;
    }

    struct ini_sect *item = ini_sect_new(err);
    if (*err) {
      goto do_cleanup;
    }

    item->key = key;
    item->used = false;
    item->line = state->line_n;

    LL_PREPEND(state->sections, item);
  } else {
    if (!state->sections) {
      fmc_error_set(err, "config error: key-value has no section (line %zu)",
                    state->line_n);
      return;
    }

    size_t sep;
    for (sep = 0; line[sep] != '=' && sep < sz; ++sep)
      ;
    if (sep >= sz) {
      fmc_error_set(err, "config error: invalid key-value entry (line %zu)",
                    state->line_n);
      return;
    }

    key = fmc_cstr_new2(&line[0], sep, err);
    if (*err) {
      goto do_cleanup;
    }

    value = fmc_cstr_new2(&line[sep + 1], sz - sep - 1, err);
    if (*err) {
      goto do_cleanup;
    }

    struct ini_field *item = ini_field_new(err);
    if (*err) {
      goto do_cleanup;
    }
    item->key = key;
    item->val = value;
    item->used = false;
    item->line = state->line_n;

    LL_PREPEND(state->sections->fields, item);
  }
  return;

do_cleanup:
  free(key);
  free(value);
}

static struct ini_sect *ini_file_parse(fmc_fd fd, fmc_error_t **err) {
  fmc_error_clear(err);

  struct parser_state_t state;
  memset(&state, 0, sizeof(state));
  char buffer[INI_PARSER_BUFF_SIZE];

  size_t read = 0;
  while (true) {
    if (INI_PARSER_BUFF_SIZE == read) {
      fmc_error_set(err, "config error: line %zu is too long", ++state.line_n);
      goto do_cleanup;
    }

    ssize_t ret =
        fmc_fread(fd, buffer + read, INI_PARSER_BUFF_SIZE - read, err);
    if (*err) {
      goto do_cleanup;
    }
    if (ret == 0) {
      while (read > 0 && buffer[read - 1] == ' ') {
        --read;
      }
      fmc_error_clear(err);
      ini_line_parse(&state, buffer, read, err);
      if (*err) {
        goto do_cleanup;
      }

      return state.sections;
    }
    size_t line_start = 0;
    size_t start = read;
    size_t end = read + ret;
    for (size_t i = start; i < end; ++i) {
      if (buffer[i] == '\n') {
        size_t j = i;
        if (j > 0 && buffer[j - 1] == '\r') {
          --j;
        }
        while (j > 0 && buffer[j - 1] == ' ') {
          --j;
        }
        ini_line_parse(&state, buffer + line_start, j - line_start, err);
        if (*err) {
          goto do_cleanup;
        }
        line_start = i + 1;
      }
    }
    read = end - line_start;
    if (line_start != end && line_start > 0) {
      memcpy(buffer, buffer + line_start, read);
    }
  }

do_cleanup:
  ini_sect_del(state.sections);
  return NULL;
}

struct fmc_cfg_sect_item *
fmc_cfg_sect_parse_ini_file(struct fmc_cfg_node_spec *spec, fmc_fd fd,
                            const char *root_key, fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *ret = NULL;
  struct fmc_cfg_sect_item *sect = NULL;

  size_t root_key_len = strlen(root_key);
  char root_key_copy[root_key_len + 1];
  strcpy(root_key_copy, root_key);

  struct ini_sect *ini = ini_file_parse(fd, err);
  if (*err) {
    goto do_cleanup;
  }

  sect = parse_section(ini, spec, root_key_copy, root_key_len, 0, err);
  if (*err) {
    goto do_cleanup;
  }

  ret = sect;
  sect = NULL;

do_cleanup:
  fmc_cfg_sect_del(sect);
  ini_sect_del(ini);
  return ret;
}

const char *fmc_cfg_type_name(FMC_CFG_TYPE type) {
  switch (type) {
  case FMC_CFG_NONE:
    return "none";
  case FMC_CFG_BOOLEAN:
    return "boolean";
  case FMC_CFG_INT64:
    return "int64";
  case FMC_CFG_FLOAT64:
    return "float64";
  case FMC_CFG_STR:
    return "string";
  case FMC_CFG_SECT:
    return "section";
  case FMC_CFG_ARR:
    return "array";
  default:
    return "unknown";
  }
}

static void fmc_cfg_arr_spec_check(struct fmc_cfg_type *spec,
                                   struct fmc_cfg_arr_item *cfg,
                                   fmc_error_t **err);

static void check_value(struct fmc_cfg_type *spec, struct fmc_cfg_item *item,
                        fmc_error_t **err) {
  switch (item->type) {
  case FMC_CFG_NONE:
  case FMC_CFG_BOOLEAN:
  case FMC_CFG_INT64:
  case FMC_CFG_FLOAT64:
  case FMC_CFG_STR:
    break;
  case FMC_CFG_SECT: {
    fmc_cfg_node_spec_check(spec->spec.node, item->value.sect, err);
    if (*err) {
      return;
    }
  } break;
  case FMC_CFG_ARR: {
    fmc_cfg_arr_spec_check(spec->spec.array, item->value.arr, err);
    if (*err) {
      return;
    }
  } break;
  }
}

static void fmc_cfg_arr_spec_check(struct fmc_cfg_type *spec,
                                   struct fmc_cfg_arr_item *cfg,
                                   fmc_error_t **err) {
  fmc_error_clear(err);
  for (; cfg; cfg = cfg->next) {
    if (cfg->item.type != spec->type) {
      fmc_error_set(err, "config error: item in array %s must be %s",
                    fmc_cfg_type_name(cfg->item.type),
                    fmc_cfg_type_name(spec->type));
      return;
    }

    check_value(spec, &cfg->item, err);
    if (*err) {
      return;
    }
  }
}

void fmc_cfg_node_spec_check(struct fmc_cfg_node_spec *spec,
                             struct fmc_cfg_sect_item *cfg, fmc_error_t **err) {
  fmc_error_clear(err);

  size_t used_count = 0;
  for (struct fmc_cfg_node_spec *spec_item = spec; spec_item->key;
       ++spec_item) {
    struct fmc_cfg_sect_item *item = NULL;
    for (item = cfg; item; item = item->next) {
      if (strcmp(spec_item->key, item->key) == 0) {
        break;
      }
    }
    if (!item) {
      if (spec_item->required) {
        fmc_error_set(err, "config error: missing required field %s",
                      spec_item->key);
        return;
      } else {
        continue;
      }
    }
    for (struct fmc_cfg_sect_item *dup = item->next; dup; dup = dup->next) {
      if (strcmp(spec_item->key, dup->key) == 0) {
        fmc_error_set(err, "config error: duplicated field %s", spec_item->key);
        return;
      }
    }
    ++used_count;

    if (item->node.type != spec_item->type.type) {
      fmc_error_set(err, "config error: field %s (%s) must be %s",
                    spec_item->key, fmc_cfg_type_name(item->node.type),
                    fmc_cfg_type_name(spec_item->type.type));
      return;
    }
    check_value(&spec_item->type, &item->node, err);
    if (*err) {
      return;
    }
  }

  size_t expected_count = 0;
  for (struct fmc_cfg_sect_item *item = cfg; item; item = item->next) {
    ++expected_count;
  }

  if (expected_count != used_count) {
    for (struct fmc_cfg_sect_item *item = cfg; item; item = item->next) {
      struct fmc_cfg_node_spec *spec_item = NULL;
      for (spec_item = spec; spec_item->key; ++spec_item) {
        if (strcmp(spec_item->key, item->key) == 0) {
          break;
        }
      }

      if (!spec_item->key) {
        fmc_error_set(err, "config error: unknown field %s", item->key);
        return;
      }
    }
    fmc_error_set(err, "config error: unknown field");
    return;
  }
}

static int cmp_key(struct fmc_cfg_sect_item *item, const char *key) {
  return strcmp(item->key, key);
}

struct fmc_cfg_sect_item *
fmc_cfg_sect_item_get(struct fmc_cfg_sect_item *cfg,
                      const char *key, fmc_error_t **err) {
  struct fmc_cfg_sect_item *item = NULL;
  LL_SEARCH(cfg, item, key, cmp_key);
  return item;
}