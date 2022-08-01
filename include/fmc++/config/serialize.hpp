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
 * @file serialize.hpp
 * @date 10 Aug 2021
 * @brief File contains configuration serialization utilities
 *
 * This file contains configuration serialization utilities
 */

#pragma once

#include "fmc++/config/variant_map.hpp"
#include "fmc++/misc.hpp"
#include "fmc/files.h"
#include "fmc/platform.h"

#include <cstring>
#include <fmc/alignment.h>
#include <fmc/error.h>
#include <fmc/files.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace fmc {

namespace configs {

namespace serialize {

namespace detail {

class ini_parser {
public:
  ini_parser(fmc::configs::variant_map::section &n,
             size_t buffer_size) noexcept;

  void load_file(fmc_fd fd, fmc_error_t **error) noexcept;

private:
  void parse_line(std::string_view line, fmc_error_t **error) noexcept;

private:
  fmc::configs::variant_map::section &cfg;
  fmc::configs::variant_map::section *current_section = nullptr;
  size_t buffer_size;
  uint64_t line_number = 0;
};

enum result { ERROR, SUCCESS, SKIP };

using result_t = std::pair<result, std::string_view>;

inline result_t parse_quoted(fmc::configs::variant_map::node &ret,
                             std::string_view str, fmc_error_t **error);

inline result_t parse_comma(std::string_view str);

inline result_t parse_element(fmc::configs::variant_map::node &ret,
                              fmc::configs::variant_map::node &srcnode,
                              std::string_view str, fmc_error_t **error);

inline fmc::configs::variant_map::node
parse_value(std::string_view str, fmc::configs::variant_map::node &srcnode,
            fmc_error_t **error);

inline result_t parse_comma(std::string_view str) {
  if (str[0] == ',') {
    return make_pair<result, std::string_view>(
        result::SUCCESS, std::string_view(str.data() + 1, str.size() - 1));
  }
  return make_pair<result, std::string_view>(result::SKIP, std::string_view());
}

inline result_t parse_quoted(fmc::configs::variant_map::node &ret,
                             std::string_view str, fmc_error_t **error) {
  auto pos = str.find("\"", 1);
  if (pos == std::string_view::npos) {
    FMC_ERROR_REPORT(error, "Error while parsing value; unable to parse quoted "
                            "string, missing ending quotes");
    return make_pair<result, std::string_view>(result::ERROR,
                                               std::string_view());
  }
  ret = fmc::configs::variant_map::node(std::string(str.data() + 1, pos - 1));
  return make_pair<result, std::string_view>(
      result::SUCCESS,
      std::string_view(str.data() + pos + 1, str.size() - pos - 1));
}

inline result_t parse_unquoted(fmc::configs::variant_map::node &ret,
                               fmc::configs::variant_map::node &srcnode,
                               std::string_view str, fmc_error_t **error) {
  std::string_view section_name = str;
  auto pos = str.find(",", 1);
  std::string_view retstr;
  if (pos != std::string_view::npos) {
    section_name = str.substr(0, pos);
    retstr = std::string_view(str.data() + pos, str.size() - pos);
  }
  auto &&[intval, processed_int_str] = from_string_view<int64_t>(section_name);
  if (processed_int_str.size() == section_name.size()) {
    ret = fmc::configs::variant_map::node(intval);
    return make_pair<result, std::string_view>(result::SUCCESS,
                                               std::move(retstr));
  }
  fmc::configs::variant_map::section submap;
  for (auto &&[key, node] : srcnode[section_name].to_d()) {
    auto val_str = node.to_s();
    submap[key] = parse_value(val_str, srcnode, error);
    if (*error) {
      return make_pair<result, std::string_view>(result::ERROR,
                                                 std::string_view());
    }
  }
  ret = std::move(submap);
  return make_pair<result, std::string_view>(result::SUCCESS,
                                             std::move(retstr));
}

inline result_t parse_element(fmc::configs::variant_map::node &ret,
                              fmc::configs::variant_map::node &srcnode,
                              std::string_view str, fmc_error_t **error,
                              bool first) {
  if (str.empty()) {
    FMC_ERROR_REPORT(error, "Error while parsing value; empty string");
    return make_pair<result, std::string_view>(result::ERROR,
                                               std::string_view());
  }
  if (str[0] == '"') {
    return parse_quoted(ret, str, error);
  } else if (str[0] == ',') {
    if (first && str.size() == 1) {
      ret = fmc::configs::variant_map::array();
      return make_pair<result, std::string_view>(
          result::SUCCESS, std::string_view(str.data() + 1, str.size() - 1));
    } else {
      FMC_ERROR_REPORT(error, "Error while parsing value; empty section name");
      return make_pair<result, std::string_view>(result::ERROR,
                                                 std::string_view());
    }
  } else {
    return parse_unquoted(ret, srcnode, str, error);
  }
}

inline fmc::configs::variant_map::node
parse_value(std::string_view str, fmc::configs::variant_map::node &srcnode,
            fmc_error_t **error) {
  fmc::configs::variant_map::node ret;

  auto [res, rest] = parse_element(ret, srcnode, str, error, true);
  if (res == result::ERROR) {
    return ret;
  }
  if (res == result::SUCCESS && !rest.empty()) {
    auto [res2, rest2] = parse_comma(rest);
    if (res2 == result::ERROR) {
      FMC_ERROR_REPORT(error,
                       "Error while parsing value; unable to parse comma");
      return ret;
    }
    if (res2 == result::SUCCESS) {
      fmc::configs::variant_map::array arr;
      arr.push_back(std::move(ret));
      std::string_view remaining = rest2;
      while (true) {
        if (remaining.empty()) {
          break;
        }
        fmc::configs::variant_map::node arr_item;
        auto [res3, rest3] =
            parse_element(arr_item, srcnode, remaining, error, false);
        if (res3 == result::ERROR) {
          return ret;
        }
        arr.push_back(std::move(arr_item));
        if (rest3.empty()) {
          break;
        }
        auto [res4, rest4] = parse_comma(rest3);
        if (res4 == result::ERROR) {
          FMC_ERROR_REPORT(error,
                           "Error while parsing value; unable to parse comma");
          return ret;
        }
        remaining = rest4;
      }
      ret = std::move(arr);
    }
  }
  return ret;
}

fmc::configs::variant_map::node
process_structured_config(fmc::configs::variant_map::node &src,
                          std::string_view main, fmc_error_t **error) {
  fmc::configs::variant_map::section ret;

  for (auto &&[key, node] : src[main].to_d()) {
    auto val_str = node.to_s();
    ret[key] = parse_value(val_str, src, error);
  }

  return fmc::configs::variant_map::node(std::move(ret));
}

} // namespace detail

inline detail::ini_parser::ini_parser(variant_map::section &n,
                                      size_t buffer_size = 1024) noexcept
    : cfg(n), buffer_size(buffer_size) {}

inline void detail::ini_parser::load_file(fmc_fd fd,
                                          fmc_error_t **error) noexcept {
  std::string buffer(buffer_size, '\0');

  size_t read = 0;
  while (true) {
    if (buffer.size() == read) {
      FMC_ERROR_REPORT(error,
                       "Error while parsing config file: line is too long");
      return;
    }

    auto ret = fmc_fread(fd, buffer.data() + read, buffer.size() - read, error);
    if (*error) {
      return;
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
      fmc_error_clear(error);
      parse_line(std::string_view(buffer.data(), read), error);
      return;
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
        parse_line(std::string_view(buffer.data() + line_start, j - line_start),
                   error);
        if (*error) {
          return;
        }
        line_start = i + 1;
      }
    }
    read = end - line_start;
    if (line_start != end && line_start > 0) {
      std::memcpy(buffer.data(), buffer.data() + line_start, read);
    }
    if (*error) {
      return;
    }
  }
}

inline void detail::ini_parser::parse_line(std::string_view line,
                                           fmc_error_t **error) noexcept {
  ++line_number;
  if (line.empty()) {
    return;
  }

  if (line[0] == '[' && line[line.size() - 1] == ']') {
    auto section_name = line.substr(1, line.size() - 2);
    auto &sec = cfg[std::string(section_name)];
    sec = fmc::configs::variant_map::section();
    current_section = static_cast<fmc::configs::variant_map::section *>(
        &cfg[std::string(section_name)].to_d());
    return;
  } else {
    auto sep_index = line.find('=');
    if (sep_index == std::string_view::npos) {
      FMC_ERROR_REPORT(
          error,
          std::string("Invalid configuration file key-value entry (line " +
                      std::to_string(line_number) + ")")
              .c_str());
      return;
    }
    if (current_section == nullptr) {
      FMC_ERROR_REPORT(error, std::string("Configuration entry doesn't have "
                                          "a section in the file (line " +
                                          std::to_string(line_number) + ")")
                                  .c_str());
      return;
    }
    auto key = line.substr(0, sep_index);
    auto value = line.substr(sep_index + 1);
    (*current_section)[std::string(key)] =
        fmc::configs::variant_map::node(std::string(value));
  }
}

inline variant_map::node variant_map_load_ini(fmc_fd file,
                                              fmc_error_t **error) {

  fmc::configs::variant_map::node ret = fmc::configs::variant_map::section();

  fmc::configs::serialize::detail::ini_parser parser(
      static_cast<fmc::configs::variant_map::section &>(ret.to_d()));

  parser.load_file(file, error);

  return ret;
}

inline variant_map::node
variant_map_load_ini_structured(const char *file_name,
                                const char *main_section_key) {

  fmc_error_t *error;
  auto fd = fmc_fopen(file_name, fmc_fmode::READ, &error);
  fmc_runtime_error_unless(!error)
      << "unable to open file with error :" << fmc_error_msg(error);

  fmc::configs::variant_map::node cfg =
      fmc::configs::serialize::variant_map_load_ini(fd, &error);
  fmc_runtime_error_unless(!error)
      << "unable to parse file with error :" << fmc_error_msg(error);

  auto ret = fmc::configs::serialize::detail::process_structured_config(
      cfg, main_section_key, &error);
  fmc_runtime_error_unless(!error)
      << "unable to process structured configuration with error :"
      << fmc_error_msg(error);

  return ret;
}

inline variant_map::node
variant_map_load_ini_main(const char *file_name, const char *main_section_key,
                          const char *sub_sections_key) {

  fmc_error_t *error;
  auto fd = fmc_fopen(file_name, fmc_fmode::READ, &error);
  fmc_runtime_error_unless(!error)
      << "unable to open file with error :" << fmc_error_msg(error);

  fmc::configs::variant_map::node cfg =
      fmc::configs::serialize::variant_map_load_ini(fd, &error);
  fmc_runtime_error_unless(!error)
      << "unable to parse file with error :" << fmc_error_msg(error);

  fmc_fclose(fd, &error);
  fmc_runtime_error_unless(!error)
      << "unable to close file with error :" << fmc_error_msg(error);

  std::string sessions_str = cfg[main_section_key][sub_sections_key].to_s();

  fmc::configs::variant_map::node structured_cfg = cfg[main_section_key];

  auto &structured_cfg_section =
      static_cast<fmc::configs::variant_map::section &>(structured_cfg.to_d());

  structured_cfg_section[sub_sections_key] =
      fmc::configs::variant_map::node(fmc::configs::variant_map::section());

  auto &sessions_cfg = static_cast<fmc::configs::variant_map::section &>(
      structured_cfg_section[sub_sections_key].to_d());

  size_t last = 0;
  size_t next = 0;
  do {
    next = sessions_str.find(',', last);
    sessions_cfg[sessions_str.substr(last, next - last)] =
        cfg[sessions_str.substr(last, next - last)];
    last = next + 1;
  } while (next != std::string::npos);

  return structured_cfg;
}

} // namespace serialize

} // namespace configs

} // namespace fmc
