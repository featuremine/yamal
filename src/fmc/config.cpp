/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file config.cpp
 * @date 11 May 2023
 * @brief Implementation of the fmc configuration API
 *
 * @see http://www.featuremine.com
 */

#include <exception>
#include <fmc/config.h>
#include <fmc/string.h>
#include <json/json.hpp>
#include <set>
#include <uthash/utlist.h>

#define JSON_PARSER_BUFF_SIZE 8192

static struct fmc_cfg_arr_item *
parse_array(nlohmann::json j_obj, struct fmc_cfg_type *spec, fmc_error_t **err);
static struct fmc_cfg_sect_item *parse_section(nlohmann::json j_obj,
                                               struct fmc_cfg_node_spec *spec,
                                               fmc_error_t **err);
static void parse_value(nlohmann::json j_obj, struct fmc_cfg_type *spec,
                        struct fmc_cfg_item *out, fmc_error_t **err);

static struct fmc_cfg_arr_item *parse_array(nlohmann::json j_obj,
                                            struct fmc_cfg_type *spec,
                                            fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_arr_item *arr = NULL;
  if (j_obj.size() == 0) {
    return arr;
  }

  for (auto i = j_obj.size(); i > 0; --i) {
    struct fmc_cfg_arr_item *item = fmc_cfg_arr_item_new(err);
    if (*err) {
      goto do_cleanup;
    }
    LL_PREPEND(arr, item);
    parse_value(j_obj[i - 1], spec, &item->item, err);
    if (*err) {
      goto do_cleanup;
    }
  }

  return arr;

do_cleanup:
  fmc_cfg_arr_del(arr);
  return NULL;
}

static void parse_value(nlohmann::json j_obj, struct fmc_cfg_type *spec,
                        struct fmc_cfg_item *out, fmc_error_t **err) {
  fmc_error_clear(err);

  out->type = FMC_CFG_NONE;
  switch (spec->type) {
  case FMC_CFG_NONE: {
    if (j_obj.is_null()) {
      out->type = FMC_CFG_NONE;
    } else {
      fmc_error_set(err, "config error: unable to parse none from '%s'",
                    j_obj.dump().c_str());
      return;
    }
  } break;
  case FMC_CFG_BOOLEAN: {
    if (j_obj.is_boolean()) {
      out->type = FMC_CFG_BOOLEAN;
      out->value.boolean = j_obj.get<bool>();
    } else {
      fmc_error_set(err, "config error: unable to parse boolean from '%s'",
                    j_obj.dump().c_str());
      return;
    }
  } break;
  case FMC_CFG_INT64: {
    if (j_obj.is_number_integer()) {
      out->type = FMC_CFG_INT64;
      out->value.int64 = j_obj.get<int64_t>();
    } else {
      fmc_error_set(err, "config error: unable to parse int64 from '%s'",
                    j_obj.dump().c_str());
      return;
    }
  } break;
  case FMC_CFG_FLOAT64: {
    if (j_obj.is_number_float()) {
      out->type = FMC_CFG_FLOAT64;
      out->value.float64 = j_obj.get<double>();
    } else {
      fmc_error_set(err, "config error: unable to parse float64 from '%s'",
                    j_obj.dump().c_str());
      return;
    }
  } break;
  case FMC_CFG_STR: {
    if (j_obj.is_string()) {
      out->type = FMC_CFG_STR;
      auto str = j_obj.get<std::string_view>();
      out->value.str = fmc_cstr_new2(str.data(), str.size(), err);
    } else {
      fmc_error_set(err, "config error: unable to parse string from '%s'",
                    j_obj.dump().c_str());
      return;
    }
  } break;
  case FMC_CFG_SECT: {
    if (j_obj.is_object()) {
      out->type = FMC_CFG_SECT;
      struct fmc_cfg_sect_item *sect =
          parse_section(j_obj, spec->spec.node, err);
      if (*err) {
        return;
      }
      out->value.sect = sect;
    } else {
      fmc_error_set(err, "config error: unable to parse section from '%s'",
                    j_obj.dump().c_str());
      return;
    }
  } break;
  case FMC_CFG_ARR: {
    if (j_obj.is_array()) {
      out->type = FMC_CFG_ARR;
      struct fmc_cfg_arr_item *subarr =
          parse_array(j_obj, spec->spec.array, err);
      if (*err) {
        return;
      }
      out->value.arr = subarr;
    } else {
      fmc_error_set(err, "config error: unable to parse array from '%s'",
                    j_obj.dump().c_str());
      return;
    }
  } break;
  }
}

static struct fmc_cfg_sect_item *parse_section(nlohmann::json j_obj,
                                               struct fmc_cfg_node_spec *spec,
                                               fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *sect = NULL;

  std::set<std::string> visited;

  for (struct fmc_cfg_node_spec *spec_item = spec; spec_item->key;
       ++spec_item) {
    auto elemit = j_obj.find(spec_item->key);
    if (elemit == j_obj.end()) {
      if (spec_item->required) {
        fmc_error_set(err, "config error: missing required field %s",
                      spec_item->key);
        goto do_cleanup;
      }
      continue;
    }
    visited.emplace(spec_item->key);

    struct fmc_cfg_sect_item *sitem = fmc_cfg_sect_item_new(err);
    if (*err) {
      goto do_cleanup;
    }
    LL_PREPEND(sect, sitem);
    sitem->key = fmc_cstr_new(spec_item->key, err);
    if (*err) {
      goto do_cleanup;
    }
    parse_value(*elemit, &spec_item->type, &sitem->node, err);
    if (*err) {
      goto do_cleanup;
    }
  }

  if (visited.size() != j_obj.size()) {
    for (auto it = j_obj.begin(); it != j_obj.end(); ++it) {
      if (visited.find(it.key()) == visited.end()) {
        fmc_error_set(err, "config error: unknown field %s", it.key().c_str());
        goto do_cleanup;
      }
    }
  }

  return sect;

do_cleanup:
  fmc_cfg_sect_del(sect);
  return NULL;
}

struct fmc_cfg_sect_item *
fmc_cfg_sect_parse_json(struct fmc_cfg_node_spec *spec, const char *buffer,
                        size_t sz, fmc_error_t **err) {
  fmc_error_clear(err);

  struct fmc_cfg_sect_item *ret = NULL;
  struct fmc_cfg_sect_item *sect = NULL;

  try {

    nlohmann::json j_obj = nlohmann::json::parse(std::string_view(buffer, sz));

    sect = parse_section(j_obj, spec, err);
    if (*err) {
      goto do_cleanup;
    }

    ret = sect;
    sect = NULL;

    return ret;
  } catch (const std::exception &e) {
    fmc_error_set(err, "config error: unable to parse with error '%s'",
                  e.what());
  }

do_cleanup:
  fmc_cfg_sect_del(sect);
  return ret;
}
