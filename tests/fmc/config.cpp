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
#include <fmc/uthash/utlist.h>

#include <fmc++/gtestwrap.hpp>

#define ASSERT_NOERR(err) ASSERT_EQ((err) ? std::string_view(fmc_error_msg(err)) : std::string_view(), std::string_view())
#define EXPECT_ERR(err, msg) EXPECT_EQ((err) ? std::string_view(fmc_error_msg(err)) : std::string_view(), std::string_view(msg))

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

unique_sect parse_cfg(const std::vector<std::string_view> &config_parts, struct fmc_cfg_node_spec *spec, fmc_error_t *&err) {
  fmc_fd pipe_descriptors[2];

  if(pipe(pipe_descriptors) != 0) {
    fmc_error_set(&err, "pipe failed");
    return nullptr;
  }

  std::thread write_thread([fd=pipe_descriptors[1], &config_parts]() {
    for (size_t i = 0; i < config_parts.size(); ++i) {
      if (i > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }
      write(fd, config_parts[i].data(), config_parts[i].size());
    }
    fmc_error_t *err;
    fmc_fclose(fd, &err);
  });

  auto sect = unique_sect(fmc_cfg_sect_parse_ini_file(spec, pipe_descriptors[0], "main", &err));

  close(pipe_descriptors[0]);
  write_thread.join();
  return sect;
}

unique_sect parse_cfg(std::string_view config, struct fmc_cfg_node_spec *spec, fmc_error_t *&err) {
  return parse_cfg(std::vector<std::string_view>({config}), spec, err);
}

TEST(parser, required_1) {
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
  EXPECT_ERR(err, "config error: missing required field int64 (line 9)");

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
  EXPECT_ERR(err, "config error: missing required field arr (line 1)");

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
  EXPECT_ERR(err, "config error: missing required field sect (line 1)");

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
  EXPECT_ERR(err, "config error: missing required field none (line 1)");

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
  EXPECT_ERR(err, "config error: missing required field boolean (line 1)");

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
  EXPECT_ERR(err, "config error: missing required field int64 (line 1)");

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
  EXPECT_ERR(err, "config error: missing required field str (line 1)");

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
  EXPECT_ERR(err, "config error: missing required field float64 (line 1)");
}

TEST(parser, unknown_field_1) {
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
  EXPECT_ERR(err, "config error: unknown field int63 (line 2)");
}

TEST(parser, unknown_section_1) {
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
  EXPECT_ERR(err, "config error: section main not found (line 0)");
}

TEST(parser, unknown_section_2) {
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
  EXPECT_ERR(err, "config error: section subsact not found (line 2)");
}

TEST(parser, invalid_array_1) {
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
  EXPECT_ERR(err, "config error: closing bracket was expected in array (line 2)");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=1]\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to parse field arr (line 2)");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=,1,2,3\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to parse field arr (line 2)");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=1,,3\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to parse int64 (line 2)");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=[1,2,3\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: closing bracket was expected in array (line 2)");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=1,2,3]\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to parse field arr (line 2)");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=[1,2a,3]\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: comma was expected in array (line 2)");
}

TEST(parser, invalid_array_2) {
  struct fmc_cfg_type subarray = {
      .type = FMC_CFG_NONE,
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
                        "arr=1,2,3\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to parse none (line 2)");
}

TEST(parser, invalid_array_3) {
  struct fmc_cfg_type subarray = {
      .type = FMC_CFG_BOOLEAN,
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
                        "arr=1,2,3\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to parse boolean (line 2)");
}

TEST(parser, invalid_array_4) {
  struct fmc_cfg_type subarray = {
      .type = FMC_CFG_FLOAT64,
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
                        "arr=a,b,c\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to parse float64 (line 2)");
}

TEST(parser, invalid_array_5) {
  struct fmc_cfg_type subarray = {
      .type = FMC_CFG_STR,
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
                        "arr=a,b,c\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to parse string (line 2)");
}

TEST(parser, invalid_array_6) {
  struct fmc_cfg_type subarray = {
      .type = FMC_CFG_STR,
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
                        "arr=\"a\",\"b\",\"c\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to find closing quotes for string (line 2)");
}

TEST(parser, invalid_ini_1) {
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
  EXPECT_ERR(err, "config error: line 2 is too long");

  sect = parse_cfg(""
                        "arr=1,2,3\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: key-value has no section (line 1)");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr1,2,3\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: invalid key-value entry (line 2)");
}

TEST(parser, ini_format_1) {
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
  auto sect = parse_cfg(std::vector<std::string_view>({
                          ""
                          "[main]\r\n"
                          "arr=1,2,","3    \r\n"}),
                        main, err);
  ASSERT_NOERR(err);
  EXPECT_EQ(cfg_to_string(sect), ""
                                 "{\n"
                                 "  arr = [\n"
                                 "    1,\n"
                                 "    2,\n"
                                 "    3,\n"
                                 "  ]\n"
                                 "}\n");
}

TEST(parser, string_1) {
  struct fmc_cfg_type subarray = {
      .type = FMC_CFG_STR,
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
                        "arr=\"a\",\"b\",\"c\"\n"
                        "",
                        main, err);
  ASSERT_NOERR(err);
  EXPECT_EQ(cfg_to_string(sect), ""
                                 "{\n"
                                 "  arr = [\n"
                                 "    \"a\",\n"
                                 "    \"b\",\n"
                                 "    \"c\",\n"
                                 "  ]\n"
                                 "}\n");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=\"a\",\"b\",\"c\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to find closing quotes for string (line 2)");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=\"a\",\"b,\"c\"\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: comma was expected in array (line 2)");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=a\",\"b\",\"c\"\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to parse string (line 2)");
}

TEST(parser, boolean_1) {
  struct fmc_cfg_type subarray = {
      .type = FMC_CFG_BOOLEAN,
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
                        "arr=true,false,false\n"
                        "",
                        main, err);
  ASSERT_NOERR(err);
  EXPECT_EQ(cfg_to_string(sect), ""
                                 "{\n"
                                 "  arr = [\n"
                                 "    1,\n"
                                 "    0,\n"
                                 "    0,\n"
                                 "  ]\n"
                                 "}\n");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=true,falsee,false\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: comma was expected in array (line 2)");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=truee,false,false\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: comma was expected in array (line 2)");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=abc,false,false\n"
                        "",
                        main, err);
  EXPECT_ERR(err, "config error: unable to parse boolean (line 2)");
}

TEST(parser, arraysect_1) {
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

  struct fmc_cfg_type subarray2 = {
      .type = FMC_CFG_SECT,
      .spec {
          .node = subsect,
      },
  };

  struct fmc_cfg_type subarray1 = {
      .type = FMC_CFG_ARR,
      .spec {
          .array = &subarray2,
      },
  };

  struct fmc_cfg_node_spec main[] = {
      fmc_cfg_node_spec{
        .key = "arr",
        .descr = "arr descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_ARR,
          .spec {
            .array = &subarray1,
          },
        },
      },
      fmc_cfg_node_spec{NULL},
  };
  fmc_error_t *err;
  auto sect = parse_cfg(""
                        "[main]\n"
                        "arr=[[sect1],[sect2,sect3]]\n"
                        "\n"
                        "[sect3]\n"
                        "int64=3\n"
                        "[sect1]\n"
                        "int64=1\n"
                        "[sect2]\n"
                        "int64=2\n"
                        "",
                        main, err);
  ASSERT_NOERR(err);
  EXPECT_EQ(cfg_to_string(sect), ""
                                 "{\n"
                                 "  arr = [\n"
                                 "    [\n"
                                 "      {\n"
                                 "        int64 = 1\n"
                                 "      },\n"
                                 "    ],\n"
                                 "    [\n"
                                 "      {\n"
                                 "        int64 = 2\n"
                                 "      },\n"
                                 "      {\n"
                                 "        int64 = 3\n"
                                 "      },\n"
                                 "    ],\n"
                                 "  ]\n"
                                 "}\n");

  sect = parse_cfg(""
                        "[main]\n"
                        "arr=sect1,sect2,sect3\n"
                        "\n"
                        "[sect3]\n"
                        "int64=3\n"
                        "[sect1]\n"
                        "int64=1\n"
                        "[sect2]\n"
                        "int64=2\n"
                        "",
                        main, err);
  ASSERT_NOERR(err);
  EXPECT_EQ(cfg_to_string(sect), ""
                                 "{\n"
                                 "  arr = [\n"
                                 "    [\n"
                                 "      {\n"
                                 "        int64 = 1\n"
                                 "      },\n"
                                 "      {\n"
                                 "        int64 = 2\n"
                                 "      },\n"
                                 "      {\n"
                                 "        int64 = 3\n"
                                 "      },\n"
                                 "    ],\n"
                                 "  ]\n"
                                 "}\n");
}

TEST(direct, basic_1) {
  fmc_error_t *err;

  auto sect = unique_sect(fmc_cfg_sect_item_add_none(nullptr, "none", &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_boolean(sect.release(), "booleantrue", true, &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_boolean(sect.release(), "booleanfalse", false, &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_int64(sect.release(), "int64", -45, &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_float64(sect.release(), "float64", -45.5, &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_str(sect.release(), "str", "message", &err));
  ASSERT_NOERR(err);

  auto arr = unique_arr(fmc_cfg_arr_item_add_none(nullptr, &err));
  ASSERT_NOERR(err);
  arr = unique_arr(fmc_cfg_arr_item_add_boolean(arr.release(), true, &err));
  ASSERT_NOERR(err);
  arr = unique_arr(fmc_cfg_arr_item_add_boolean(arr.release(), false, &err));
  ASSERT_NOERR(err);
  arr = unique_arr(fmc_cfg_arr_item_add_int64(arr.release(), -45, &err));
  ASSERT_NOERR(err);
  arr = unique_arr(fmc_cfg_arr_item_add_float64(arr.release(), -45.5, &err));
  ASSERT_NOERR(err);
  arr = unique_arr(fmc_cfg_arr_item_add_str(arr.release(), "message", &err));
  ASSERT_NOERR(err);

  auto subsect1 = unique_sect(fmc_cfg_sect_item_add_none(nullptr, "none", &err));
  ASSERT_NOERR(err);
  arr = unique_arr(fmc_cfg_arr_item_add_sect(arr.release(), subsect1.release(), &err));
  ASSERT_NOERR(err);

  auto subarr = unique_arr(fmc_cfg_arr_item_add_none(nullptr, &err));
  ASSERT_NOERR(err);
  subarr = unique_arr(fmc_cfg_arr_item_add_boolean(subarr.release(), true, &err));
  ASSERT_NOERR(err);
  arr = unique_arr(fmc_cfg_arr_item_add_arr(arr.release(), subarr.release(), &err));
  ASSERT_NOERR(err);

  sect = unique_sect(fmc_cfg_sect_item_add_arr(sect.release(), "arr", arr.release(), &err));
  ASSERT_NOERR(err);

  auto subsect2 = unique_sect(fmc_cfg_sect_item_add_none(nullptr, "none", &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_sect(sect.release(), "sect", subsect2.release(), &err));
  ASSERT_NOERR(err);

  EXPECT_EQ(cfg_to_string(sect), ""
                                  "{\n"
                                  "  sect = {\n"
                                  "    none = none\n"
                                  "  }\n"
                                  "  arr = [\n"
                                  "    [\n"
                                  "      1,\n"
                                  "      none,\n"
                                  "    ],\n"
                                  "    {\n"
                                  "      none = none\n"
                                  "    },\n"
                                  "    \"message\",\n"
                                  "    -45.500000,\n"
                                  "    -45,\n"
                                  "    0,\n"
                                  "    1,\n"
                                  "    none,\n"
                                  "  ]\n"
                                  "  str = \"message\"\n"
                                  "  float64 = -45.500000\n"
                                  "  int64 = -45\n"
                                  "  booleanfalse = 0\n"
                                  "  booleantrue = 1\n"
                                  "  none = none\n"
                                  "}\n"
                                 "");
}

TEST(check, test_1) {
  fmc_error_t *err;

  auto sect = unique_sect(fmc_cfg_sect_item_add_none(nullptr, "none", &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_boolean(sect.release(), "booleantrue", true, &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_boolean(sect.release(), "booleanfalse", false, &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_int64(sect.release(), "int64", -45, &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_float64(sect.release(), "float64", -45.5, &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_str(sect.release(), "str", "message", &err));
  ASSERT_NOERR(err);

  auto subarr1 = unique_arr(fmc_cfg_arr_item_add_boolean(nullptr, false, &err));
  ASSERT_NOERR(err);
  subarr1 = unique_arr(fmc_cfg_arr_item_add_boolean(subarr1.release(), true, &err));
  ASSERT_NOERR(err);
  auto arr = unique_arr(fmc_cfg_arr_item_add_arr(nullptr, subarr1.release(), &err));
  ASSERT_NOERR(err);

  auto subarr2 = unique_arr(fmc_cfg_arr_item_add_boolean(nullptr, false, &err));
  ASSERT_NOERR(err);
  subarr2 = unique_arr(fmc_cfg_arr_item_add_boolean(subarr2.release(), true, &err));
  ASSERT_NOERR(err);
  arr = unique_arr(fmc_cfg_arr_item_add_arr(arr.release(), subarr2.release(), &err));
  ASSERT_NOERR(err);

  sect = unique_sect(fmc_cfg_sect_item_add_arr(sect.release(), "arr", arr.release(), &err));
  ASSERT_NOERR(err);

  auto subsect2 = unique_sect(fmc_cfg_sect_item_add_none(nullptr, "none", &err));
  ASSERT_NOERR(err);
  sect = unique_sect(fmc_cfg_sect_item_add_sect(sect.release(), "sect", subsect2.release(), &err));
  ASSERT_NOERR(err);

  struct fmc_cfg_node_spec subsect[] = {
      fmc_cfg_node_spec{
        .key = "none",
        .descr = "none descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_NONE,
        },
      },
      fmc_cfg_node_spec{NULL},
  };

  struct fmc_cfg_type subarray2 = {
      .type = FMC_CFG_BOOLEAN,
  };

  struct fmc_cfg_type subarray1 = {
      .type = FMC_CFG_ARR,
      .spec {
          .array = &subarray2,
      },
  };

  struct fmc_cfg_node_spec main[] = {
      fmc_cfg_node_spec{
        .key = "missing",
        .descr = "missing descr",
        .required = false,
        .type = fmc_cfg_type{
          .type = FMC_CFG_INT64,
        },
      },
      fmc_cfg_node_spec{
        .key = "int64",
        .descr = "int64 descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_INT64,
        },
      },
      fmc_cfg_node_spec{
        .key = "booleantrue",
        .descr = "booleantrue descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_BOOLEAN,
        },
      },
      fmc_cfg_node_spec{
        .key = "booleanfalse",
        .descr = "booleanfalse descr",
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
        .descr = "arr descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_ARR,
          .spec {
            .array = &subarray1,
          },
        },
      },
      fmc_cfg_node_spec{NULL},
  };

  EXPECT_EQ(cfg_to_string(sect), ""
                                 "{\n"
                                 "  sect = {\n"
                                 "    none = none\n"
                                 "  }\n"
                                 "  arr = [\n"
                                 "    [\n"
                                 "      1,\n"
                                 "      0,\n"
                                 "    ],\n"
                                 "    [\n"
                                 "      1,\n"
                                 "      0,\n"
                                 "    ],\n"
                                 "  ]\n"
                                 "  str = \"message\"\n"
                                 "  float64 = -45.500000\n"
                                 "  int64 = -45\n"
                                 "  booleanfalse = 0\n"
                                 "  booleantrue = 1\n"
                                 "  none = none\n"
                                 "}\n"
                                 "");

  EXPECT_TRUE(fmc_cfg_node_spec_check(main, sect.get(), &err));
  ASSERT_NOERR(err);

  main[0].required = true;
  EXPECT_FALSE(fmc_cfg_node_spec_check(main, sect.get(), &err));
  EXPECT_ERR(err, "config error: missing required field missing");
  main[0].required = false;

  sect = unique_sect(fmc_cfg_sect_item_add_int64(sect.release(), "missssing", -101, &err));
  EXPECT_FALSE(fmc_cfg_node_spec_check(main, sect.get(), &err));
  EXPECT_ERR(err, "config error: unknown field missssing");

  auto p = sect.release();
  LL_DELETE(p, p);
  sect = unique_sect(p);

  sect = unique_sect(fmc_cfg_sect_item_add_int64(sect.release(), "int64", -101, &err));
  EXPECT_FALSE(fmc_cfg_node_spec_check(main, sect.get(), &err));
  EXPECT_ERR(err, "config error: duplicated field int64");
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
