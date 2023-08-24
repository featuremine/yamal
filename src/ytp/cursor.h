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

#pragma once

#include <ytp/yamal.h>

#include <uthash/utarray.h>
#include <uthash/uthash.h>

#include <stddef.h>

struct ytp_cursor {
  ytp_yamal_t *yamal;
  ytp_iterator_t it_data;
  ytp_iterator_t it_ann;
  uint64_t ann_processed;

  struct ytp_cursor_streams_data_item_t *cb_data;

  UT_array cb_ann;
  int cb_ann_locked;
};

struct ytp_cursor_streams_data_item_t {
  UT_hash_handle hh;
  ytp_mmnode_offs stream;
  UT_array cb_data;
  int cb_data_locked;
};

extern struct ytp_cursor_streams_data_item_t *
streams_data_get(struct ytp_cursor_streams_data_item_t *m, ytp_mmnode_offs key);
extern struct ytp_cursor_streams_data_item_t *
streams_data_emplace(struct ytp_cursor_streams_data_item_t **m,
                     ytp_mmnode_offs key, fmc_error_t **error);
