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
 * @file yamal.hpp
 * @date 29 Apr 2021
 * @brief File contains C declaration of the mmlist
 *
 * This file contains declarations of the memory mapped list.
 * @see http://www.featuremine.com
 */

#pragma once

#include <ytp/yamal.h>

#include <fmc++/error.hpp>

struct ytp_yamal_wrap : ytp_yamal_t {
  ytp_yamal_wrap(int fd, bool enable_thread, YTP_CLOSABLE_MODE closable);
  ~ytp_yamal_wrap();

  ytp_yamal_wrap(const ytp_yamal_wrap &) = delete;
  ytp_yamal_wrap &operator=(const ytp_yamal_wrap &) = delete;
};

inline ytp_yamal_wrap::ytp_yamal_wrap(int fd, bool enable_thread,
                                      YTP_CLOSABLE_MODE closable) {
  ytp_yamal_t *yamal = this;
  fmc_error_t *error;
  ytp_yamal_init_3(yamal, fd, enable_thread, closable, &error);
  if (error) {
    throw fmc::error(*error);
  }
}

inline ytp_yamal_wrap::~ytp_yamal_wrap() {
  ytp_yamal_t *yamal = this;
  fmc_error_t *error;
  ytp_yamal_destroy(yamal, &error);
}
