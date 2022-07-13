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
#include <string>
#include <string_view>
#include <memory>

#define INI_PARSER_BUFF_SIZE 8192

static char *string_copy_len(const char *src, size_t len) {
  void *p = calloc(len + 1, sizeof(char));
  if (!p) {
    return NULL;
  }
  memcpy(p, src, len);
  return (const char *)p;
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
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
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
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
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
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
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
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
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
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
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
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
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
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
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
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_boolean(struct fmc_cfg_arr_item * tail, bool new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_BOOLEAN;
  item->item.value.boolean = new_value;
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_int64(struct fmc_cfg_arr_item * tail, int64_t new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_INT64;
  item->item.value.int64 = new_value;
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_float64(struct fmc_cfg_arr_item * tail, double new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_FLOAT64;
  item->item.value.float64 = new_value;
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_str(struct fmc_cfg_arr_item * tail, const char * new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_STR;
  item->item.value.str = string_copy(new_value);
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_sect(struct fmc_cfg_arr_item * tail, struct fmc_cfg_sect_item * new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_SECT;
  item->item.value.sect = new_value;
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
  return item;
}
struct fmc_cfg_arr_item *fmc_cfg_arr_item_add_arr(struct fmc_cfg_arr_item * tail, struct fmc_cfg_arr_item * new_value) {
  struct fmc_cfg_arr_item *item = new_arr_item();
  if (!item) {
    return NULL;
  }
  item->item.type = FMC_CFG_ARR;
  item->item.value.arr = new_value;
  item->next = NULL;
  if (tail) {
    tail->next = item;
  }
  return item;
}

struct deleter_t {
  void operator()(struct fmc_cfg_sect_item *head) {
    fmc_cfg_sect_del(head);
  }
  void operator()(struct fmc_cfg_arr_item *head) {
    fmc_cfg_arr_del(head);
  }
};
template<typename T>
using unique_cfg_ptr = std::unique_ptr<T, deleter_t>;

struct parser_state_t {
  struct fmc_cfg_sect_item *sections_head;
  struct fmc_cfg_sect_item *sections_tail;
  struct fmc_cfg_sect_item *fields_tail;
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

    state->sections_tail = new_sect_item();
    if (!state->sections_tail) {
      goto do_oom;
    }

    if (!state->sections_head) {
      state->sections_head = state->sections_tail;
    }
    state->sections_tail->key = key;
    state->fields_tail = NULL;
  }
  else {
    size_t sep;
    for (sep = 0; sep != '=' && sep < sz; ++sep);
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

    state->fields_tail = fmc_cfg_sect_item_add_str(state->fields_tail, key, value);
    if (!state->fields_tail) {
      free(value);
      free(key);
      goto do_oom;
    }
  }
  return;

  do_oom:
  fmc_error_set(error, "Out of memory");
  return;
}

struct fmc_cfg_sect_item *fmc_cfg_sect_parse_ini_file(struct fmc_cfg_node_spec *spec, fmc_fd fd, fmc_error_t **err) {
  parser_state_t state {NULL, NULL, 0};
  char buffer[INI_PARSER_BUFF_SIZE];

  size_t read = 0;
  while (true) {
    if (INI_PARSER_BUFF_SIZE == read) {
      FMC_ERROR_REPORT(err,
                       "Error while parsing config file: line is too long");
      goto do_error;
    }

    auto ret = fmc_fread(fd, buffer + read, INI_PARSER_BUFF_SIZE - read, err);
    if (*err) {
      goto do_error;
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
      goto do_success;
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
          goto do_error;
        }
        line_start = i + 1;
      }
    }
    read = end - line_start;
    if (line_start != end && line_start > 0) {
      memcpy(buffer, buffer + line_start, read);
    }
    if (*err) {
      goto do_error;
    }
  }

  do_success:
  return state.sections_head;

  do_error:
  if (state.sections_head) {
    fmc_cfg_sect_del(state.sections_head);
  }
  return NULL;
}
