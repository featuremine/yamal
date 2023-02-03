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
 * @file websocket.hpp
 * @date 16 Jun 2019
 * @brief Generic websocket components
 */

#pragma once

#include "fmc/memory.h"
#include "fmc/time.h"
#include <string_view>
#include "fmc++/error.hpp"
#include "fmc/endianness.h"

namespace fmc {

namespace websocket {

class frame {
public:
  frame() = delete;
  frame(const frame &);
  frame &operator=(const frame &);
  frame(frame &&) = delete;
  frame &operator=(frame &&) = delete;
  frame(struct fmc_pool *p, size_t sz = 0);
  frame(struct fmc_shmem data);
  ~frame();
  void set(const std::string_view &payload, int8_t optcode = 1);
  void set_optcode(int8_t optcode);
  std::string_view buffer() const;
  std::string_view payload() const;
  bool fin() const;
  void reserve(size_t);
  int8_t &operator[](size_t) const;
  const struct fmc_shmem &raw() const;
  void set_offset(size_t);

private:
  uint64_t sz_;
  uint64_t offset_;
  struct fmc_shmem data_;
  bool fin_;
}; // class frame

template<typename Network, typename Clbl>
void async_read_extended(Network &net, frame f, bool fin, int64_t receive_ns,
                         size_t payload_sz_sz, Clbl &&cb) {
  f.reserve(2 + payload_sz_sz);
  net.async_read_exactly(std::string_view((char*)&f[2], payload_sz_sz), payload_sz_sz,
      [&net, fin, receive_ns, payload_sz_sz, cb = std::forward<Clbl>(cb),
       f](const auto &ec, std::size_t bytes_transferred) mutable {
        if (ec) {
          cb(receive_ns, f, ec ? fmc::error(ec.message()) : fmc::error());
          return;
        }
        // Get rid of mask and get payload size
        size_t payload_sz = payload_sz_sz == 2
                                ? fmc_be16toh(*(uint16_t *)&f[2])
                                : fmc_be64toh(*(uint64_t *)&f[2]);

        async_read_mask_and_payload(net, f, fin, receive_ns, payload_sz_sz,
                                    payload_sz, std::forward<Clbl>(cb));
      });
}

template<typename Network, typename Clbl>
void async_read_mask_and_payload(Network &net, frame f, bool fin, int64_t receive_ns,
                                 size_t offset, size_t payload_sz,
                                 Clbl &&cb) {
  size_t mask_sz = f[1] & 0x80 ? 4 : 0;
  f.reserve(2 + offset + mask_sz + payload_sz);
  net.async_read_exactly(std::string_view((char *)&f[2 + offset], mask_sz + payload_sz), mask_sz + payload_sz,
      [payload_sz, receive_ns, offset, mask_sz, cb,
       f](const auto &ec, std::size_t bytes_transferred) mutable {
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
        cb(receive_ns, f, ec ? fmc::error(ec.message()) : fmc::error());
      });
}

template<typename Network, typename Pool, typename Clbl>
void async_read_ws_frame(Network &net, Pool *pool, Clbl &&cb) {
  frame f(pool, 2);
  net.async_read_exactly(f.buffer(), 2,
    [&net, pool, cb = std::forward<Clbl>(cb), f](const auto &ec, std::size_t bytes_transferred) mutable {
        auto receive_ns = fmc_cur_time_ns();
        if (ec) {
          cb(receive_ns, f, fmc::error(ec.message()));
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
              async_read_mask_and_payload(net, f, fin, receive_ns, 0, payload_sz,
                                          cb);
            } else if (payload_sz == 0x7E) {
              f.set_offset(2);
              async_read_extended(net, f, fin, receive_ns, 2, cb);
            } else if (payload_sz == 0x7F) {
              f.set_offset(8);
              async_read_extended(net, f, fin, receive_ns, 8, cb);
            }
          }
          break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
          /*reserved non control frames*/
          cb(receive_ns, f, fmc::error("Operation not supported"));
          break;
        case 8:
          /*connection close frames*/
          cb(receive_ns, f, fmc::error("Shut down"));
          break;
        case 9:
          /*ping frames*/
          {
            auto send_pong = [f, cb = std::forward<Clbl>(cb), &net, pool](int64_t r, const websocket::frame &newf,
                                     const fmc::error &ec) mutable {
              if (ec) {
                cb(r, f, ec);
                return;
              }
              f.set_optcode(0xA);
              net.async_write(
                  f.buffer(),
                  [&net, r, f, cb = std::forward<Clbl>(cb), pool](const auto &ec, size_t) mutable {
                    if (ec) {
                      cb(r, f, fmc::error(ec.message()));
                      return;
                    }
                    async_read_ws_frame(net, pool, std::forward<Clbl>(cb));
                  });
            };
            size_t payload_sz = f[1] & 0x7F;
            if (payload_sz < 0x7E) {
              f.set_offset(0);
              async_read_mask_and_payload(net, f, fin, receive_ns, 0, payload_sz,
                                          send_pong);
            } else if (payload_sz == 0x7E) {
              f.set_offset(2);
              async_read_extended(net, f, fin, receive_ns, 2, send_pong);
            } else if (payload_sz == 0x7F) {
              f.set_offset(8);
              async_read_extended(net, f, fin, receive_ns, 8, send_pong);
            }
          }
          break;
        case 10:
          /*pong frames*/
          async_read_ws_frame(net, pool, std::forward<Clbl>(cb));
          break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
          /*reserved control frames*/
          cb(receive_ns, f, fmc::error("Operation not supported"));
          break;
        default:
          cb(receive_ns, f, fmc::error("Operation not supported"));
          break;
        }
      });
}

template<typename Network, typename Buffer, typename Pool, typename Clbl>
void async_write_ws_frame(Network &net, Buffer &buffer, Pool *pool, Clbl &&cb) {
  frame send(pool, buffer.size());
  send.set(buffer);
  net.async_write(send.buffer(), [send, cb = std::forward<Clbl>(cb)] (const auto &ec, size_t bytes_transferred) mutable
      {
        cb(ec ? fmc::error(ec.message()) : fmc::error());
      });
}

} // namespace websocket

} // namespace fmc