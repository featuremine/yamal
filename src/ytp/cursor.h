/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
