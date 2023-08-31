/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <fmc/error.h>

#include <ytp/stream.h>

void ytp_stream_close(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);

  for (size_t lstidx = YTP_STREAM_LIST_MIN; lstidx <= YTP_STREAM_LIST_MAX;
       ++lstidx) {
    ytp_yamal_close(yamal, lstidx, error);
  }
}
