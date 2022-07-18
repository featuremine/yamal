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

void cfg_to_string(const std::string &prefix, std::string &result, struct fmc_cfg_sect_item *sect);
void cfg_to_string(const std::string &prefix, std::string &result, struct fmc_cfg_arr_item *arr) {
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

void cfg_to_string(const std::string &prefix, std::string &result, struct fmc_cfg_sect_item *sect) {
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

unique_sect parse_cfg(std::string_view config, struct fmc_cfg_node_spec *spec, fmc_error_t *&err) {
  fmc_fd pipe_descriptors[2];

  if(pipe(pipe_descriptors) != 0) {
    fmc_error_set(&err, "pipe failed");
    return nullptr;
  }

  write(pipe_descriptors[1], config.data(), config.size());
  fmc_fclose(pipe_descriptors[1], &err);
  if (err) {
    return nullptr;
  }

  auto sect = unique_sect(fmc_cfg_sect_parse_ini_file(spec, pipe_descriptors[0], "main", &err));
  if (err) {
    return nullptr;
  }
  fmc_fclose(pipe_descriptors[0], &err);
  if (err) {
    return nullptr;
  }

  return sect;
}

TEST(error, required_1) {
  struct fmc_cfg_node_spec subsect[] = {
      fmc_cfg_node_spec{
        .key = "int64",
        .descr = "int64 descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_INT64,
        },
      },
      fmc_cfg_node_spec{NULL},
  };

  struct fmc_cfg_type subarray = {
      .type = FMC_CFG_INT64,
  };

  struct fmc_cfg_node_spec main[] = {
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
        .key = "arr",
        .descr = "arr3 descr",
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
  fmc_error_t *err;
  auto sect = parse_cfg(""
                        "[main]\n"
                        "float64=-19.5\n"
                        "str=\"message\"\n"
                        "int64=-99\n"
                        "boolean=false\n"
                        "none=none\n"
                        "sect=sect1\n"
                        "arr=[]\n"
                        "[sect1]\n"
                        "int64=-100000\n"
                        "",
                        main, err);
  ASSERT_NOERR(err);
  EXPECT_EQ(cfg_to_string(sect), ""
                                 "{\n"
                                 "  arr = [\n"
                                 "  ]\n"
                                 "  sect = {\n"
                                 "    int64 = -100000\n"
                                 "  }\n"
                                 "  none = none\n"
                                 "  str = \"message\"\n"
                                 "  float64 = -19.500000\n"
                                 "  boolean = 0\n"
                                 "  int64 = -99\n"
                                 "}\n");

  sect = parse_cfg(""
                   "[main]\n"
                   "float64=-19.5\n"
                   "str=\"message\"\n"
                   "int64=-99\n"
                   "boolean=false\n"
                   "none=none\n"
                   "sect=sect1\n"
                   "arr=[]\n"
                   "[sect1]\n"
                   "",
                   main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: missing required field int64");

  sect = parse_cfg(""
                   "[main]\n"
                   "float64=-19.5\n"
                   "str=\"message\"\n"
                   "int64=-99\n"
                   "boolean=false\n"
                   "none=none\n"
                   "sect=sect1\n"
                   "[sect1]\n"
                   "int64=-100000\n"
                   "",
                   main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: missing required field arr");

  sect = parse_cfg(""
                   "[main]\n"
                   "float64=-19.5\n"
                   "str=\"message\"\n"
                   "int64=-99\n"
                   "boolean=false\n"
                   "none=none\n"
                   "arr=[]\n"
                   "[sect1]\n"
                   "int64=-100000\n"
                   "",
                   main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: missing required field sect");

  sect = parse_cfg(""
                   "[main]\n"
                   "float64=-19.5\n"
                   "str=\"message\"\n"
                   "int64=-99\n"
                   "boolean=false\n"
                   "sect=sect1\n"
                   "arr=[]\n"
                   "[sect1]\n"
                   "int64=-100000\n"
                   "",
                   main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: missing required field none");

  sect = parse_cfg(""
                   "[main]\n"
                   "float64=-19.5\n"
                   "str=\"message\"\n"
                   "int64=-99\n"
                   "none=none\n"
                   "sect=sect1\n"
                   "arr=[]\n"
                   "[sect1]\n"
                   "int64=-100000\n"
                   "",
                   main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: missing required field boolean");

  sect = parse_cfg(""
                   "[main]\n"
                   "float64=-19.5\n"
                   "str=\"message\"\n"
                   "boolean=false\n"
                   "none=none\n"
                   "sect=sect1\n"
                   "arr=[]\n"
                   "[sect1]\n"
                   "int64=-100000\n"
                   "",
                   main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: missing required field int64");

  sect = parse_cfg(""
                   "[main]\n"
                   "float64=-19.5\n"
                   "int64=-99\n"
                   "boolean=false\n"
                   "none=none\n"
                   "sect=sect1\n"
                   "arr=[]\n"
                   "[sect1]\n"
                   "int64=-100000\n"
                   "",
                   main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: missing required field str");

  sect = parse_cfg(""
                   "[main]\n"
                   "str=\"message\"\n"
                   "int64=-99\n"
                   "boolean=false\n"
                   "none=none\n"
                   "sect=sect1\n"
                   "arr=[]\n"
                   "[sect1]\n"
                   "int64=-100000\n"
                   "",
                   main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: missing required field float64");
}

TEST(error, unknown_field_1) {
  struct fmc_cfg_node_spec main[] = {
      fmc_cfg_node_spec{
        .key = "int64",
        .descr = "int64 descr",
        .required = false,
        .type = fmc_cfg_type{
          .type = FMC_CFG_INT64,
        },
      },
      fmc_cfg_node_spec{NULL},
  };
  fmc_error_t *err;
  auto sect = parse_cfg(""
                        "[main]\n"
                        "int63=1\n"
                        "",
                        main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: unknown field int63");
}

TEST(error, unknown_section_1) {
  struct fmc_cfg_node_spec main[] = {
      fmc_cfg_node_spec{
        .key = "int64",
        .descr = "int64 descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_INT64,
        },
      },
      fmc_cfg_node_spec{NULL},
  };
  fmc_error_t *err;
  auto sect = parse_cfg(""
                        "[start]\n"
                        "int64=1\n"
                        "",
                        main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: section main not found");
}

TEST(error, unknown_section_2) {
  struct fmc_cfg_node_spec subsect[] = {
      fmc_cfg_node_spec{
          .key = "int64",
          .descr = "int64 descr",
          .required = true,
          .type = fmc_cfg_type{
              .type = FMC_CFG_INT64,
          },
      },
      fmc_cfg_node_spec{NULL},
  };
  struct fmc_cfg_node_spec main[] = {
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
      fmc_cfg_node_spec{NULL},
  };
  fmc_error_t *err;
  auto sect = parse_cfg(""
                        "[main]\n"
                        "sect=subsact\n"
                        "[subsect]\n"
                        "int64=1\n"
                        "",
                        main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: section subsact not found");
}

TEST(error, invalid_array_1) {
  struct fmc_cfg_type subarray = {
      .type = FMC_CFG_INT64,
  };

  struct fmc_cfg_node_spec main[] = {
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
  fmc_error_t *err;
  auto sect = parse_cfg(""
                        "[main]\n"
                        "arr=[1\n"
                        "",
                        main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: closing bracket was expected in array");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=1]\n"
                        "",
                        main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: unable to parse field arr");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=,1,2,3\n"
                        "",
                        main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: unable to parse field arr");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=1,,3\n"
                        "",
                        main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: unable to parse int64 in array");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=[1,2,3\n"
                        "",
                        main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: closing bracket was expected in array");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=1,2,3]\n"
                        "",
                        main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: unable to parse field arr");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=[1,2a,3]\n"
                        "",
                        main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: comma was expected in array");
}

TEST(error, invalid_ini_1) {
  struct fmc_cfg_type subarray = {
      .type = FMC_CFG_INT64,
  };

  struct fmc_cfg_node_spec main[] = {
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
  fmc_error_t *err;
  std::string str = ""
             "[main]\n"
             "arr=";
  while (str.size() < 10000) {
    str += "1,";
  }
  str += "\n";
  auto sect = parse_cfg(str,
                        main, err);
  EXPECT_EQ(std::string_view(fmc_error_msg(err)), "Error while parsing config file: line is too long");
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
