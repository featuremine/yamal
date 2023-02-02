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
#include "asio.hpp"
#include "asio/ssl.hpp"

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

asio::mutable_buffer frame::buffer() const {
  return asio::buffer(&(*this)[0], sz_);
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

client::client(struct fmc_pool *p, asio::io_service &io)
    : pool_(p), io_(io), ctx_(setup_ctx()), socket_(io_, ctx_),
      frame_send_(pool_) {}

asio::ssl::context client::setup_ctx() {
  auto ctx = asio::ssl::context(asio::ssl::context::tlsv12_client);
  ctx.set_options(asio::ssl::context::default_workarounds |
                  asio::ssl::context::no_sslv2 | asio::ssl::context::no_sslv3 |
                  asio::ssl::context::single_dh_use);
  return ctx;
}

void client::async_connect(asio::ip::tcp::resolver::iterator endpoint_iterator,
                           std::string path, connect_clbck_t cb) {
  asio::async_connect(
      socket_.lowest_layer(), endpoint_iterator,
      [path, cb, this](asio::error_code ec,
                       asio::ip::tcp::resolver::iterator res_it) {
        if (ec) {
          cb(ec);
          return;
        }
        if (!SSL_set_tlsext_host_name(socket_.native_handle(),
                                      res_it->host_name().c_str())) {
          cb(asio::error::connection_refused);
          return;
        }
        socket_.async_handshake(
            asio::ssl::stream_base::client,
            [res_it, path, cb, this](asio::error_code ec) {
              if (ec) {
                cb(ec);
                return;
              }
              std::random_device rd;
              std::default_random_engine re(rd());
              std::independent_bits_engine<std::default_random_engine, CHAR_BIT,
                                           uint8_t>
                  rbe(re);
              std::vector<uint8_t> data(16);
              std::generate(begin(data), end(data), std::ref(rbe));

              const unsigned int pl = 4 * ((data.size() + 2) / 3);
              std::string base64_encode(pl, 0);
              const unsigned int ol =
                  EVP_EncodeBlock((unsigned char *)base64_encode.data(),
                                  data.data(), data.size());
              if (pl != ol) {
                cb(asio::error::connection_aborted);
                return;
              }

              request_ = "GET /" + path +
                         " HTTP/1.1\r\n"
                         "Upgrade: websocket\r\n"
                         "Host: " +
                         res_it->host_name() +
                         "\r\n"
                         "Sec-WebSocket-Key: " +
                         base64_encode +
                         "\r\n"
                         "Sec-WebSocket-Version: 13\r\n"
                         "Connection: Upgrade\r\n\r\n";
              asio::async_write(
                  socket_, asio::buffer(request_.data(), request_.size()),
                  asio::transfer_all(),
                  [cb, this](const asio::error_code &ec,
                             std::size_t bytes_transferred) {
                    if (ec) {
                      cb(ec);
                      return;
                    }

                    asio::async_read_until(
                        socket_, asio::dynamic_buffer(response_), "\r\n\r\n",
                        [cb, this](const asio::error_code &ec,
                                   std::size_t bytes_transferred) {
                          if (ec) {
                            cb(ec);
                            return;
                          }
                          if (bytes_transferred == 0) {
                            cb(asio::error::connection_refused);
                            return;
                          }
                          auto http_pos = response_.find("HTTP/1.1", 0);
                          std::string ret_code =
                              http_pos != std::string::npos
                                  ? response_.substr(http_pos + 9, 3)
                                  : "";
                          cb(ret_code == "101"
                                 ? ec
                                 : asio::error::connection_refused);
                          response_.clear();
                        });
                  });
            });
      });
}

void client::async_write(std::string_view msg, write_clbck_t cb,
                         int8_t optcode) {
  frame_send_.set(msg, optcode);
  asio::async_write(socket_, frame_send_.buffer(), asio::transfer_all(),
                    [cb](const asio::error_code &ec,
                         std::size_t bytes_transferred) { cb(ec); });
}

void client::async_read(read_clbck_t cb) {
  frame f(pool_, 2);
  asio::async_read(
      socket_, f.buffer(), asio::transfer_exactly(2),
      [cb, this, self = shared_from_this(),
       f](const asio::error_code &ec, std::size_t bytes_transferred) mutable {
        auto receive_ns = fmc_cur_time_ns();
        if (ec) {
          cb(receive_ns, f, ec);
          return;
        }

        bool fin = f[0] & 0x80;

        switch (f[0] & 0x0F) { // optcode
        case 0:
          /*continuation frame*/
        case 1:
          /*text frame*/
        case 2:
          /*binary frame*/
          {
            size_t payload_sz = f[1] & 0x7F;
            if (payload_sz < 0x7E) {
              f.set_offset(0);
              async_read_mask_and_payload(f, fin, receive_ns, 0, payload_sz,
                                          cb);
            } else if (payload_sz == 0x7E) {
              f.set_offset(2);
              async_read_extended(f, fin, receive_ns, 2, cb);
            } else if (payload_sz == 0x7F) {
              f.set_offset(8);
              async_read_extended(f, fin, receive_ns, 8, cb);
            }
          }
          break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
          /*reserved non control frames*/
          cb(receive_ns, f, asio::error::operation_not_supported);
          break;
        case 8:
          /*connection close frames*/
          cb(receive_ns, f, asio::error::shut_down);
          break;
        case 9:
          /*ping frames*/
          {
            auto send_pong = [&, cb](int64_t r, const websocket::frame &f,
                                     const asio::error_code &ec) {
              if (ec) {
                cb(r, f, ec);
                return;
              }
              async_write(
                  f.payload(),
                  [this, r, f, cb](const asio::error_code &ec) {
                    if (ec) {
                      cb(r, f, ec);
                      return;
                    }
                    async_read(cb);
                  },
                  0xA);
            };
            size_t payload_sz = f[1] & 0x7F;
            if (payload_sz < 0x7E) {
              f.set_offset(0);
              async_read_mask_and_payload(f, fin, receive_ns, 0, payload_sz,
                                          send_pong);
            } else if (payload_sz == 0x7E) {
              f.set_offset(2);
              async_read_extended(f, fin, receive_ns, 2, send_pong);
            } else if (payload_sz == 0x7F) {
              f.set_offset(8);
              async_read_extended(f, fin, receive_ns, 8, send_pong);
            }
          }
          break;
        case 10:
          /*pong frames*/
          async_read(cb);
          break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
          /*reserved control frames*/
          cb(receive_ns, f, asio::error::operation_not_supported);
          break;
        default:
          cb(receive_ns, f, asio::error::operation_not_supported);
          break;
        }
      });
}

void client::async_read_extended(frame f, bool fin, int64_t receive_ns,
                                 size_t payload_sz_sz, read_clbck_t cb) {
  f.reserve(2 + payload_sz_sz);
  asio::async_read(
      socket_, asio::buffer(&f[2], payload_sz_sz),
      asio::transfer_exactly(payload_sz_sz),
      [fin, receive_ns, payload_sz_sz, cb, this,
       f](const asio::error_code &ec, std::size_t bytes_transferred) mutable {
        if (ec) {
          cb(receive_ns, f, ec);
          return;
        }
        // Get rid of mask and get payload size
        size_t payload_sz = payload_sz_sz == 2
                                ? fmc_be16toh(*(uint16_t *)&f[2])
                                : fmc_be64toh(*(uint64_t *)&f[2]);

        async_read_mask_and_payload(f, fin, receive_ns, payload_sz_sz,
                                    payload_sz, cb);
      });
}

void client::async_read_mask_and_payload(frame f, bool fin, int64_t receive_ns,
                                         size_t offset, size_t payload_sz,
                                         read_clbck_t cb) {
  size_t mask_sz = f[1] & 0x80 ? 4 : 0;
  f.reserve(2 + offset + mask_sz + payload_sz);
  asio::async_read(
      socket_, asio::buffer(&f[2 + offset], mask_sz + payload_sz),
      asio::transfer_exactly(mask_sz + payload_sz),
      [payload_sz, receive_ns, offset, mask_sz, cb,
       f](const asio::error_code &ec, std::size_t bytes_transferred) mutable {
        if (!ec && mask_sz) {
          // Find where mask currently is. Data will start there.
          size_t start = 2 + offset;
          // Copy mask so we can override data
          int8_t mask[4];
          memcpy(&mask, &f[2 + offset], sizeof(mask));
          // Apply mask and move content
          for (uint i = 0; i < payload_sz; ++i) {
            const_cast<int8_t &>(f[start + i]) =
                f[start + mask_sz + i] ^ mask[i % 4];
          }
          // Invert the mask bit to signal no mask
          const_cast<int8_t &>(f[1]) &= ~(1 << 7);
          f.reserve(2 + offset + payload_sz);
        }
        cb(receive_ns, f, ec);
      });
}
