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

#include <fmc/error.h>

#include <ytp/stream.h>

void ytp_stream_close(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);

  for (size_t lstidx = YTP_STREAM_LIST_MIN; lstidx <= YTP_STREAM_LIST_MAX; ++lstidx) {
    ytp_yamal_close(yamal, lstidx, error);
  }
}
