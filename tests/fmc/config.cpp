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
using namespace std;

TEST(error, simple_types_1) {
  fmc_error_t *err;
  fmc_fd pipe_descriptors[2];

  ASSERT_EQ(pipe(pipe_descriptors), 0);

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
        .key = "none",
        .descr = "none descr",
        .required = true,
        .type = fmc_cfg_type{
          .type = FMC_CFG_NONE,
        },
      },
  };

  std::string_view config = "[main]\n"
                            "int64=123\n"
                            "boolean=true\n"
                            "float64=1.2\n"
                            "none=none\n";

  write(pipe_descriptors[1], config.data(), config.size());
  fmc_fclose(pipe_descriptors[1], &err);
  ASSERT_EQ(err, nullptr);

  fmc_cfg_sect_parse_ini_file(spec, pipe_descriptors[0], "main", &err);
  ASSERT_EQ(err, nullptr);
  fmc_fclose(pipe_descriptors[0], &err);
  ASSERT_EQ(err, nullptr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
