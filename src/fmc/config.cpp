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
 * @file config.cpp
 * @date 13 Jul 2022
 * @brief Implementation of the fmc configuration API
 *
 * @see http://www.featuremine.com
 */

#include <fmc/config.h>
#include <exception>
#include <json/json.hpp>

#define JSON_PARSER_BUFF_SIZE 8192


struct fmc_cfg_sect_item *
fmc_cfg_sect_parse_json_file(struct fmc_cfg_node_spec *spec, fmc_fd fd,
                             fmc_error_t **err) {
  fmc_error_clear(err);
  struct fmc_cfg_sect_item *ret = NULL;

  try
  {

    std::string buffer;
    buffer.reserve(JSON_PARSER_BUFF_SIZE);
    while (fmc_fread(fd, buffer.data() + buffer.size(), JSON_PARSER_BUFF_SIZE, err)) {
      if (*err) {
        return nullptr;
      }
      buffer.reserve(buffer.size() + JSON_PARSER_BUFF_SIZE);
    }
    if (*err) {
      return nullptr;
    }

    auto jsonmsg = nlohmann::json::parse(buffer);



  }
  catch(const std::exception& e)
  {
    //TODO: set error
    return nullptr;
  }
  

  return ret;
do_cleanup:
  return ret;
}
