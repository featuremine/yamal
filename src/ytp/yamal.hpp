/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
