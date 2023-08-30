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

#include "yamal-common.hpp"

#include <tclap/CmdLine.h>

#include <fmc/process.h>
#include <ytp/version.h>

int main(int argc, char **argv) {
  TCLAP::CmdLine cmd("yamal copy tool", ' ', YTP_VERSION);

  TCLAP::UnlabeledValueArg<std::string> srcArg("src", "source yamal path", true,
                                               "src", "string");
  cmd.add(srcArg);

  TCLAP::UnlabeledValueArg<std::string> destArg(
      "dest", "destination yamal path", true, "dest", "string");
  cmd.add(destArg);

  TCLAP::ValueArg<int> affinityArg("a", "affinity",
                                   "set the CPU affinity of the main process",
                                   false, 0, "cpuid");
  cmd.add(affinityArg);

  TCLAP::ValueArg<int> auxArg("x", "auxiliary",
                              "set the CPU affinity of the auxiliary process",
                              false, 0, "cpuid");
  cmd.add(auxArg);

  cmd.parse(argc, argv);

  if (auxArg.isSet()) {
    ytp_yamal_set_aux_thread_affinity(auxArg.getValue());
  }

  if (affinityArg.isSet()) {
    fmc_error_t *error;
    auto cur_thread = fmc_tid_cur(&error);
    if (error) {
      std::string message;
      message += "Error getting current thread: ";
      message += fmc_error_msg(error);
      std::cerr << message << std::endl;
      return -1;
    } else {
      auto cpuid = affinityArg.getValue();
      fmc_set_affinity(cur_thread, cpuid, &error);
      if (error) {
        std::string message;
        message += "Error set affinity: ";
        message += fmc_error_msg(error);
        std::cerr << message << std::endl;
        return -1;
      }
    }
  }

  auto &src_name = srcArg.getValue();
  auto &dest_name = destArg.getValue();

  struct handler_t {
    bool on_message(ytp_yamal_t *yamal, int64_t ts, size_t sz,
                    fmc_error_t **error) {
      if (shift != 0) {
        auto wait_until = ts + shift;
        while (fmc_cur_time_ns() < wait_until)
          ;
      } else {
        shift = fmc_cur_time_ns() - ts;
      }
      fmc_error_clear(error);
      return true;
    }

    void on_error(fmc_error_t *error) {
      stop = true;
      ret = 1;
      std::cerr << fmc_error_msg(error) << std::endl;
    }

    int64_t shift = 0;
    int ret = 0;
    bool stop = false;
  } handler;

  ann_cl_t ann_cl(handler);
  ann_cl.init(src_name.c_str(), dest_name.c_str());
  while (!handler.stop) {
    fmc_error_t *error;
    auto polled = ytp_cursor_poll(ann_cl.cursor, &error);
    if (error) {
      handler.on_error(error);
      break;
    }
    if (!polled) {
      break;
    }
  }

  return handler.ret;
}
