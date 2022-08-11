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

#include <fmc/component.h>
#include <fmc/config.h>
#include <fmc/error.h>
#include <fmc/string.h>
#include <fmc/time.h>
#include <stdlib.h>
#include <string.h>
#include <uthash/utlist.h>

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
};

/*Components for multiple inputs and outputs test*/

struct producer_component_3 {
  fmc_component_HEAD;
  size_t first;
  size_t second;
  size_t third;
};

struct producer_component_2 {
  fmc_component_HEAD;
  size_t fourth;
  size_t fifth;
};

struct consumer_component_2 {
  fmc_component_HEAD;
  size_t first;
  size_t second;
  size_t executed;
};

struct consumer_component_3 {
  fmc_component_HEAD;
  size_t third;
  size_t fourth;
  size_t fifth;
  size_t executed;
};

#ifdef __cplusplus
}
#endif
