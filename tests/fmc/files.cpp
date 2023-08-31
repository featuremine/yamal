/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file files.cpp
 * @author Federico Ravchina
 * @date 28 Jun 2021
 * @brief File contains tests for FMC files API
 *
 * @see http://www.featuremine.com
 */

#include <fmc++/fs.hpp>
#include <fmc++/gtestwrap.hpp>
#include <fmc/files.h>
#include <fmc/platform.h>

using namespace std;

#define ERROR_UNINITIALIZED_VALUE ((fmc_error_t *)1)

TEST(fmc, basedir_mk_new) {
  fmc_error_t *error;
  char file_path[] = "/tmp/fm-XXXXXX";
  fmc_ftemp_2(file_path, &error);
  std::string name1(file_path);

  fs::path basedir = name1;
  fs::path filename = basedir / "file1.txt";

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_basedir_mk((filename.c_str()), &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_TRUE(fs::is_directory(name1));
  ASSERT_TRUE(fs::remove(name1));
}

TEST(fmc, basedir_mk_existing) {
  fmc_error_t *error;
  char file_path[] = "/tmp/fm-XXXXXX";
  fmc_ftemp_2(file_path, &error);
  std::string name1(file_path);

  fs::path basedir = name1;
  fs::path filename = basedir / "file1.txt";

  fs::create_directories(name1);

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_basedir_mk((filename.c_str()), &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_TRUE(fs::is_directory(name1));
  ASSERT_TRUE(fs::remove(name1));
}

TEST(fmc, basedir_mk_curr_dir) {
  fmc_error_t *error;

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_basedir_mk(("file1.txt"), &error);
  ASSERT_EQ(error, nullptr);
}

TEST(fmc, basedir_exists_unexisting) {
  fmc_error_t *error;
  char file_path[] = "/tmp/fm-XXXXXX";
  fmc_ftemp_2(file_path, &error);
  std::string name1(file_path);

  fs::path basedir = name1;
  fs::path filename1 = basedir / "file1.txt";
  fs::path filename2 = basedir / "invalid" / "file1.txt";

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_FALSE(fmc_basedir_exists(filename1.c_str(), &error));
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_FALSE(fmc_basedir_exists(filename2.c_str(), &error));
  ASSERT_EQ(error, nullptr);
}

TEST(fmc, basedir_exists_existing) {
  fmc_error_t *error;
  char file_path[] = "/tmp/fm-XXXXXX";
  fmc_ftemp_2(file_path, &error);
  std::string name1(file_path);

  fs::path basedir = name1;
  fs::path filename = basedir / "file1.txt";

  fs::create_directories(name1);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_TRUE(fmc_basedir_exists(filename.c_str(), &error));
  ASSERT_EQ(error, nullptr);

  ASSERT_TRUE(fs::remove(name1));
}

TEST(fmc, popen_success) {
  fmc_error_t *error;

  error = ERROR_UNINITIALIZED_VALUE;
  FILE *cmd = fmc_popen("echo hello ; exit 0", "r", &error);
  ASSERT_EQ(error, nullptr);

  char data[128];
  ASSERT_EQ(fscanf(cmd, "%s", data), 1);

  ASSERT_EQ(std::string_view(data), "hello");

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_EQ(fmc_pclose(cmd, &error), 0);
  ASSERT_EQ(error, nullptr);
}

TEST(fmc, popen_fail) {
  fmc_error_t *error;

  error = ERROR_UNINITIALIZED_VALUE;
  FILE *cmd = fmc_popen("echo failure ; exit 1", "r", &error);
  ASSERT_EQ(error, nullptr);

  char data[128];
  ASSERT_EQ(fscanf(cmd, "%s", data), 1);

  ASSERT_EQ(std::string_view(data), "failure");

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_EQ(fmc_pclose(cmd, &error), 1);
  ASSERT_EQ(error, nullptr);
}

TEST(fmc, ftemp) {
  fmc_error_t *error;

  error = ERROR_UNINITIALIZED_VALUE;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

#if defined(FMC_SYS_WIN)
#error "Unsupported test"
#else
  struct stat stat_data {};
  ASSERT_EQ(fstat(fd, &stat_data), 0);
  ASSERT_EQ(stat_data.st_nlink, 0);
#endif

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(fmc, fexists_unexisting) {
  fmc_error_t *error;

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_FALSE(fmc_fexists("invalid_file.txt", &error));
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_FALSE(fmc_fexists("invalid_path/invalid_file.txt", &error));
  ASSERT_EQ(error, nullptr);
}

TEST(fmc, fexists_existing) {
  fmc_error_t *error;
  char file_path[] = "/tmp/fm-XXXXXX";
  fmc_ftemp_2(file_path, &error);
  std::string name1(file_path);

  error = ERROR_UNINITIALIZED_VALUE;
  auto fd = fmc_fopen(name1.c_str(), fmc_fmode::READWRITE, &error);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_TRUE(fmc_fexists(name1.c_str(), &error));
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_TRUE(fs::remove(name1));
}

TEST(fmc, fsize) {
  fmc_error_t *error;

  error = ERROR_UNINITIALIZED_VALUE;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_EQ(fmc_fsize(fd, &error), 0);
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(write(fd, "123", 3), 3);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_EQ(fmc_fsize(fd, &error), 3);
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(write(fd, "123", 3), 3);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_EQ(fmc_fsize(fd, &error), 6);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fresize(fd, 2, &error);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_EQ(fmc_fsize(fd, &error), 2);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fresize(fd, 10, &error);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_EQ(fmc_fsize(fd, &error), 10);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(fmc, falloc) {
  fmc_error_t *error;

  error = ERROR_UNINITIALIZED_VALUE;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(write(fd, "123456", 6), 6);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_EQ(fmc_fsize(fd, &error), 6);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_falloc(fd, 2, &error);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_EQ(fmc_fsize(fd, &error), 6);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_falloc(fd, 10, &error);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  ASSERT_EQ(fmc_fsize(fd, &error), 10);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(fmc, fview) {
  fmc_error_t *error;

  error = ERROR_UNINITIALIZED_VALUE;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  fmc_fview_t view;

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_falloc(fd, 16, &error);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fview_init(&view, 8, fd, 0, &error);
  ASSERT_EQ(error, nullptr);

  {
    char *data = (char *)fmc_fview_data(&view);
    strcpy(data, "HELLO");

    error = ERROR_UNINITIALIZED_VALUE;
    fmc_fview_sync(&view, 8, &error);
    ASSERT_EQ(error, nullptr);

    char buffer[8];
#if defined(FMC_SYS_UNIX)
    ASSERT_EQ(read(fd, buffer, 8), 8);
#else
#error "Unsupported test"
#endif

    char expected[] = {'H', 'E', 'L', 'L', 'O', '\0', '\0', '\0'};
    ASSERT_EQ(memcmp(buffer, expected, 8), 0);
  }

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fview_remap(&view, fd, 8, 16, 0, &error);
  ASSERT_EQ(error, nullptr);

  {
    char *data = (char *)fmc_fview_data(&view);
    strcpy(data + 8, "WORLD");

    error = ERROR_UNINITIALIZED_VALUE;
    fmc_fview_sync(&view, 16, &error);
    ASSERT_EQ(error, nullptr);

    char buffer[8];
#if defined(FMC_SYS_UNIX)
    ASSERT_EQ(read(fd, buffer, 8), 8);
#else
#error "Unsupported test"
#endif

    char expected[] = {'W', 'O', 'R', 'L', 'D', '\0', '\0', '\0'};
    ASSERT_EQ(memcmp(buffer, expected, 8), 0);
  }

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fview_destroy(&view, 8, &error);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(fmc, freadonly) {
  fmc_error_t *error;
  char file_path1[] = "/tmp/fm-XXXXXX";
  fmc_ftemp_2(file_path1, &error);
  std::string name1(file_path1);
  char file_path2[] = "/tmp/fm-XXXXXX";
  fmc_ftemp_2(file_path2, &error);
  std::string name2(file_path2);

  error = ERROR_UNINITIALIZED_VALUE;
  auto fd_rdwr = fmc_fopen(name1.c_str(), fmc_fmode::READWRITE, &error);
  ASSERT_EQ(error, nullptr);

  error = ERROR_UNINITIALIZED_VALUE;
  auto fd_rdonly = fmc_fopen(name2.c_str(), fmc_fmode::READ, &error);
  ASSERT_NE(error, nullptr);
  ASSERT_FALSE(fmc_fvalid(fd_rdonly));

  EXPECT_FALSE(fmc_freadonly(fd_rdwr));

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fclose(fd_rdwr, &error);
  ASSERT_EQ(error, nullptr);

  // Once file was created and initialized, retry to open as readonly
  error = ERROR_UNINITIALIZED_VALUE;
  fd_rdonly = fmc_fopen(name1.c_str(), fmc_fmode::READ, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_TRUE(fmc_fvalid(fd_rdonly));
  EXPECT_TRUE(fmc_freadonly(fd_rdonly));

  error = ERROR_UNINITIALIZED_VALUE;
  fmc_fclose(fd_rdonly, &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_TRUE(fs::remove(name1));
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
