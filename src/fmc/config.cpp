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
#include <fmc/uthash/utlist.h>

#include <string.h>
#include <stdlib.h>

#define INI_PARSER_BUFF_SIZE 8192

static char *string_copy_len(const char *src, size_t len) {
  void *p = calloc(1, len + 1);
  if (!p) {
    return NULL;
  }
  memcpy(p, src, len);
  return (char *)p;
}

static char *string_copy(const char *src) {
  return string_copy_len(src, strlen(src));
}

void fmc_cfg_sect_del(struct fmc_cfg_sect_item *head) {
  if (head) {
    for (; head; head = head->next) {
      switch (head->node.type) {
      case FMC_CFG_SECT:
        fmc_cfg_sect_del(head->node.value.sect);
        break;
      case FMC_CFG_ARR:
        fmc_cfg_arr_del(head->node.value.arr);
        break;
      case FMC_CFG_STR:
        free((void *) head->node.value.str);
        break;
      case FMC_CFG_NONE:
      case FMC_CFG_BOOLEAN:
      case FMC_CFG_INT64:
      case FMC_CFG_FLOAT64: break;
      }
    }
    free(head);
  }
}

static struct fmc_cfg_sect_item *new_sect_item() {
  return (struct fmc_cfg_sect_item *)calloc(1, sizeof(fmc_cfg_sect_item));
}

struct fmc_cfg_sect_item *fmc_cfg_sect_item_add_none(struct fmc_cfg_sect_item * tail, const char * key) {
  struct fmc_cfg_sect_item *item = new_sect_item();
  if (!item) {
    return NULL;
  }
  item->key = string_copy(key);
  item->node.type = FMC_CFG_NONE;
  item->next = tail;
  return item;
}

struct fmc_cfg_sect_item *fmc_cfg_sect_item_add_boolean(struct fmc_cfg_sect_item * tail, const char * key, bool new_value) {
  struct fmc_cfg_sect_item *item = new_sect_item();
  if (!item) {
    return NULL;
  }
  item->key = string_copy(key);
  item->node.type = FMC_CFG_BOOLEAN;
  item->node.value.boolean = new_value;
  item->next = tail;
  return item;
}
struct fmc_cfg_sect_item *fmc_cfg_sect_item_add_int64(struct fmc_cfg_sect_item * tail, const char * key, int64_t new_value) {
  struct fmc_cfg_sect_item *item = new_sect_item();
  if (!item) {
    return NULL;
  }
  item->key = string_copy(key);
  item->node.type = FMC_CFG_INT64;
  item->node.value.int64 = new_value;
  item->next = tail;
  return item;
}
struct fmc_cfg_sect_item *fmc_cfg_sect_item_add_float64(struct fmc_cfg_sect_item * tail, const char * key, double new_value) {
  struct fmc_cfg_sect_item *item = new_sect_item();
  if (!item) {
    return NULL;
  }
  item->key = string_copy(key);
  item->node.type = FMC_CFG_FLOAT64;
  item->node.value.float64 = new_value;
  item->next = tail;
  return item;
}
struct fmc_cfg_sect_item *fmc_cfg_sect_item_add_str(struct fmc_cfg_sect_item * tail, const char * key, const char * new_value) {
  struct fmc_cfg_sect_item *item = new_sect_item();
  if (!item) {
    return NULL;
  }
  item->key = string_copy(key);
  item->node.type = FMC_CFG_STR;
  item->node.value.str = string_copy(new_value);
  item->next = tail;
  return item;
}
struct fmc_cfg_sect_item *fmc_cfg_sect_item_add_sect(struct fmc_cfg_sect_item * tail, const char * key, struct fmc_cfg_sect_item * new_value) {
  struct fmc_cfg_sect_item *item = new_sect_item();
  if (!item) {
    return NULL;
  }
  item->key = string_copy(key);
  item->node.type = FMC_CFG_SECT;
  item->node.value.sect = new_value;
  item->next = tail;
  return item;
}
struct fmc_cfg_sect_item *fmc_cfg_sect_item_add_arr(struct fmc_cfg_sect_item * tail, const char * key, struct fmc_cfg_arr_item * new_value) {
  struct fmc_cfg_sect_item *item = new_sect_item();
  if (!item) {
    return NULL;
  }
  item->key = string_copy(key);
  item->node.type = FMC_CFG_ARR;
  item->node.value.arr = new_value;
  item->next = tail;
  return item;
}

void fmc_cfg_arr_del(struct fmc_cfg_arr_item *head) {
  if (head) {
    for (; head; head = head->next) {
      switch (head->item.type) {
      case FMC_CFG_SECT:
        fmc_cfg_sect_del(head->item.value.sect);
        break;
      case FMC_CFG_ARR:
        fmc_cfg_arr_del(head->item.value.arr);
        break;
      case FMC_CFG_STR:
        free((void *) head->item.value.str);
        break;
      case FMC_CFG_NONE:
      case FMC_CFG_BOOLEAN:
      case FMC_CFG_INT64:
      case FMC_CFG_FLOAT64: break;
      }
    }
    free(head);
  }
}

static struct fmc_cfg_arr_item *new_arr_item() {
  return (struct fmc_cfg_arr_item *)calloc(1, sizeof(fmc_cfg_arr_item));
}

struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_none(struct fmc_cfg_arr_item * tail) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_NONE;
  item->next = tail;
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_boolean(struct fmc_cfg_arr_item * tail, bool new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_BOOLEAN;
  item->item.value.boolean = new_value;
  item->next = tail;
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_int64(struct fmc_cfg_arr_item * tail, int64_t new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_INT64;
  item->item.value.int64 = new_value;
  item->next = tail;
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_float64(struct fmc_cfg_arr_item * tail, double new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_FLOAT64;
  item->item.value.float64 = new_value;
  item->next = tail;
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_str(struct fmc_cfg_arr_item * tail, const char * new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_STR;
  item->item.value.str = string_copy(new_value);
  item->next = tail;
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_sect(struct fmc_cfg_arr_item * tail, struct fmc_cfg_sect_item * new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_SECT;
  item->item.value.sect = new_value;
  item->next = tail;
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_arr(struct fmc_cfg_arr_item * tail, struct fmc_cfg_arr_item * new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_ARR;
  item->item.value.arr = new_value;
  item->next = tail;
  return item;
}

struct ini_field {
  char *key;
  char *val;
  size_t line;
  bool used;
  struct ini_field *next;
};

struct ini_sect {
  char *key;
  struct ini_field *fields;
  size_t line;
  bool used;
  struct ini_sect *next;
};

static struct ini_sect *ini_sect_new() {
  return (struct ini_sect *)calloc(1, sizeof(struct ini_sect));
}

static struct ini_field *ini_field_new() {
  return (struct ini_field *)calloc(1, sizeof(struct ini_field));
}

static void ini_field_del(struct ini_field *field) {
  if (field) {
    free(field->key);
    free(field->val);
    free(field);
  }
}

static void ini_sect_del(struct ini_sect *sect) {
  if (sect) {
    struct ini_field *field = sect->fields;
    while (field) {
      struct ini_field *next = field->next;
      ini_field_del(field);
      field = next;
    }
    free(sect->key);
    free(sect);
  }
}

static struct fmc_cfg_arr_item *parse_array(struct ini_sect *ini, struct fmc_cfg_type *spec, char **str, char *end, fmc_error_t **err);
static struct fmc_cfg_sect_item *parse_section(struct ini_sect *ini, struct fmc_cfg_node_spec *spec, char *name, size_t len, fmc_error_t **err);
static void parse_value(struct ini_sect *ini, struct fmc_cfg_type *spec, char **str, char *end, fmc_cfg_item *out, fmc_error_t **err) {
  out->type = FMC_CFG_NONE;
  switch (spec->type) {
  case FMC_CFG_NONE: {
    if (memcmp(*str, "none", 4) == 0) {
      out->type = FMC_CFG_NONE;
      *str += 4;
    }
    else {
      fmc_error_set(err, "Error while parsing config file: unable to parse none in array");
      return;
    }
  } break;
  case FMC_CFG_BOOLEAN: {
    if (memcmp(*str, "false", 5) == 0) {
      *str += 5;
      out->type = FMC_CFG_BOOLEAN;
      out->value.boolean = false;
    }
    else if (memcmp(*str, "true", 4) == 0) {
      *str += 4;
      out->type = FMC_CFG_BOOLEAN;
      out->value.boolean = true;
    }
    else {
      fmc_error_set(err, "Error while parsing config file: unable to parse boolean in array");
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
    }
    else {
      fmc_error_set(err, "Error while parsing config file: unable to parse int64 in array");
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
    }
    else {
      fmc_error_set(err, "Error while parsing config file: unable to parse float64 in array");
      return;
    }
  } break;
  case FMC_CFG_STR: {
    if (**str != '"') {
      fmc_error_set(err, "Error while parsing config file: invalid string");
      return;
    }
    ++*str;
    char *endptr = *str;
    while(endptr < end && *endptr != '"') {
      ++endptr;
    }
    if (endptr == end) {
      fmc_error_set(err, "Error while parsing config file: unable to find closing quotes for string in array");
      return;
    }
    out->type = FMC_CFG_STR;
    out->value.str = string_copy_len(*str, endptr - *str);
    *str = endptr + 1;
  } break;
  case FMC_CFG_SECT: {
    char *endptr = *str;
    while(endptr < end && *endptr != ',' && *endptr != ']') {
      ++endptr;
    }
    struct fmc_cfg_sect_item *sect = parse_section(ini, spec->spec.node, *str, endptr - *str, err);
    if (*err) {
      return;
    }
    *str = endptr;
    out->type = FMC_CFG_SECT;
    out->value.sect = sect;
  } break;
  case FMC_CFG_ARR: {
    struct fmc_cfg_arr_item *subarr = parse_array(ini, spec->spec.array, str, end, err);
    if (*err) {
      return;
    }
    out->type = FMC_CFG_ARR;
    out->value.arr = subarr;
  } break;
  }
}

static struct fmc_cfg_arr_item *parse_array_unwrapped(struct ini_sect *ini, struct fmc_cfg_type *spec, char **str, char *end, fmc_error_t **err) {
  fmc_cfg_arr_item *arr = NULL;
  if (**str == ',') {
    ++*str;
    return arr;
  }
  else if (**str == ']') {
    return arr;
  }

  while(*str < end) {
    struct fmc_cfg_arr_item *item = new_arr_item();
    LL_APPEND(arr, item);
    parse_value(ini, spec, str, end, &item->item, err);
    if (err) {
      goto do_cleanup;
    }
    if(*str == end) {
      break;
    }
    else if (**str == ',') {
      ++*str;
    }
    else if (**str == ']') {
      ++*str;
      break;
    }
    else {
      fmc_error_set(err, "Error while parsing config file: comma was expected in array");
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

static struct fmc_cfg_arr_item *parse_array(struct ini_sect *ini, struct fmc_cfg_type *spec, char **str, char *end, fmc_error_t **err) {
  if (**str == '[') {
    ++*str;
    struct fmc_cfg_arr_item *arr = parse_array_unwrapped(ini, spec, str, end, err);
    if (**str == ']') {
      ++*str;
    }
    else {
      fmc_error_set(err, "Error while parsing config file: closing bracket was expected in array");
      fmc_cfg_arr_del(arr);
      return NULL;
    }
  }
  else {
    return parse_array_unwrapped(ini, spec, str, end, err);
  }
}

static struct fmc_cfg_sect_item *parse_section(struct ini_sect *ini, struct fmc_cfg_node_spec *spec, char *name, size_t len, fmc_error_t **err) {
  struct ini_sect *isect;
  struct fmc_cfg_sect_item *sect = NULL;
  for (isect = ini; isect; ++isect) {
    if (memcmp(isect->key, name, len) == 0 && isect->key[len] == '\0') {
      break;
    }
  }

  if (!isect || isect->used) {
    char prev = name[len];
    name[len] = '\0';
    fmc_error_set(err, "Error while parsing config file: section %s not found", name);
    name[len] = prev;
    return NULL;
  }

  for (struct fmc_cfg_node_spec *spec_item = spec; spec_item->key; ++spec_item) {
    struct ini_field *item = NULL;
    for (item = isect->fields; item; item = item->next) {
      if (strcmp(spec_item->key, item->key) == 0) {
        break;
      }
    }
    if (!item || item->used) {
      if (spec_item->required) {
        fmc_error_set(err, "Error while parsing config file: missing required field %s", spec_item->key);
        goto do_cleanup;
      }
      else {
        continue;
      }
    }

    struct fmc_cfg_sect_item *sitem = new_sect_item();
    LL_APPEND(sect, sitem);
    char *str = item->val;
    char *end = item->val + strlen(item->val);
    parse_value(ini, &spec_item->type, &str, end, &sitem->node, err);
    if (err) {
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

static void ini_line_parse(parser_state_t *state, char *line, size_t sz, fmc_error_t **error) {
  ++state->line_n;
  if (sz == 0) {
    return;
  }

  char *key = NULL;
  char *value = NULL;

  if (line[0] == '[' && line[sz - 1] == ']') {
    key = string_copy_len(&line[1], sz - 2);
    if (!key) {
      goto do_oom;
    }

    struct ini_sect *item = ini_sect_new();
    if (!item) {
      goto do_oom;
    }

    item->key = key;
    item->used = false;
    item->line = state->line_n;

    LL_APPEND(state->sections, item);
  }
  else {
    if (!state->sections) {
      fmc_error_set(error, "Configuration entry doesn't have a section in the file (line %zu)", state->line_n);
      return;
    }

    size_t sep;
    for (sep = 0; line[sep] != '=' && sep < sz; ++sep);
    if (sep >= sz) {
      fmc_error_set(error, "Invalid configuration file key-value entry (line %zu)", state->line_n);
      return;
    }

    key = string_copy_len(&line[0], sep);
    if (!key) {
      goto do_oom;
    }

    value = string_copy_len(&line[sep + 1], sz - sep - 1);
    if (!value) {
      goto do_oom;
    }

    struct ini_field *item = ini_field_new();
    if (!item) {
      goto do_oom;
    }
    item->key = key;
    item->val = value;
    item->used = false;
    item->line = state->line_n;

    LL_APPEND(state->sections->fields, item);
  }

  do_cleanup:
  free(key);
  free(value);
  return;

  do_oom:
  fmc_error_set(error, "Out of memory");
  goto do_cleanup;
}

static struct ini_sect *ini_file_parse(fmc_fd fd, const char *root_key, fmc_error_t **err) {
  parser_state_t state {NULL, 0};
  char buffer[INI_PARSER_BUFF_SIZE];

  size_t read = 0;
  while (true) {
    if (INI_PARSER_BUFF_SIZE == read) {
      fmc_error_set(err, "Error while parsing config file: line is too long");
      goto do_cleanup;
    }

    auto ret = fmc_fread(fd, buffer + read, INI_PARSER_BUFF_SIZE - read, err);
    if (*err) {
      goto do_cleanup;
    }
    if (ret == 0) {
      if (read > 0 && buffer[read - 1] == '\n') {
        --read;
        if (read > 0 && buffer[read - 1] == '\r') {
          --read;
        }
        while (read > 0 && buffer[read - 1] == ' ') {
          --read;
        }
      }
      fmc_error_clear(err);
      ini_line_parse(&state, buffer, read, err);
      if (*err) {
        goto do_cleanup;
      }

      return state.sections;
    }
    size_t line_start = 0;
    auto start = read;
    auto end = read + ret;
    for (auto i = start; i < end; ++i) {
      if (buffer[i] == '\n') {
        auto j = i;
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

struct fmc_cfg_sect_item *fmc_cfg_sect_parse_ini_file(struct fmc_cfg_node_spec *spec, fmc_fd fd, const char *root_key, fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *ret = NULL;
  struct fmc_cfg_sect_item *sect = NULL;

  size_t root_key_len = strlen(root_key);
  char root_key_copy[root_key_len + 1];
  strcpy(root_key_copy, root_key);

  struct ini_sect *ini = ini_file_parse(fd, root_key, err);
  if (*err) {
    goto do_cleanup;
  }

  sect = parse_section(ini, spec, root_key_copy, root_key_len, err);
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
