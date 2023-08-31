/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include "yamal-common.hpp"

#include <tclap/CmdLine.h>

#include <ytp/version.h>

int main(int argc, char **argv) {
  TCLAP::CmdLine cmd("yamal copy tool", ' ', YTP_VERSION);

  TCLAP::UnlabeledValueArg<std::string> srcArg("src", "source yamal path", true,
                                               "src", "string");
  cmd.add(srcArg);

  TCLAP::UnlabeledValueArg<std::string> destArg(
      "dest", "destination yamal path", true, "dest", "string");
  cmd.add(destArg);

  TCLAP::ValueArg<ssize_t> countArg("n", "count", "number of messages to copy",
                                    false, -1, "unsigned");
  cmd.add(countArg);

  TCLAP::ValueArg<ssize_t> sizeArg(
      "s", "size", "maximum amount of data to copy", false, -1, "megabytes");
  cmd.add(sizeArg);

  TCLAP::SwitchArg fArg("f", "follow", "copy data as the file grows;");
  cmd.add(fArg);

  cmd.parse(argc, argv);

  auto &src_name = srcArg.getValue();
  auto &dest_name = destArg.getValue();
  auto max_count = countArg.getValue();
  auto max_size = 1024ll * 1024ll * sizeArg.getValue();

  struct handler_t {
    bool on_message(ytp_yamal_t *yamal, int64_t ts, size_t sz,
                    fmc_error_t **error) {
      if (max_count >= 0) {
        stop = stop || max_count-- > 0;
      }
      if (max_size >= 0) {
        stop = stop ||
               (size_t)max_size >= ytp_yamal_reserved_size(yamal, error) + sz;
      }
      return !stop;
    }

    void on_error(fmc_error_t *error) {
      stop = true;
      ret = 1;
      std::cerr << fmc_error_msg(error) << std::endl;
    }

    ssize_t max_count;
    ssize_t max_size;
    int ret = 0;
    bool stop = false;
  } handler{max_count, max_size};

  ann_cl_t ann_cl(handler);
  ann_cl.init(src_name.c_str(), dest_name.c_str());
  while (!handler.stop) {
    fmc_error_t *error;
    auto polled = ytp_cursor_poll(ann_cl.cursor, &error);
    if (error) {
      handler.on_error(error);
      return handler.ret;
    }
    if (!polled && !fArg.getValue()) {
      break;
    }
  }

  return handler.ret;
}
