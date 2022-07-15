/******************************************************************************

        COPYRIGHT (c) 2018 by Featuremine Corporation.
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
 * @date 14 Jul 2022
 * @brief File contains tests for FMC config API
 *
 * @see http://www.featuremine.com
 */

#include <fmc/config.h>

#include <fmc++/gtestwrap.hpp>

#define ASSERT_NOERR(err) ASSERT_EQ((err) ? std::string_view(fmc_error_msg(err)) : std::string_view(), std::string_view())

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
using unique_sect = unique_cfg_ptr<fmc_cfg_sect_item>;
using unique_arr = unique_cfg_ptr<fmc_cfg_arr_item>;

void cfg_to_string(std::string prefix, std::string &result, struct fmc_cfg_sect_item *sect);
void cfg_to_string(std::string prefix, std::string &result, struct fmc_cfg_arr_item *arr) {
  for (; arr; arr = arr->next) {
    result += prefix;
    switch (arr->item.type) {
    case FMC_CFG_NONE: {
      result += "none,\n";
    } break;
    case FMC_CFG_BOOLEAN: {
      result += std::to_string(arr->item.value.boolean);
      result += ",\n";
    } break;
    case FMC_CFG_INT64: {
      result += std::to_string(arr->item.value.int64);
      result += ",\n";
    } break;
    case FMC_CFG_FLOAT64: {
      result += std::to_string(arr->item.value.float64);
      result += ",\n";
    } break;
    case FMC_CFG_STR: {
      result += "\"";
      result += arr->item.value.str;
      result += "\",\n";
    } break;
    case FMC_CFG_SECT: {
      result += "{\n";
      cfg_to_string(prefix + "  ", result, arr->item.value.sect);
      result += prefix;
      result += "},\n";
    } break;
    case FMC_CFG_ARR: {
      result += "[\n";
      cfg_to_string(prefix + "  ", result, arr->item.value.arr);
      result += prefix;
      result += "],\n";
    } break;
    }
  }
}

void cfg_to_string(std::string prefix, std::string &result, struct fmc_cfg_sect_item *sect) {
  for (; sect; sect = sect->next) {
    result += prefix;
    result += sect->key;
    result += " = ";
    switch (sect->node.type) {
    case FMC_CFG_NONE: {
      result += "none\n";
    } break;
    case FMC_CFG_BOOLEAN: {
      result += std::to_string(sect->node.value.boolean);
      result += "\n";
    } break;
    case FMC_CFG_INT64: {
      result += std::to_string(sect->node.value.int64);
      result += "\n";
    } break;
    case FMC_CFG_FLOAT64: {
      result += std::to_string(sect->node.value.float64);
      result += "\n";
    } break;
    case FMC_CFG_STR: {
      result += "\"";
      result += sect->node.value.str;
      result += "\"\n";
    } break;
    case FMC_CFG_SECT: {
      result += "{\n";
      cfg_to_string(prefix + "  ", result, sect->node.value.sect);
      result += prefix;
      result += "}\n";
    } break;
    case FMC_CFG_ARR: {
      result += "[\n";
      cfg_to_string(prefix + "  ", result, sect->node.value.arr);
      result += prefix;
      result += "]\n";
    } break;
    }
  }
}

std::string cfg_to_string(const unique_sect &sect) {
  std::string result;
  result += "{\n";
  cfg_to_string("  ", result, sect.get());
  result += "}\n";
  return result;
}

TEST(error, simple_types_1) {
  fmc_error_t *err;
  fmc_fd pipe_descriptors[2];

  ASSERT_EQ(pipe(pipe_descriptors), 0);

  struct fmc_cfg_node_spec subsect[] = {
      fmc_cfg_node_spec{
        .key = "float64",
        .descr = "float64 descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_FLOAT64,
        },
      },
      fmc_cfg_node_spec{NULL},
  };

  struct fmc_cfg_type subarray = {
      .type = FMC_CFG_INT64,
  };

  struct fmc_cfg_type subarray2 = {
      .type = FMC_CFG_SECT,
      .spec {
        .node = subsect,
      }
  };

  struct fmc_cfg_type subarray3 = {
      .type = FMC_CFG_ARR,
      .spec {
        .array = &subarray,
      }
  };

  struct fmc_cfg_node_spec spec[] = {
      fmc_cfg_node_spec{
        .key = "int64",
        .descr = "int64 descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_INT64,
        },
      },
      fmc_cfg_node_spec{
        .key = "boolean",
        .descr = "boolean descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_BOOLEAN,
        },
      },
      fmc_cfg_node_spec{
        .key = "float64",
        .descr = "float64 descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_FLOAT64,
        },
      },
      fmc_cfg_node_spec{
        .key = "str",
        .descr = "str descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_STR,
        },
      },
      fmc_cfg_node_spec{
        .key = "none",
        .descr = "none descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_NONE,
        },
      },
      fmc_cfg_node_spec{
        .key = "sect",
        .descr = "sect descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_SECT,
          .spec {
            .node = subsect,
          },
        },
      },
      fmc_cfg_node_spec{
        .key = "arr3",
        .descr = "arr3 descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_ARR,
          .spec {
            .array = &subarray3,
          },
        },
      },
      fmc_cfg_node_spec{
        .key = "arr2",
        .descr = "arr2 descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_ARR,
          .spec {
            .array = &subarray2,
          },
        },
      },
      fmc_cfg_node_spec{
        .key = "arr",
        .descr = "arr descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_ARR,
          .spec {
            .array = &subarray,
          },
        },
      },
      fmc_cfg_node_spec{NULL},
  };

  std::string_view config = "[main]\n"
                            "int64=123\n"
                            "boolean=true\n"
                            "float64=1.2\n"
                            "none=none\n"
                            "sect=sect1\n"
                            "arr2=[sect2,sect3]\n"
                            "arr3=[[1,2,3],[1,2,3]]\n"
                            "str=\"strstr\"\n"
                            "arr=1,2,3,4,5\n"
                            "\n"
                            "[sect1]\n"
                            "float64=1.5\n"
                            "[sect2]\n"
                            "float64=1.5\n"
                            "[sect3]\n"
                            "float64=1.5\n"
                            ;

  write(pipe_descriptors[1], config.data(), config.size());
  fmc_fclose(pipe_descriptors[1], &err);
  ASSERT_NOERR(err);

  auto sect = unique_sect(fmc_cfg_sect_parse_ini_file(spec, pipe_descriptors[0], "main", &err));
  ASSERT_NOERR(err);
  fmc_fclose(pipe_descriptors[0], &err);
  ASSERT_NOERR(err);

  ASSERT_EQ(cfg_to_string(sect), ""
    "{\n"
    "  arr = [\n"
    "    1,\n"
    "    2,\n"
    "    3,\n"
    "    4,\n"
    "    5,\n"
    "  ]\n"
    "  arr2 = [\n"
    "    {\n"
    "      float64 = 1.500000\n"
    "    },\n"
    "    {\n"
    "      float64 = 1.500000\n"
    "    },\n"
    "  ]\n"
    "  arr3 = [\n"
    "    [\n"
    "      1,\n"
    "      2,\n"
    "      3,\n"
    "    ],\n"
    "    [\n"
    "      1,\n"
    "      2,\n"
    "      3,\n"
    "    ],\n"
    "  ]\n"
    "  sect = {\n"
    "    float64 = 1.500000\n"
    "  }\n"
    "  none = none\n"
    "  str = \"strstr\"\n"
    "  float64 = 1.200000\n"
    "  boolean = 1\n"
    "  int64 = 123\n"
    "}\n");
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
