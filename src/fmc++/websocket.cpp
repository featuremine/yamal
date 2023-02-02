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
 * @file websocket.cpp
 * @date 16 Jun 2019
 * @brief Generic websocket components implementation
 */

#include "fmc++/websocket.hpp"

#include "fmc/memory.h"
#include <fmc++/mpl.hpp>
#include <fmc/endianness.h>
#include <fmc/time.h>
#include <random>
#include <string_view>

using namespace fmc::websocket;

frame::frame(struct fmc_pool *p, size_t sz)
    : sz_(sz), offset_(0), data_{}, fin_(true) {
  fmc_error_t *e;
  fmc_shmem_init_alloc(&data_, p, sz, &e);
  fmc_runtime_error_unless(!e)
      << "unable to create frame. " << fmc_error_msg(e);
}

frame::frame(const frame &other) {
  sz_ = other.sz_;
  fin_ = other.fin_;
  offset_ = other.offset_;
  fmc_shmem_init_share(&data_, (struct fmc_shmem *)&other.data_);
}

frame::frame(struct fmc_shmem data) {
  fmc_shmem_init_share(&data_, &data);
  fin_ = (*this)[0] & 0x80;
  size_t payload_sz = (*this)[1] & 0x7F;
  if (payload_sz < 0x7E) {
    offset_ = 0;
  } else if (payload_sz == 0x7E) {
    offset_ = 2;
  } else if (payload_sz == 0x7F) {
    offset_ = 8;
  }
  sz_ = fmc_shmem_sz(&data);
}

frame &frame::operator=(const frame &other) {
  sz_ = other.sz_;
  fin_ = other.fin_;
  offset_ = other.offset_;
  fmc_shmem_init_share(&data_, (struct fmc_shmem *)&other.data_);
  return *this;
}

frame::~frame() {
  fmc_error_t *e;
  fmc_shmem_destroy(&data_, &e);
}

void frame::set(const std::string_view &payload, int8_t optcode) {
  uint64_t payload_sz = payload.size();
  fin_ = true;

  // compute base mask, reserve memory and copy payload size
  int8_t base_mask = int8_t(1 << 7);
  if (payload_sz <= 125) {
    base_mask |= (uint8_t)payload_sz;
    offset_ = 0;
    reserve(6 + payload_sz);
  } else if (payload_sz <= 65535) {
    base_mask |= 126U;
    uint16_t n_payload_sz = fmc_htobe16(payload_sz);
    offset_ = 2;
    reserve(6 + offset_ + payload_sz);
    memcpy(&(*this)[2], &n_payload_sz, sizeof(uint16_t));
  } else {
    base_mask |= 127U;
    uint64_t n_payload_sz = fmc_htobe64(payload_sz);
    offset_ = 8;
    reserve(6 + offset_ + payload_sz);
    memcpy(&(*this)[2], &n_payload_sz, sizeof(uint64_t));
  }

  // Set FIN to 1, rsv 0 and opt code to 1
  (*this)[0] = int8_t(1 << 7) | optcode;
  // Set base_mask
  (*this)[1] = base_mask;

  std::random_device rd;
  std::default_random_engine e(rd());
  uint32_t mask = e();
  memcpy(&(*this)[2 + offset_], &mask, sizeof(uint32_t));

  // Set payload
  memcpy(&(*this)[6 + offset_], payload.data(), payload_sz);

  // Store size
  sz_ = 6 + offset_ + payload_sz;

  // Mask data
  for (uint i = 0; i < payload_sz; ++i) {
    (*this)[6 + offset_ + i] =
        (*this)[6 + offset_ + i] ^ (*this)[2 + offset_ + (i % 4)];
  }
}

std::string_view frame::buffer() const {
  return std::string_view((char *)&(*this)[0], sz_);
}

std::string_view frame::payload() const {
  return std::string_view((char *)&(*this)[2 + offset_], sz_ - 2 - offset_);
}

bool frame::fin() const { return fin_; }

void frame::set_offset(size_t offset) { offset_ = offset; }

void frame::reserve(size_t sz) {
  fmc_error_t *e;
  fmc_shmem_realloc(&data_, sz, &e);
  sz_ = sz;
}

int8_t &frame::operator[](size_t sz) const {
  return ((int8_t *)*data_.view)[sz];
}

const struct fmc_shmem &frame::raw() const { return data_; }
