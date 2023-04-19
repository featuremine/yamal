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

#include "control.hpp"
#include "timeline.hpp"
#include "yamal.hpp"

#include <vector>
#include <ytp/control.h>
#include <ytp/sequence.h>
#include <ytp/yamal.h>
#include <ytp/sequence.h>

struct ytp_sequence {
  ytp_control_t ctrl;
  ytp_timeline_t timeline;
};
