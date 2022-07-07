/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

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

static bool fmc_exec(const char *cmd, const char **result) {
#if defined(FMC_SYS_WIN)
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
#else
  return false;
#endif
}

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
          .append("')>$TMP2; numdiff -q -V -s=',\\n' -a 1.0e-5  $TMP1 $TMP2");

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
