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

#include <atomic>
#include <condition_variable>
#include <fmc/endianness.h>
#include <fmc/files.h>
#include <mutex>
#include <thread>
#include <ytp/yamal.h>

static const size_t fm_mmlist_page_count = 1024 * 64 * 8;

struct ytp_yamal {
  struct yamal_hdr_t *header(fmc_error_t **err);
  fmc_fd fd;
  std::condition_variable cv_;
  std::mutex m_;
  std::mutex pa_mutex_;
  std::thread thread_;
  bool done_ = false;
  bool readonly_ = false;
  fmc_fview pages[fm_mmlist_page_count] = {{0}};
};

#if !defined(YTP_USE_BIG_ENDIAN)
#define ye64toh(x) fmc_le64toh(x)
#define htoye64(x) fmc_htole64(x)
#define ye16toh(x) fmc_le16toh(x)
#define htoye16(x) fmc_htole16(x)
#if FMC_BYTE_ORDER == FMC_LITTLE_ENDIAN
#define DIRECT_BYTE_ORDER
#endif
#else
#define ye64toh(x) fmc_be64toh(x)
#define htoye64(x) fmc_htobe64(x)
#define ye16toh(x) fmc_be16toh(x)
#define htoye16(x) fmc_htobe16(x)
#if FMC_BYTE_ORDER == FMC_BIG_ENDIAN
#define DIRECT_BYTE_ORDER
#endif
#endif
