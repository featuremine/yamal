/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <fmc/component.h>

#ifdef __cplusplus
extern "C" {
#endif

struct shutdown_component_enabled_cb {
  fmc_component_HEAD;
  size_t shutdown_count;
  size_t post_shutdown_count;
  size_t limit;
  size_t post_finish_count;
};

#ifdef __cplusplus
}
#endif
