/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file test.cpp
 * @author Maxim Trokhimtchouk
 * @date 11 Aug 2017
 * @brief Test utilities
 *
 * @see http://www.featuremine.com
 */

#include <fmc++/mpl.hpp>
#include <fmc/platform.h>
#include <fmc/test.h>

#include <stdlib.h>
#include <string>
#if defined(FMC_SYS_UNIX)
#include <sys/wait.h>
#include <unistd.h>
#endif // FMC_SYS_UNIX

using namespace std;

#if defined(FMC_SYS_WIN)
static bool fmc_exec(const char *cmd, const char **result) {
  static thread_local std::string result_buffer;
  char buffer[128];
  result_buffer = "";
  FILE *pipe = _popen(cmd, "r");
  fmc_system_error_unless(pipe) << "popen() failed!";
  try {
    while (fgets(buffer, sizeof buffer, pipe) != NULL) {
      result_buffer += buffer;
    }
  } catch (...) {
    _pclose(pipe);
    fmc_system_error_unless(false) << "pipe error";
    return false;
  }

  *result = result_buffer.c_str();

  return _pclose(pipe) == 0;
}
#endif

bool fmc_run_base_vs_test_diff(const char *base, const char *test) {
#if defined(FMC_SYS_UNIX)
  string cmd = "diff -q ";
  cmd.append(base);
  cmd.append(" ");
  cmd.append(test);

  int pipefd[2] = {0};
  fmc_system_error_unless(pipe(pipefd) == 0) << "cannot create pipe";
  pid_t cpid;
  int s = 0;

  fmc_system_error_unless((cpid = fork()) != -1) "cannot fork";

  int status = 0;
  if (cpid == 0) {
    close(pipefd[0]);
    dup2(pipefd[1], 1);
    dup2(pipefd[1], 2);
    execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), (char *)NULL);
    cerr << "error running command " << cmd.c_str() << endl;
    close(pipefd[1]);
    _exit(EXIT_FAILURE);
  } else {
    close(pipefd[1]);
    waitpid(cpid, &status, 0);
    const size_t LEN = 64 * 1024;
    char buf[LEN];
    s = read(pipefd[0], buf, LEN);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      fmc_system_error_unless(write(2, buf, s) == s) << "could not write to "
                                                        "stderr";
    }
    close(pipefd[0]);
  }
  return WIFEXITED(status) && WEXITSTATUS(status) == 0;
#else
  const char *msg = NULL;
  auto ret = fmc_exec(
      string("FC ").append(base).append(" ").append(test).c_str(), &msg);
  if (!ret) {
    cerr << msg << endl;
  }
  return ret;
#endif
}

bool fmc_numdiff_base_vs_test(const char *base, const char *test) {
#if defined(FMC_SYS_UNIX)
  string cmd =
      string("TMP1=`mktemp -t tmp.XXXXXX`; TMP2=`mktemp -t tmp.XXXXXX`; (sed "
             "'s/-nan/nan/' '")
          .append(base)
          .append("')>$TMP1; (sed 's/-nan/nan/' '")
          .append(test)
          .append("')>$TMP2; numdiff -q -V -s=',\\n' -a 1.0e-5 -r 1.0e-15  "
                  "$TMP1 $TMP2");

  int pipefd[2] = {0};
  fmc_system_error_unless(pipe(pipefd) == 0) << "cannot create pipe";
  pid_t cpid;
  int s = 0;

  fmc_system_error_unless((cpid = fork()) != -1) "cannot fork";

  const size_t LEN = 64 * 1024;
  char buf[LEN];

  int status = 0;
  if (cpid == 0) {
    close(pipefd[0]);
    dup2(pipefd[1], 1);
    dup2(pipefd[1], 2);
    execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), (char *)NULL);
    cerr << "error running command " << cmd.c_str() << endl;
    close(pipefd[1]);
    _exit(EXIT_FAILURE);
  } else {
    close(pipefd[1]);
    waitpid(cpid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      s = read(pipefd[0], buf, LEN);
      fmc_system_error_unless(write(2, buf, s) == s) << "could not write to "
                                                        "stderr";
    }
    close(pipefd[0]);
  }
  return WIFEXITED(status) && WEXITSTATUS(status) == 0;
#else
  const char *msg = NULL;
  auto ret = fmc_exec(string("numdiff -q -V -s=',\\n' -a 1.0e-5 ")
                          .append(base)
                          .append(" ")
                          .append(test)
                          .c_str(),
                      &msg);
  if (!ret) {
    cerr << msg << endl;
  }
  return true;
#endif
}
