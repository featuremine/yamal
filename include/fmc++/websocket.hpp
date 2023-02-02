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

template<typename ErrorCode>
void async_read_extended(frame f, bool fin, int64_t receive_ns,
                         size_t payload_sz_sz, std::function<void(int64_t, frame, ErrorCode)> cb) {}

template<typename ErrorCode>
void async_read_mask_and_payload(frame f, bool fin, int64_t receive_ns,
                                 size_t offset, size_t payload_sz,
                                 std::function<void(int64_t, frame, ErrorCode)> cb) {}

template<typename Network, typename Buffer, typename Pool, typename ErrorCode>
void async_read_ws_frame(Network &net, Buffer &buffer, Pool *pool, std::function<void(int64_t, frame, ErrorCode)> cb) {
  frame f(pool, 2);
  net.async_read_exactly(f.buffer(), 2,
    [cb, f](const ErrorCode &ec, std::size_t bytes_transferred) mutable {
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
          cb(receive_ns, f, ErrorCode(EOPNOTSUPP));
          break;
        case 8:
          /*connection close frames*/
          cb(receive_ns, f, ErrorCode(ESHUTDOWN));
          break;
        case 9:
          /*ping frames*/
          {
            auto send_pong = [&, cb](int64_t r, const websocket::frame &f,
                                     const ErrorCode &ec) {
              if (ec) {
                cb(r, f, ec);
                return;
              }
              async_write(
                  f.payload(),
                  [r, f, cb](const ErrorCode &ec) {
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
          cb(receive_ns, f, ErrorCode(EOPNOTSUPP));
          break;
        default:
          cb(receive_ns, f, ErrorCode(EOPNOTSUPP));
          break;
        }
      });
}

template<typename Network, typename Buffer, typename Pool, typename Clbl, typename ErrorCode>
void async_write_ws_frame(Network &net, Buffer &buffer, Pool *pool, Clbl &&cb) {
  frame send(pool, buffer.size());
  send.set(buffer);
  net.async_write(send.buffer(), [send, cb = std::forward<Clbl>(cb)] (const ErrorCode &ec, size_t bytes_transferred)
      {
        cb(ec);
      });
}

} // namespace websocket

} // namespace fmc