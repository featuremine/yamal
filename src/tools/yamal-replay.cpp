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

#include <tclap/CmdLine.h>

#include <fmc++/time.hpp>
#include <fmc/signals.h>
#include <ytp/control.h>
#include <ytp/sequence.h>
#include <ytp/version.h>

#include <cstring>
#include <iostream>

#define CHECK(E) if (E) {goto error;}

int main(int argc, char **argv) {
  TCLAP::CmdLine cmd("yamal copy tool", ' ', YTP_VERSION);

  TCLAP::UnlabeledValueArg<std::string> srcArg("src", "source yamal path", true, "src", "string");
  cmd.add(srcArg);

  TCLAP::UnlabeledValueArg<std::string> destArg("dest", "destination yamal path", true, "dest", "string");
  cmd.add(destArg);

  cmd.parse(argc, argv);

  auto src_name = srcArg.getValue();
  auto dest_name = destArg.getValue();

  fmc_error_t *error;
  fmc_fd src_fd = -1;
  fmc_fd dest_fd = -1;
  ytp_yamal_t *src_yml = NULL;
  ytp_yamal_t *dest_yml = NULL;
  ytp_iterator_t it = NULL;
  size_t count = 0;
  size_t size = YTP_YAMAL_HEADER_SIZE;
  int64_t delta = 0;

  src_fd = fmc_fopen(src_name.c_str(), fmc_fmode::READ, &error);
  if (!fmc_fvalid(src_fd)) {
    std::cerr << "Unable to open file " << src_name << std::endl;
    goto error;
  }

  dest_fd = fmc_fopen(dest_name.c_str(), fmc_fmode::READWRITE, &error);
  if (!fmc_fvalid(dest_fd)) {
    std::cerr << "Unable to open file " << dest_name << std::endl;
    goto error;
  }

  src_yml = ytp_yamal_new(src_fd, &error);
  CHECK(error);
  dest_yml = ytp_yamal_new(dest_fd, &error);
  CHECK(error);

  it = ytp_yamal_begin(src_yml, &error);
  CHECK(error);
  for (; !ytp_yamal_term(it); it = ytp_yamal_next(src_yml, it, &error)) {
    CHECK(error);
    size_t sz;
    char *src;
    ytp_peer_t peer;
    ytp_channel_t ch;
    uint64_t time;
    ytp_time_read(src_yml, it, &peer, &ch, &time, &sz, (const char **)&src, &error);
    CHECK(error);
    if (!delta) {
      delta = fmc_cur_time_ns() - time;
    }
    time += delta;
    while (time > fmc_cur_time_ns());
    char *dest = ytp_time_reserve(dest_yml, sz, &error);
    CHECK(error);
    memcpy(dest, src, sz);
    ytp_time_commit(dest_yml, peer, ch, time, dest, &error);
    CHECK(error);
  }

  return 0;

error:
  std::cerr << fmc_error_msg(error) << std::endl;
  ytp_yamal_del(src_yml, &error);
  ytp_yamal_del(dest_yml, &error);
  fmc_fclose(src_fd, &error);
  fmc_fclose(dest_fd, &error);

  return -1;
}