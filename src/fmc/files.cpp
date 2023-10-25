/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file files.cpp
 * @author Federico Ravchina
 * @date 10 May 2021
 * @brief File contains C implementation of fmc files API
 *
 * This file contains C implementation of fmc files API.
 * @see http://www.featuremine.com
 */

#include <fmc++/fs.hpp>
#include <fmc/files.h>
#include <fmc/platform.h>
#include <cstring>

#if defined(FMC_SYS_MACH)
#include <fcntl.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach/machine.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#elif defined(FMC_SYS_UNIX)
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#elif defined(FMC_SYS_WIN)
#include <io.h>
#include <memoryapi.h>
#include <windows.h>
#endif

void fmc_basedir_mk(const char *file_path, fmc_error_t **error) {
  fmc_error_clear(error);
  auto path = fs::path(file_path);
  auto parent_path = path.parent_path();
  if (parent_path.empty()) {
    return;
  }
  std::error_code ec;
  fs::create_directories(parent_path, ec);
  if (ec) {
    auto msg = ec.message();
    FMC_ERROR_REPORT(error, msg.c_str());
  }
}

bool fmc_basedir_exists(const char *file_path, fmc_error_t **error) {
  fmc_error_clear(error);
  auto path = fs::path(file_path);
  auto parent_path = path.parent_path();
  if (parent_path.empty()) {
    return true;
  }
  std::error_code ec;
  bool dir_exists = fs::is_directory(parent_path, ec);

  if (ec == std::errc::no_such_file_or_directory) {
    return false;
  }

  if (!ec) {
    return dir_exists;
  }

  auto msg = ec.message();
  FMC_ERROR_REPORT(error, msg.c_str());
  return false;
}

int fmc_path_join(char *dest, size_t sz, const char *p1, const char *p2) {
#ifdef FMC_SYS_UNIX
  char sep = '/';
#elif defined(FMC_SYS_WIN)
  char sep = '\\';
#else
#error "Not supported"
#endif
  if (p1[0] == '\0') {
    return snprintf(dest, sz, "%s", p2);
  }
  return snprintf(dest, sz, "%s%c%s", p1, sep, p2);
}

int fmc_path_parent(char *dest, size_t sz, const char *src) {
  auto path = fs::path(src);
  auto parent_path = path.parent_path();
  if (parent_path.empty()) {
    return -1;
  }
  return snprintf(dest, sz, "%s", parent_path.c_str());
}

int fmc_exec_path_get(char *dest, size_t sz) {
#if defined(FMC_SYS_LINUX)
  char buff[PATH_MAX];
  ssize_t pathsz = readlink("/proc/self/exe", buff, PATH_MAX);
  return snprintf(dest, sz, "%.*s", static_cast<int>(pathsz), buf);
#elif defined(FMC_SYS_MACH)
  uint32_t bufsz = sz;
  if (_NSGetExecutablePath(dest, &bufsz) == 0 && bufsz < sz) {
    dest[bufsz] = '\0';
  }
  return bufsz + 1;
#else
#error "operating system is not supported"
#endif
}

FILE *fmc_popen(const char *command, const char *read_mode,
                fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_WIN)
  auto *fp = _popen(command, read_mode);
#else
  auto *fp = popen(command, read_mode);
#endif
  if (fp == NULL) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
  return fp;
}

int fmc_pclose(FILE *pipe, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_WIN)
  auto ret = _pclose(pipe);
#else
  auto ret = pclose(pipe);
#endif
  if (ret == -1) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }

  if (WEXITSTATUS(ret) != 0) {
    return WEXITSTATUS(ret);
  }

  return ret;
}

fmc_fd fmc_ftemp(fmc_error_t **error) {
#if defined(FMC_SYS_WIN)
  char filename[] =
      "long_dummy_string_with_maximum_temporary_file_path_length_for_windows";
#else
  char filename[] = "/tmp/FMC_XXXXXX";
#endif
  return fmc_ftemp_2(filename, error);
}

fmc_fd fmc_ftemp_2(char *file_path, fmc_error_t **error) {
  fmc_error_clear(error);
  if (file_path == NULL) {
    FMC_ERROR_REPORT(error, "file_path is NULL");
    return -1;
  }
  size_t file_path_max_length = strnlen(file_path, FMC_MAX_PATH);
  // max temp file path length should be *less* than FMC_MAX_PATH
  if (file_path_max_length >= FMC_MAX_PATH) {
    FMC_ERROR_REPORT(error,
                     "file_path length is greater or equal than FMC_MAX_PATH");
    return -1;
  }

  size_t file_path_buffer_size = file_path_max_length + 1;

#if defined(FMC_SYS_WIN)
  char tmppath[file_path_buffer_size];
  char filename[file_path_buffer_size];
  GetTempPath(file_path_buffer_size, tmppath);
  GetTempFileName(tmppath, "FMC_", 0, filename);
  auto fd =
      CreateFile(filename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_DELETE,
                 NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
#else // FMC_SYS_UNIX
  char filename[file_path_buffer_size];
  strcpy(filename, file_path);
  // Implicit O_RDWR | O_CREAT | O_EXCL. File is opened
  int fd = mkstemp(filename);
  if (fd < 0) {
    FMC_ERROR_REPORT(error, "mkstemp failed");
    return -1;
  }
  // Call unlink so that whenever the file is closed or the program exits
  // the temporary file is deleted
  unlink(filename);
  if (fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0) {
    FMC_ERROR_REPORT(error, "fchmod failed");
    close(fd);
    return -1;
  }
#endif
  if (!fmc_fvalid(fd)) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  } else {
    strcpy(file_path, filename);
  }

  return fd;
}

bool fmc_fexists(const char *file_path, fmc_error_t **error) {
  fmc_error_clear(error);

  std::error_code ec;
  bool exists = fs::exists(file_path, ec);

  if (ec) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
    return false;
  }

  return exists;
}

fmc_fd fmc_fd_get(FILE *file, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_WIN)
  auto fd = (HANDLE)_get_osfhandle(_fileno(file));
#else
  auto fd = fileno(file);
#endif
  if (!fmc_fvalid(fd)) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
  return fd;
}

fmc_fd fmc_fopen(const char *path, fmc_fmode flags, fmc_error_t **error) {
  fmc_error_clear(error);
  int flag = 0;
  if ((flags & fmc_fmode::READ) != 0) {
#if defined(FMC_SYS_WIN)
    flag |= GENERIC_READ;
#else
    flag |= O_RDONLY;
#endif
  }
  if ((flags & fmc_fmode::WRITE) != 0) {
#if defined(FMC_SYS_WIN)
    flag |= GENERIC_WRITE;
#else
    flag |= O_WRONLY | O_CREAT;
#endif
  }

  if ((flags & fmc_fmode::READWRITE) == fmc_fmode::READWRITE) {
#if defined(FMC_SYS_WIN)
#else
    flag = O_RDWR | O_CREAT;
#endif
  }
#if defined(FMC_SYS_WIN)
  auto fd = CreateFile(path, flag, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                       OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
  auto fd = open(path, flag, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif
  if (!fmc_fvalid(fd)) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
  return fd;
}

ssize_t fmc_fread(fmc_fd fd, void *buf, size_t bytes, fmc_error_t **error) {
#if defined(FMC_SYS_UNIX)
  auto ret = read(fd, buf, bytes);
  if (ret == -1) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
  return ret;
#else
#error "Not supported"
#endif
}

void fmc_fclose(fmc_fd fd, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_WIN)
  bool success = CloseHandle(fd) == TRUE;
#else
  bool success = close(fd) == 0;
#endif
  if (!success) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
}

bool fmc_fvalid(fmc_fd fd) {
#if defined(FMC_SYS_WIN)
  return fd != INVALID_HANDLE_VALUE;
#else
  return fd != -1;
#endif
}

bool fmc_freadonly(fmc_fd fd) {
  return (fcntl(fd, F_GETFL) & O_ACCMODE) == O_RDONLY;
}

size_t fmc_fsize(fmc_fd fd, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_WIN)
  auto file_sz = GetFileSize(fd, NULL);
  if (file_sz == INVALID_FILE_SIZE) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
    return -1;
  }
  return file_sz;
#else
  auto offset = lseek(fd, 0, SEEK_END);
  if (offset == (int64_t)-1) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
  return offset;
#endif
}

void fmc_fresize(fmc_fd fd, size_t new_size, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_WIN)
  LARGE_INTEGER liSize;
  liSize.QuadPart = new_size;
  if (!SetFilePointerEx(fd, liSize, NULL, FILE_BEGIN)) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
  if (!SetEndOfFile(fd)) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
  if (!SetFileValidData(fd, new_size)) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
  if (GetFileSize(fd, NULL) == INVALID_FILE_SIZE) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
#else
  auto success = ftruncate(fd, new_size) == 0;
  if (!success) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
#endif
}

void fmc_falloc(fmc_fd fd, long long sz, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_LINUX)
  auto err = posix_fallocate(fd, 0, sz);
  if (err != 0) {
    FMC_ERROR_REPORT(error, strerror(err));
  }
#elif defined(FMC_SYS_WIN)
  bool result = true;
  LARGE_INTEGER liSize;
  liSize.QuadPart = sz;
  LARGE_INTEGER lioSize;
  if (!GetFileSizeEx(fd, &lioSize)) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
    return;
  }
  LARGE_INTEGER file_pointer;
  LARGE_INTEGER zero;
  zero.QuadPart = 0;
  if (!SetFilePointerEx(fd, zero, &file_pointer, FILE_CURRENT)) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
    return;
  }
  if (liSize.QuadPart > lioSize.QuadPart) {
    // move to pre-last position and write one symbol to increase file size
    LARGE_INTEGER liSize2 = liSize;
    liSize2.QuadPart--;
    if (!SetFilePointerEx(fd, liSize2, NULL, FILE_END)) {
      FMC_ERROR_REPORT(error, fmc_syserror_msg());
      result = false;
      goto done;
    }
    DWORD nBytes;
    if (!WriteFile(fd, "\0", 1, &nBytes, NULL)) {
      FMC_ERROR_REPORT(error, fmc_syserror_msg());
      result = false;
      goto done;
    }
  }
done:
  bool success = SetFilePointerEx(fd, file_pointer, NULL, FILE_BEGIN);
  if (!success && result) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
#elif defined(FMC_SYS_MACH)
  fstore_t store = {F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, sz};
  // Try to get a continous chunk of disk space
  int ret = fcntl(fd, F_PREALLOCATE, &store);
  if (-1 == ret) {
    // OK, perhaps we are too fragmented, allocate non-continuous
    store.fst_flags = F_ALLOCATEALL;
    ret = fcntl(fd, F_PREALLOCATE, &store);
    if (-1 == ret) {
      FMC_ERROR_REPORT(error, fmc_syserror_msg());
      return;
    }
  }
  struct stat sb;
  if (fstat(fd, &sb) == 0) {
    if (sb.st_size >= sz)
      return;
    bool success = ftruncate(fd, sz) == 0;
    if (!success) {
      FMC_ERROR_REPORT(error, fmc_syserror_msg());
      return;
    }
    return;
  }
  FMC_ERROR_REPORT(error, fmc_syserror_msg());
#elif defined(FMC_SYS_UNIX)
  // The following is copied from fcntlSizeHint in sqlite
  /* If the OS does not have posix_fallocate(), fake it. First use
  ** ftruncate() to set the file size, then write a single byte to
  ** the last byte in each block within the extended region. This
  ** is the same technique used by glibc to implement posix_fallocate()
  ** on systems that do not have a real fallocate() system call.
  */
  struct stat buf;
  int fd = PR_FileDesc2NativeHandle(aFD);
  if (fstat(fd, &buf)) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
    return;
  }

  if (buf.st_size >= sz) {
    FMC_ERROR_REPORT(error, "File is larger");
    return;
  }

  const int nBlk = buf.st_blksize;

  if (!nBlk) {
    FMC_ERROR_REPORT(error, "st_blksize is zero");
    return;
  }

  if (ftruncate(fd, sz)) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
    return;
  }

  int nWrite; // Return value from write()
  PRInt64 iWrite = ((buf.st_size + 2 * nBlk - 1) / nBlk) * nBlk -
                   1; // Next offset to write to
  do {
    nWrite = 0;
    if (PR_Seek64(aFD, iWrite, PR_SEEK_SET) == iWrite)
      nWrite = PR_Write(aFD, "", 1);
    iWrite += nBlk;
  } while (nWrite == 1 && iWrite < sz);
  if (nWrite != 1) {
    FMC_ERROR_REPORT(error, "PR_Write failed");
  }
#else
#error "Not supported"
#endif
}

void fmc_fview_init(fmc_fview_t *view, size_t length, fmc_fd fd, int64_t offset,
                    fmc_error_t **error) {
  fmc_error_clear(error);
  bool readonly = fmc_freadonly(fd);

#if defined(FMC_SYS_WIN)
  LARGE_INTEGER lilength;
  lilength.QuadPart = length + offset;
  view->md = CreateFileMapping(
      fd,                                        // current file handle
      NULL,                                      // default security
      readonly ? PAGE_READONLY : PAGE_READWRITE, // read/write permission
      lilength.HighPart,                         // size of mapping object, high
      lilength.LowPart,                          // size of mapping object, low
      NULL);                                     // name of mapping object
                                                 // validate

  if (view->md == NULL) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
    return;
  }

  LARGE_INTEGER liofset;
  liofset.QuadPart = offset;

  view->mem = MapViewOfFile(view->md, // handle to
                                      // mapping object
                            readonly ? FILE_MAP_READ
                                     : FILE_MAP_ALL_ACCESS, // read/write
                            liofset.HighPart,               // high-order 32
                                                            // bits of file
                                                            // offset
                            liofset.LowPart,                // low-order 32
                                                            // bits of file
                                                            // offset
                            length);                        // number of bytes
                                                            // to map
  if (view->mem == NULL) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }

#elif defined(FMC_SYS_LINUX)
  int prot = !readonly ? PROT_READ | PROT_WRITE : PROT_READ;

  view->mem = mmap(NULL, length, prot, MAP_SHARED | MAP_POPULATE, fd, offset);
  if (view->mem == MAP_FAILED) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
#elif defined(FMC_SYS_MACH)
  int prot = !readonly ? PROT_READ | PROT_WRITE : PROT_READ;
  view->mem = mmap(NULL, length, prot, MAP_SHARED, fd, offset);
  if (view->mem == MAP_FAILED) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
#else
#error "operating system is not supported"
#endif
}

void fmc_fview_destroy(fmc_fview_t *view, size_t length, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_WIN)
  UnmapViewOfFile(view->mem);
  if (!CloseHandle(view->md)) {
    FMC_ERROR_REPORT(error, "CloseHandle failed on page");
  }
#else
  if (munmap(view->mem, length) != 0) {
    FMC_ERROR_REPORT(error, "munmap failed on page");
  }
#endif
}

void *fmc_fview_data(fmc_fview_t *view) { return view->mem; }

void fmc_fview_remap(fmc_fview_t *view, fmc_fd fd, size_t old_size,
                     size_t new_size, size_t offset, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_WIN)
  LARGE_INTEGER lilength;
  lilength.QuadPart = new_size + offset;
  view->md =
      CreateFileMapping(fd,                // current file handle
                        NULL,              // default security
                        PAGE_READWRITE,    // read/write permission
                        lilength.HighPart, // size of mapping object, high
                        lilength.LowPart,  // size of mapping object, low
                        NULL);             // name of mapping object
                                           // validate
  if (view->md == INVALID_HANDLE_VALUE) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
  LARGE_INTEGER liofset;
  liofset.QuadPart = offset;
  view->mem = MapViewOfFile(view->md, FILE_MAP_ALL_ACCESS, liofset.HighPart,
                            liofset.LowPart, new_size);
  if (view->mem == NULL) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
#elif defined(FMC_SYS_LINUX)
  view->mem = mremap(view->mem, old_size, new_size, MREMAP_MAYMOVE);
  if (view->mem == (void *)-1) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
#elif defined(FMC_SYS_MACH)
  if (auto *tmp =
          mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
      tmp != MAP_FAILED) {
    munmap(view->mem, old_size);
    view->mem = tmp;
  } else {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
#endif
}

void fmc_fview_sync(fmc_fview_t *view, size_t size, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_WIN)
  if (!FlushViewOfFile(view->mem, 0)) {
    FMC_ERROR_REPORT(error, "FlushViewOfFile failed on page");
  } // Async flush of dirty pages
  if (!FlushFileBuffers(view->md)) {
    FMC_ERROR_REPORT(error, "FlushFileBuffers failed on page");
  } // flush metadata and wait
#else
  if (msync(view->mem, size, MS_ASYNC) != 0) {
    FMC_ERROR_REPORT(error, "msync failed on page");
  }
#endif
}

void fmc_fflush() {
  fflush(NULL);
#ifdef FMC_SYS_UNIX
  sync();
#elif defined(FMC_SYS_WIN)
#else
#error "fflush is not defined";
#endif
}

ssize_t fmc_write(fmc_fd fd, const void *buf, size_t n) {
#ifdef FMC_SYS_UNIX
  return write(fd, buf, n);
#else
#error "write is not defined";
#endif
}
