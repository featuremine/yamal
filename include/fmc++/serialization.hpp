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

/**
 * @file cmp/serialization.hpp
 * @authors Maxim Trokhimtchouk
 * @date 28 Oct 2018
 * @brief File contains C++ utilities for cmp reading and writing
 *
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include <cmp/cmp.h>
}

#include <fmc++/strings.hpp>
#include <fmc/files.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <string_view>
#include <vector>

struct cmp_mem_t {
  cmp_ctx_t ctx;
  size_t size = 0;
  size_t offset = 0;
  const void *buffer = nullptr;
};

inline bool cmp_mem_reader(cmp_ctx_t *ctx, void *data, size_t limit) {
  auto *cl = (cmp_mem_t *)ctx->buf;
  if (cl->offset + limit > cl->size) {
    return false;
  }
  memcpy(data, (char *)cl->buffer + cl->offset, limit);
  cl->offset = cl->offset + limit;
  return true;
}

inline bool cmp_mem_skipper(cmp_ctx_t *ctx, size_t count) {
  auto *cl = (cmp_mem_t *)ctx->buf;
  if (cl->offset + count > cl->size) {
    return false;
  }
  cl->offset = cl->offset + count;
  return true;
}

inline void cmp_mem_init(cmp_mem_t *cmp) {
  cmp_init(&cmp->ctx, cmp, cmp_mem_reader, cmp_mem_skipper, NULL);
}

inline void cmp_mem_set(cmp_mem_t *cmp, size_t size, const void *buf) {
  cmp->offset = 0;
  cmp->size = size;
  cmp->buffer = buf;
}

struct cmp_str_t {
  cmp_ctx_t ctx;
  std::string str;
};

inline size_t cmp_str_writer(cmp_ctx_t *ctx, const void *data, size_t count) {
  auto *cl = (cmp_str_t *)ctx->buf;
  auto offset = cl->str.size();
  if (offset + count > offset) {
    cl->str.resize(offset + count);
  }
  memcpy((char *)cl->str.data() + offset, data, count);
  return count;
}

inline void cmp_str_reset(cmp_str_t *cmp) { cmp->str.resize(0); }

inline size_t cmp_str_size(cmp_str_t *cmp) { return cmp->str.size(); }

inline void *cmp_str_data(cmp_str_t *cmp) { return (void *)cmp->str.data(); }

inline void cmp_str_init(cmp_str_t *cmp) {
  cmp_init(&cmp->ctx, cmp, NULL, NULL, cmp_str_writer);
}

inline bool cmp_file_read_bytes(void *data, size_t sz, FILE *fh) {
  return fread(data, sizeof(uint8_t), sz, fh) == (sz * sizeof(uint8_t));
}

inline bool cmp_file_skipper(cmp_ctx_t *ctx, size_t count) {
  static std::vector<char> buf;
  buf.resize(count);
  return cmp_file_read_bytes(buf.data(), count, (FILE *)ctx->buf);
}

inline bool cmp_file_reader(cmp_ctx_t *ctx, void *data, size_t limit) {
  return cmp_file_read_bytes(data, limit, (FILE *)ctx->buf);
}

struct cmp_file_t {
  cmp_ctx_t ctx;
  bool is_pipe;
};

inline bool cmp_file_init(cmp_file_t *cmp, const char *arg) {
  fmc_error_t *err;
  FILE *file = nullptr;
  auto pipe = fmc::ends_with_pipe(arg);
  if (pipe.first) {
    file = fmc_popen(pipe.second.c_str(), "r", &err);
    if (!file) {
      return false;
    }
    cmp->is_pipe = true;
  } else {
    file = fopen(arg, "r");
    if (!file) {
      return false;
    }
    cmp->is_pipe = false;
  }
  cmp_init(&cmp->ctx, file, cmp_file_reader, cmp_file_skipper, nullptr);
  return true;
}

inline bool cmp_file_close(cmp_file_t *cmp) {
  auto file = (FILE *)cmp->ctx.buf;
  if (file) {
    if (cmp->is_pipe) {
      fmc_error_t *err;
      fmc_pclose(file, &err);
    } else
      fclose(file);
    return true;
  }
  return false;
}

inline bool cmp_read_string(cmp_ctx_t *ctx, std::string &str) {
  uint32_t size = 0;
  if (!cmp_read_str_size(ctx, &size))
    return false;
  str.resize(size);
  return ctx->read(ctx, &str[0], size);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, int64_t *arg) {
  return cmp_read_long(ctx, arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, uint64_t *arg) {
  return cmp_read_ulong(ctx, arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, int32_t *arg) {
  return cmp_read_int(ctx, arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, uint32_t *arg) {
  return cmp_read_uint(ctx, arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, int8_t *arg) {
  return cmp_read_char(ctx, arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, char *arg) {
  return cmp_read_char(ctx, (int8_t *)arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, uint8_t *arg) {
  return cmp_read_uchar(ctx, arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, int16_t *arg) {
  return cmp_read_short(ctx, arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, uint16_t *arg) {
  return cmp_read_ushort(ctx, arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, bool *arg) {
  return cmp_read_bool(ctx, arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, double *arg) {
  return cmp_read_double(ctx, arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, std::string *arg) {
  return cmp_read_string(ctx, *arg);
}

inline bool cmp_read_item(cmp_ctx_t *ctx, fmc_decimal128_t *arg) {
  cmp_object_t obj;
  if (!cmp_read_object(ctx, &obj)) {
    return false;
  }
  if (cmp_object_is_str(&obj)) {
    char buf[FMC_DECIMAL128_STR_SIZE];
    uint32_t sz = 0;
    if (!cmp_object_as_str(&obj, &sz) || sz >= FMC_DECIMAL128_STR_SIZE) {
      return false;
    }
    if (!cmp_object_to_str(ctx, &obj, buf, FMC_DECIMAL128_STR_SIZE)) {
      return false;
    }
    fmc_decimal128_from_str(arg, buf);
    return true;
  }
  if (cmp_object_is_sinteger(&obj)) {
    int64_t num = 0;
    if (!cmp_object_as_sinteger(&obj, &num)) {
      return false;
    }
    fmc_decimal128_from_int(arg, num);
    return true;
  }
  if (cmp_object_is_uinteger(&obj)) {
    uint64_t num = 0;
    if (!cmp_object_as_uinteger(&obj, &num)) {
      return false;
    }
    fmc_decimal128_from_uint(arg, num);
    return true;
  }
  return false;
}

template <class... Args>
bool cmp_read_many(cmp_ctx_t *ctx, uint32_t *left, Args *...args) {
  auto read_one = [ctx, left](auto *arg) {
    bool ret = *left > 0 && cmp_read_item(ctx, arg);
    *left -= ret == true;
    return ret;
  };
  return (true && ... && read_one(args));
}

inline bool cmp_read_rest(cmp_ctx_t *ctx, uint32_t left) {
  for (uint32_t i = 0; i < left; ++i) {
    if (!cmp_skip_object(ctx, NULL)) {
      return false;
    }
  }
  return true;
}

inline bool cmp_write_item(cmp_ctx_t *ctx, int64_t arg) {
  return cmp_write_integer(ctx, arg);
}

inline bool cmp_write_item(cmp_ctx_t *ctx, uint64_t arg) {
  return cmp_write_uinteger(ctx, arg);
}

inline bool cmp_write_item(cmp_ctx_t *ctx, int32_t arg) {
  return cmp_write_int(ctx, arg);
}

inline bool cmp_write_item(cmp_ctx_t *ctx, uint32_t arg) {
  return cmp_write_uint(ctx, arg);
}

inline bool cmp_write_item(cmp_ctx_t *ctx, int8_t arg) {
  return cmp_write_integer(ctx, arg);
}

inline bool cmp_write_item(cmp_ctx_t *ctx, char arg) {
  return cmp_write_integer(ctx, (int8_t)arg);
}

inline bool cmp_write_item(cmp_ctx_t *ctx, uint8_t arg) {
  return cmp_write_uinteger(ctx, arg);
}

inline bool cmp_write_item(cmp_ctx_t *ctx, int16_t arg) {
  return cmp_write_integer(ctx, arg);
}

inline bool cmp_write_item(cmp_ctx_t *ctx, uint16_t arg) {
  return cmp_write_uinteger(ctx, arg);
}

inline bool cmp_write_item(cmp_ctx_t *ctx, bool arg) {
  return cmp_write_bool(ctx, arg);
}

inline bool cmp_write_item(cmp_ctx_t *ctx, double arg) {
  return cmp_write_double(ctx, arg);
}

inline bool cmp_write_item(cmp_ctx_t *ctx, std::string &arg) {
  return cmp_write_str(ctx, arg.data(), arg.size());
}

inline bool cmp_write_item(cmp_ctx_t *ctx, std::string_view &arg) {
  return cmp_write_str(ctx, arg.data(), arg.size());
}

template <class... Args>
bool cmp_write_many(cmp_ctx_t *ctx, uint32_t *left, Args &&...args) {
  auto write_one = [ctx, left](auto &&arg) {
    bool ret = *left > 0 && cmp_write_item(ctx, arg);
    *left -= ret == true;
    return ret;
  };
  return (true && ... && write_one(args));
}
