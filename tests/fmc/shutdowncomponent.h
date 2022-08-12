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

#ifdef __cplusplus
extern "C" {
#endif

struct shutdown_component_enabled_cb {
  fmc_component_HEAD;
  size_t shutdown_count;
  size_t post_shutdown_count;
  size_t limit;
};

#ifdef __cplusplus
}
#endif
