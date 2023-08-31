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

/*Components for single input and output test*/

struct producer_component {
  fmc_component_HEAD;
  size_t count;
};

struct consumer_component {
  fmc_component_HEAD;
  size_t executed;
  fmc_error_t *e;
  struct fmc_shmem mem;
};

/*Components for multiple inputs and outputs test*/

struct consumer_component_2 {
  fmc_component_HEAD;
  size_t first;
  size_t second;
  size_t executed;
  fmc_error_t *e;
};

struct consumer_component_3 {
  fmc_component_HEAD;
  size_t third;
  size_t fourth;
  size_t fifth;
  size_t executed;
  fmc_error_t *e;
};

#ifdef __cplusplus
}
#endif
