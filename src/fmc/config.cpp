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
  for (; head; head = head->next) {
    switch (head->node.type) {
    case FMC_CFG_SECT: fmc_cfg_sect_del(head->node.value.sect); break;
    case FMC_CFG_ARR: fmc_cfg_arr_del(head->node.value.arr); break;
    case FMC_CFG_STR: free((void *)head->node.value.str); break;
    case FMC_CFG_NONE:
    case FMC_CFG_BOOLEAN:
    case FMC_CFG_INT64:
    case FMC_CFG_FLOAT64: break;
    }
  }
  free(head);
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
  for (; head; head = head->next) {
    switch (head->item.type) {
    case FMC_CFG_SECT: fmc_cfg_sect_del(head->item.value.sect); break;
    case FMC_CFG_ARR: fmc_cfg_arr_del(head->item.value.arr); break;
    case FMC_CFG_STR: free((void *)head->item.value.str); break;
    case FMC_CFG_NONE:
    case FMC_CFG_BOOLEAN:
    case FMC_CFG_INT64:
    case FMC_CFG_FLOAT64: break;
    }
  }
  free(head);
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

struct parser_state_t {
  struct fmc_cfg_sect_item *sections_tail;
  size_t line_n;
};

static void parse_ini_line(parser_state_t *state, char *line, size_t sz, fmc_error_t **error) {
  ++state->line_n;
  if (sz == 0) {
    return;
  }

  if (line[0] == '[' && line[sz - 1] == ']') {
    char *key = string_copy_len(&line[1], sz - 2);
    if (!key) {
      goto do_oom;
    }

    struct fmc_cfg_sect_item *item = new_sect_item();
    if (!item) {
      free(key);
      goto do_oom;
    }
    item->key = key;
    item->next = state->sections_tail;
    item->node.value.sect = NULL;
    item->node.type = FMC_CFG_SECT;

    state->sections_tail = item;
  }
  else {
    if (!state->sections_tail) {
      fmc_error_set(error, "Configuration entry doesn't have a section in the file (line %zu)", state->line_n);
      return;
    }
    size_t sep;
    for (sep = 0; line[sep] != '=' && sep < sz; ++sep);
    if (sep >= sz) {
      fmc_error_set(error, "Invalid configuration file key-value entry (line %zu)", state->line_n);
      return;
    }

    char *key = string_copy_len(&line[0], sep);
    if (!key) {
      goto do_oom;
    }
    char *value = string_copy_len(&line[sep + 1], sz - sep - 1);
    if (!value) {
      free(key);
      goto do_oom;
    }

    struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_add_str(state->sections_tail->node.value.sect, key, value);
    if (!item) {
      free(value);
      free(key);
      goto do_oom;
    }

    state->sections_tail->node.value.sect = item;
  }
  return;

  do_oom:
  fmc_error_set(error, "Out of memory");
}

static struct fmc_cfg_sect_item *remove_section(struct parser_state_t *state, const char *key) {
  for (struct fmc_cfg_sect_item **p = &state->sections_tail; *p; p = &(*p)->next) {
    struct fmc_cfg_sect_item *item = *p;
    if (strcmp(item->key, key) == 0) {
      *p = item->next;
      item->next = NULL;
      return item;
    }
  }
  return NULL;
}

static char *parse_string(char **str, char **end, fmc_error_t **err) {
  if (**str != '"') {
    fmc_error_set(err, "Error while parsing config file: invalid string");
    return NULL;
  }
  ++*str;
  char *endptr = *str;
  while(endptr < *end && *endptr != '"') {
    ++endptr;
  }
  if (endptr == *end) {
    fmc_error_set(err, "Error while parsing config file: unable to find closing quotes for string in array");
    return NULL;
  }
  *endptr = '\0';
  char *ret = *str;
  *str = endptr + 1;
  return ret;
}

static struct fmc_cfg_sect_item *parse_section(struct parser_state_t *state, struct fmc_cfg_node_spec *spec, const char *root_key, fmc_error_t **err);
static struct fmc_cfg_arr_item *parse_array(struct parser_state_t *state, struct fmc_cfg_type *spec, char **str, char **end, fmc_error_t **err) {
  fmc_cfg_arr_item *arr = NULL;

  bool brackets_expected = *str < *end && **str == '[';
  if (brackets_expected) {
    ++*str;
  }

  if (*str + 1 == *end && **str == ',') {
    return NULL;
  }

  switch (spec->type) {
  case FMC_CFG_NONE: {
    bool comma_expected = false;
    while(*str < *end) {
      if (brackets_expected && **str == ']') {
        ++*str;
        break;
      }
      else if (comma_expected) {
        if (**str == ',') {
          ++*str;
          comma_expected = false;
        }
        else {
          fmc_error_set(err, "Error while parsing config file: comma was expected in array");
          goto do_cleanup;
        }
      }
      else if ((*str + 4 == *end || *str[4] == ',' || (brackets_expected && *str[4] == ']')) && memcmp(*str, "none", 4) == 0) {
        comma_expected = true;
        *str += 4;
        arr = fmc_cfg_arr_item_add_none(arr);
      }
      else {
        fmc_error_set(err, "Error while parsing config file: unable to parse array");
        goto do_cleanup;
      }
    }
  } break;
  case FMC_CFG_BOOLEAN: {
    bool comma_expected = false;
    while(*str < *end) {
      if (brackets_expected && **str == ']') {
        ++*str;
        break;
      }
      else if (comma_expected) {
        if (**str == ',') {
          ++*str;
          comma_expected = false;
        }
        else {
          fmc_error_set(err, "Error while parsing config file: comma was expected in array");
          goto do_cleanup;
        }
      }
      else if ((*str + 5 == *end || *str[5] == ',' || (brackets_expected && *str[5] == ']')) && memcmp(*str, "false", 5) == 0) {
        comma_expected = true;
        *str += 5;
        arr = fmc_cfg_arr_item_add_boolean(arr, false);
      }
      else if ((*str + 4 == *end || *str[4] == ',' || (brackets_expected && *str[4] == ']')) && memcmp(*str, "true", 4) == 0) {
        comma_expected = true;
        *str += 4;
        arr = fmc_cfg_arr_item_add_boolean(arr, true);
      }
      else {
        fmc_error_set(err, "Error while parsing config file: unable to parse array");
        goto do_cleanup;
      }
    }
  } break;
  case FMC_CFG_INT64: {
    bool comma_expected = false;
    while(*str < *end) {
      if (brackets_expected && **str == ']') {
        ++*str;
        break;
      }
      else if (comma_expected) {
        if (**str == ',') {
          ++*str;
          comma_expected = false;
        }
        else {
          fmc_error_set(err, "Error while parsing config file: comma was expected in array");
          goto do_cleanup;
        }
      }
      else {
        comma_expected = true;
        char *endptr;
        int64_t value = strtoll(*str, &endptr, 10);
        if (endptr != *str && (endptr == *end || *endptr == ',' || (brackets_expected && *endptr == ']'))) {
          *str = endptr;
          arr = fmc_cfg_arr_item_add_int64(arr, value);
        }
        else {
          fmc_error_set(err, "Error while parsing config file: unable to parse int64 in array");
          goto do_cleanup;
        }
      }
    }
  } break;
  case FMC_CFG_FLOAT64: {
    bool comma_expected = false;
    while(*str < *end) {
      if (brackets_expected && **str == ']') {
        ++*str;
        break;
      }
      else if (comma_expected) {
        if (**str == ',') {
          ++*str;
          comma_expected = false;
        }
        else {
          fmc_error_set(err, "Error while parsing config file: comma was expected in array");
          goto do_cleanup;
        }
      }
      else {
        comma_expected = true;
        char *endptr;
        double value = strtod(*str, &endptr);
        if (endptr != *str && (endptr == *end || *endptr == ',' || (brackets_expected && *endptr == ']'))) {
          *str = endptr;
          arr = fmc_cfg_arr_item_add_float64(arr, value);
        }
        else {
          fmc_error_set(err, "Error while parsing config file: unable to parse float64 in array");
          goto do_cleanup;
        }
      }
    }
  } break;
  case FMC_CFG_STR: {
    bool comma_expected = false;
    while(*str < *end) {
      if (brackets_expected && **str == ']') {
        ++*str;
        break;
      }
      else if (comma_expected) {
        if (**str == ',') {
          ++*str;
          comma_expected = false;
        }
        else {
          fmc_error_set(err, "Error while parsing config file: comma was expected in array");
          goto do_cleanup;
        }
      }
      else {
        comma_expected = true;
        char *value = parse_string(str, end, err);
        if (*err) {
          goto do_cleanup;
        }
        arr = fmc_cfg_arr_item_add_str(arr, value);
      }
    }
  } break;
  case FMC_CFG_SECT: {
    bool comma_expected = false;
    while(*str < *end) {
      if (brackets_expected && **str == ']') {
        ++*str;
        break;
      }
      else if (comma_expected) {
        if (**str == ',') {
          ++*str;
          comma_expected = false;
        }
        else {
          fmc_error_set(err, "Error while parsing config file: comma was expected in array");
          goto do_cleanup;
        }
      }
      else {
        comma_expected = true;
        char *endptr = *str;
        while(endptr < *end && *endptr != ',' && *endptr != ']') {
          ++endptr;
        }
        if (!(endptr == *end || *endptr == ',' || (brackets_expected && *endptr == ']'))) {
          fmc_error_set(err, "Error while parsing config file: unable to parse section in array");
          goto do_cleanup;
        }
        char endch = *endptr;
        *endptr = '\0';
        struct fmc_cfg_sect_item *sect = parse_section(state, spec->spec.node, *str, err);
        *endptr = endch;
        if (*err) {
          goto do_cleanup;
        }
        *str = endptr;

        arr = fmc_cfg_arr_item_add_sect(arr, sect);
      }
    }
  } break;
  case FMC_CFG_ARR: {
    bool comma_expected = false;
    while(*str < *end) {
      if (brackets_expected && **str == ']') {
        ++*str;
        break;
      }
      else if (comma_expected) {
        if (**str == ',') {
          ++*str;
          comma_expected = false;
        }
        else {
          fmc_error_set(err, "Error while parsing config file: comma was expected in array");
          goto do_cleanup;
        }
      }
      else {
        comma_expected = true;
        struct fmc_cfg_arr_item *subarr = parse_array(state, spec->spec.array, str, end, err);
        if (*err) {
          goto do_cleanup;
        }

        arr = fmc_cfg_arr_item_add_arr(arr, subarr);
      }
    }
  } break;
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

static struct fmc_cfg_sect_item *parse_section(struct parser_state_t *state, struct fmc_cfg_node_spec *spec, const char *root_key, fmc_error_t **err) {
  struct fmc_cfg_sect_item *root = remove_section(state, root_key);
  if (!root) {
    fmc_error_set(err, "Error while parsing config file: section %s not found", root_key);
    return NULL;
  }

  struct fmc_cfg_sect_item *pending_items = root->node.value.sect;
  struct fmc_cfg_sect_item *processed_items = NULL;

  for (struct fmc_cfg_node_spec *spec_item = spec; spec_item->key; ++spec_item) {
    struct fmc_cfg_sect_item *item = NULL;
    for (struct fmc_cfg_sect_item **pitem = &pending_items; *pitem; pitem = &(*pitem)->next) {
      if (strcmp(spec_item->key, (*pitem)->key) == 0) {
        item = *pitem;
        *pitem = item->next;
        item->next = processed_items;
        processed_items = item;
        break;
      }
    }
    if (!item) {
      if (spec_item->required) {
        fmc_error_set(err, "Error while parsing config file: missing required field %s", spec_item->key);
        goto do_cleanup;
      }
      else {
        continue;
      }
    }

    switch (spec_item->type.type) {
    case FMC_CFG_NONE: {
      if (strcmp(item->node.value.str, "none") == 0) {
        free((void *) item->node.value.str);
        item->node.type = FMC_CFG_NONE;
      }
      else {
        fmc_error_set(err, "Error while parsing config file: none was expected in field %s", item->key);
        goto do_cleanup;
      }
    } break;
    case FMC_CFG_BOOLEAN: {
      if (strcmp(item->node.value.str, "true") == 0) {
        free((void *) item->node.value.str);
        item->node.value.boolean = true;
      }
      else if (strcmp(item->node.value.str, "false") == 0) {
        free((void *) item->node.value.str);
        item->node.value.boolean = false;
      }
      else {
        fmc_error_set(err, "Error while parsing config file: true or false was expected in field %s", item->key);
        goto do_cleanup;
      }
      item->node.type = FMC_CFG_BOOLEAN;
    } break;
    case FMC_CFG_INT64: {
      char *endptr;
      int64_t value = strtoll(item->node.value.str, &endptr, 10);
      if (endptr != item->node.value.str && *endptr == '\0') {
        free((void *) item->node.value.str);
        item->node.value.int64 = value;
      }
      else {
        fmc_error_set(err, "Error while parsing config file: unable to parse int64 in field %s", item->key);
        goto do_cleanup;
      }
      item->node.type = FMC_CFG_INT64;
    } break;
    case FMC_CFG_FLOAT64: {
      char *endptr;
      double value = strtod(item->node.value.str, &endptr);
      if (endptr != item->node.value.str && *endptr == '\0') {
        free((void *) item->node.value.str);
        item->node.value.float64 = value;
      }
      else {
        fmc_error_set(err, "Error while parsing config file: unable to parse float64 in field %s", item->key);
        goto do_cleanup;
      }
      item->node.type = FMC_CFG_FLOAT64;
    } break;
    case FMC_CFG_STR: {
      char *begin = item->node.value.str;
      char *end = item->node.value.str + strlen(item->node.value.str);
      char *value = parse_string(&begin, &end, err);
      if (*err) {
        goto do_cleanup;
      }
      if (begin != end) {
        fmc_error_set(err, "Error while parsing config file: unable to parse string in field %s", item->key);
        goto do_cleanup;
      }
      memcpy(item->node.value.str, value, strlen(value) + 1);
    } break;
    case FMC_CFG_SECT: {
      struct fmc_cfg_sect_item *section = parse_section(state, spec_item->type.spec.node, item->node.value.str, err);
      if (*err) {
        goto do_cleanup;
      }
      free((void *) item->node.value.str);
      item->node.value.sect = section;
      item->node.type = FMC_CFG_SECT;
    } break;
    case FMC_CFG_ARR: {
      char *begin = item->node.value.str;
      char *end = item->node.value.str + strlen(item->node.value.str);
      struct fmc_cfg_arr_item *arr = parse_array(state, spec_item->type.spec.array, &begin, &end, err);
      if (*err) {
        goto do_cleanup;
      }
      if (begin != end) {
        fmc_error_set(err, "Error while parsing config file: unable to parse array in field %s", item->key);
        goto do_cleanup;
      }
      free((void *) item->node.value.str);
      item->node.value.arr = arr;
      item->node.type = FMC_CFG_ARR;
    } break;
    }
  }

  if (pending_items) {
    fmc_error_set(err, "Error while parsing config file: unknown field %s", pending_items->key);
    goto do_cleanup;
  }

  {
    root->node.type = FMC_CFG_NONE;
    fmc_cfg_sect_del(root);
    return processed_items;
  }

  do_cleanup:
  struct fmc_cfg_sect_item **pitem = &pending_items;
  for (; *pitem; pitem = &(*pitem)->next)
    ;
  *pitem = processed_items;

  root->next = state->sections_tail;
  state->sections_tail = root;
  return NULL;
}

struct fmc_cfg_sect_item *fmc_cfg_sect_parse_ini_file(struct fmc_cfg_node_spec *spec, fmc_fd fd, const char *root_key, fmc_error_t **err) {
  fmc_error_clear(err);
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
      parse_ini_line(&state, buffer, read, err);
      if (*err) {
        goto do_cleanup;
      }

      struct fmc_cfg_sect_item *sect = parse_section(&state, spec, root_key, err);
      if (*err) {
        goto do_cleanup;
      }

      return sect;
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
        parse_ini_line(&state, buffer + line_start, j - line_start, err);
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
  if (state.sections_tail) {
    fmc_cfg_sect_del(state.sections_tail);
  }
  return NULL;
}
