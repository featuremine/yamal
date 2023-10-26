/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file files.h
 * @author Federico Ravchina
 * @date 10 May 2021
 * @brief File contains C declaration of fmc files API
 *
 * This file contains C declaration of fmc files API.
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/error.h>
#include <fmc/platform.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(FMC_SYS_WIN)
#include <windows.h>
typedef HANDLE fmc_fd;
typedef HANDLE fmc_md;
#define FMC_MAX_PATH 256
#elif defined(FMC_SYS_UNIX)
typedef int fmc_fd;
#define FMC_MAX_PATH 1024
#else
#error "OS is not supported"
#endif

struct fmc_fview {
  void *mem;
#if defined(FMC_SYS_WIN)
  fmc_md md;
#endif
};

typedef struct fmc_fview fmc_fview_t;

// open file flags
typedef enum {
  NONE = 0,
  READ = 2,
  WRITE = 4,
  READWRITE = READ | WRITE
} fmc_fmode;

/**
 * @brief Creates a the base directory of a file path
 *
 * @param file_path path to the directory to create
 * @param error out-parameter for error handling
 * @return true if created or already exists, false otherwise
 */
FMMODFUNC void fmc_basedir_mk(const char *file_path, fmc_error_t **error);

/**
 * @brief Checks if the base directory of file path exists
 *
 * @param file_path path to the directory to check
 * @param error out-parameter for error handling
 * @return true if base directory of file_path exists, false otherwise
 */
FMMODFUNC bool fmc_basedir_exists(const char *file_path, fmc_error_t **error);

/**
 * @brief Join two paths.
 * If sz if 0, it returns the length of the result string, not including the null terminating character.
 *
 * @param dest buffer to store the string with the final path.
 * @param sz size of dest buffer.
 * @param p1 base path to join
 * @param p2 last part of the path to join
 * @return the number of characters that would have been written on the
 * buffer, if ‘sz’ had been sufficiently large
 */
FMMODFUNC int fmc_path_join(char *dest, size_t sz, const char *p1,
                            const char *p2);

/**
 * @brief Obtain the path of the parent directory
 * If sz if 0, it returns the length of the result string, not including the null terminating character.
 *
 * @param dest buffer to store the string with the final path.
 * @param sz size of dest buffer.
 * @param src string with the source path.
 * @return the number of characters that would have been written on the
 * buffer, if ‘sz’ had been sufficiently large
 */
FMMODFUNC int fmc_path_parent(char *dest, size_t sz, const char *src);

/**
 * @brief Obtain path of current executable
 * If sz if 0, it returns the length of the result string, not including the null terminating character.
 *
 * @param dest buffer to store the string with the final path.
 * @param sz size of dest buffer.
 * @return the number of characters that would have been written on the
 * buffer, if ‘sz’ had been sufficiently large
 */
FMMODFUNC int fmc_exec_path_get(char *dest, size_t sz);

/**
 * @brief Opens a process by creating a pipe, forking, and invoking the shell
 *
 * @param command a shell command line
 * @param read_mode must contain either the letter 'r' for reading or the letter
 * 'w' for writing
 * @param error out-parameter for error handling
 * @return on success, returns a pointer to an open stream, NULL otherwise
 */
FMMODFUNC FILE *fmc_popen(const char *command, const char *read_mode,
                          fmc_error_t **error);

/**
 * @breif Waits for the associated process to terminate
 *
 * @param pipe pointer to an open stream
 * @param error out-parameter for error handling
 * @return on success, returns the exit status of the command, -1 otherwise
 */
FMMODFUNC int fmc_pclose(FILE *pipe, fmc_error_t **error);

/**
 * @brief Creates a temporary file
 *
 * @param error out-parameter for error handling
 * @return fmc_fd file descriptor of a temporary file
 */
FMMODFUNC fmc_fd fmc_ftemp(fmc_error_t **error);

/**
 * @brief Creates a temporary file.
 * For UNIX systems the argument input string is the temporary file path
 * template according to mkstemp.
 * For Windows system a large null terminated dummy string should be
 * provided to specify the maximum temporary file path length.
 *
 * @param file_path input/output string with the temp file template.
 * Also, outputs the new temp. file path.
 * @param error out-parameter for error handling
 * @return fmc_fd file descriptor of a temporary file
 */
FMMODFUNC fmc_fd fmc_ftemp_2(char *file_path, fmc_error_t **error);

/**
 * @brief Checks if a file path corresponds to an existing file or directory
 *
 * @param file_path path to the file or directory to check
 * @param error out-parameter for error handling
 * @return true if the file or directory that corresponds to file_path exists,
 * false otherwise
 */
FMMODFUNC bool fmc_fexists(const char *file_path, fmc_error_t **error);

/**
 * @brief Get file descriptor of a given a FILE object
 *
 * @param file FILE object
 * @param error out-parameter for error handling
 * @return fmc_fd file descriptor
 */
FMMODFUNC fmc_fd fmc_fd_get(FILE *file, fmc_error_t **error);

/**
 * @brief Opens a file given the file path and file access mode.
 *
 * @param file_path path to the file to open/create
 * @param flags file access mode
 * @param error out-parameter for error handling
 * @return fmc_fd file descriptor
 */
FMMODFUNC fmc_fd fmc_fopen(const char *path, fmc_fmode flags,
                           fmc_error_t **error);

/**
 * @brief Reads a file descriptor
 *
 * @param fd file descriptor
 * @param buf out-parameter buffer
 * @param bytes maximum number of bytes to read
 * @param error out-parameter for error handling
 * @return the number of bytes read on success, zero on end of file and -1 on
 * error.
 */
FMMODFUNC ssize_t fmc_fread(fmc_fd fd, void *buf, size_t bytes,
                            fmc_error_t **error);

/**
 * @brief Closes a file descriptor
 *
 * @param fd file descriptor
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_fclose(fmc_fd fd, fmc_error_t **error);

/**
 * @brief Checks if a file descriptor is valid
 *
 * @param fd file descriptor
 * @return true if the file descriptor is valid, false otherwise
 */
FMMODFUNC bool fmc_fvalid(fmc_fd fd);

/**
 * @brief Checks if a file descriptor access is readonly
 *
 * @param fd file descriptor
 * @return true if the file descriptor is readonly, false otherwise
 */
FMMODFUNC bool fmc_freadonly(fmc_fd fd);

/**
 * @brief Returns the size of the file
 *
 * @param fd file descriptor
 * @param error out-parameter for error handling
 * @return the size of the file on success, (size_t)-1 otherwise
 */
FMMODFUNC size_t fmc_fsize(fmc_fd fd, fmc_error_t **error);

/**
 * @brief Resizes the file given the new size
 *
 * @param fd file descriptor
 * @param new_size the new size in bytes
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_fresize(fmc_fd fd, size_t new_size, fmc_error_t **error);

/**
 * @brief Allocates file space given the size
 *
 * @param fd file descriptor
 * @param sz size to allocate
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_falloc(fmc_fd fd, long long sz, fmc_error_t **error);

/**
 * @brief Maps a file descriptor into memory
 *
 * @param view the memory mapping handler
 * @param length the length of the mapping
 * @param fd the file descriptor
 * @param offset the offset of the mapping
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_fview_init(fmc_fview_t *view, size_t length, fmc_fd fd,
                              int64_t offset, fmc_error_t **error);

/**
 * @brief Destroys a memory mapping handler
 *
 * @param view the memory mapping handler
 * @param length the length of the mapping
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_fview_destroy(fmc_fview_t *view, size_t length,
                                 fmc_error_t **error);

/**
 * @brief Returns a pointer to the mapped area
 *
 * @param view the memory mapping handler
 * @return a pointer to the mapped area
 */
FMMODFUNC void *fmc_fview_data(fmc_fview_t *view);

/**
 * @brief Remaps a virtual memory address
 *
 * @param view the memory mapping handler
 * @param fd the file descriptor
 * @param old_size the old size of the mapping
 * @param new_size the new size of the mapping
 * @param offset the offset of the mapping
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_fview_remap(fmc_fview_t *view, fmc_fd fd, size_t old_size,
                               size_t new_size, size_t offset,
                               fmc_error_t **error);

/**
 * @brief Synchronizes a file with a memory map
 *
 * @param view the memory mapping handler
 * @param size the size of the mapping
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_fview_sync(fmc_fview_t *view, size_t size,
                              fmc_error_t **error);

/**
 * @brief Make all changes done to all files actually appear on disk
 */
FMMODFUNC void fmc_fflush();

/**
 * @brief Write to file descriptor
 */
FMMODFUNC ssize_t fmc_write(fmc_fd fd, const void *buf, size_t n);

#ifdef __cplusplus
}
#endif
