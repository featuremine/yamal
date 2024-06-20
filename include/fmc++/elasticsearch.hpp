/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file websocket.hpp
 * @date 16 Jun 2019
 * @brief Generic websocket components
 */

#pragma once

#include <fmc++/mpl.hpp>
#include <fmc++/strings.hpp>
#include <fmc/platform.h>
#include <fmc/time.h>

#include <arpa/inet.h>
#include <errno.h> //For errno - the error number
#include <netdb.h> //hostent
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <string_view>


namespace fmc {

namespace elasticsearch {

inline bool starts_with(std::string_view a, std::string_view b) {
  if (a.size() < b.size())
    return false;
  for (auto i = 0; i < b.size(); ++i) {
    if (std::tolower(a[i]) != std::tolower(b[i]))
      return false;
  }
  return true;
}

class elasticsearch_client {
public:
  elasticsearch_client() = default;
  elasticsearch_client(std::string_view host, std::string_view port)
      : host_(host), port_(port) {}

  ~elasticsearch_client() { disconnect(); }

  void set_address(std::string_view host, std::string_view port) {
    host_ = host;
    port_ = port;
  }

  void disconnect() {
    request_.clear();
    response_.clear();
    if (sd != -1)
      close(sd);
  }

  void connect() {
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    /* Obtain address(es) matching host/port */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0; /* Any protocol */

    auto s = getaddrinfo(host_.c_str(), port_.c_str(), &hints, &result);
    fmc_runtime_error_unless(s == 0)
        << "error getting address :" << gai_strerror(s);

    for (rp = result; rp != NULL; rp = rp->ai_next) {
      sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sd == -1)
        continue;
      if (::connect(sd, rp->ai_addr, rp->ai_addrlen) != -1)
        break; /* Success */
      close(sd);
    }
    fmc_runtime_error_unless(rp) << "error: could not connect to " << host_;

    freeaddrinfo(result); /* No longer needed */
  }

  void set_timeout(int sec) {
    struct timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = 0;
    fmc_runtime_error_unless(
        setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) >= 0)
        << "error: could not set receive timeout for " << host_;
  }

  void bulk_update(std::string_view index, std::string_view doc,
                   std::string_view data) {
    /*
    { "index" : { "_index" : "test", "_id" : "1" } }
    { "field1" : "value1" }
    */
    bulk_.append("{\"index\":{\"_index\":\"");
    bulk_.append(index);
    bulk_.append("\",\"_id\":\"");
    bulk_.append(doc);
    bulk_.append("\"}}\n");
    bulk_.append(data);
    bulk_.append("\n");
  }

  std::pair<int, std::string_view> bulk_send() {
    using namespace std;
    char buf[32];
    request_.clear();
    request_.append("POST /_bulk HTTP/1.1\r\nHost: ");
    request_.append(host_);
    request_.append("\r\nContent-type: application/json\r\nContent-length: ");
    request_.append(fmc::to_string_view_unsigned(buf, bulk_.size()));
    request_.append("\r\n\r\n");

    if (send(sd, request_.data(), request_.size(), 0) < 0)
      return {-1, std::string_view()};
    if (send(sd, bulk_.data(), bulk_.size(), 0) < 0)
      return {-1, std::string_view()};

    cout << "bulk data size is " << bulk_.size() << endl;

    auto res = read_response();

    if (res.first / 100 == 2) {
      if (res.second.find("\"errors\":true") != std::string_view::npos) {
        res.first = 400;
      } else {
        bulk_.clear();
      }
    }

    return res;
  }

  std::pair<int, std::string_view>
  update(std::string_view index, std::string_view doc, std::string_view data, std::string_view method = "PUT") {
    char buf[32];
    request_.clear();
    request_.append(method);
    request_.append(" /");
    request_.append(index);
    if (!doc.empty()) {
      request_.append("/_doc/");
      request_.append(doc);
    }
    request_.append(" HTTP/1.1\r\n");
    request_.append("Host: ");
    request_.append(host_);
    request_.append("\r\n");
    request_.append("Content-type: application/json\n");
    request_.append("Content-length: ");
    request_.append(fmc::to_string_view_unsigned(buf, data.size()));
    request_.append("\r\n\r\n");
    request_.append(data);

    if (send(sd, request_.data(), request_.size(), 0) < 0)
      return {-1, std::string_view()};

    return read_response();
  }

  void bulk_send_with_retries() {
    using namespace std;
    constexpr int wait = 10;
    constexpr int max_attempts = 5;
    int attempts = max_attempts;
    do {
      auto res = bulk_send();
      if (res.first / 100 == 2)
        break;
      fmc_runtime_error_unless(res.first < 0 || res.first == 429)
          << "failed request " << res.first
          << " with response: " << res.second.substr(0, 1024);
      cerr << "error receiving response: " << strerror(errno) << endl;
      disconnect();
      cerr << "disconnected." << endl;
      for (; attempts > 0; --attempts) {
        try {
          cerr << "attempting to reconnect in " << wait << " seconds." << endl
               << attempts << " attempts left" << endl;
          this_thread::sleep_for(chrono::seconds(wait));
          connect();
          cerr << "connected." << endl;
          break;
        } catch (std::exception &e) {
          cerr << e.what() << endl;
        }
      }
    } while (attempts > 0);
    fmc_runtime_error_unless(attempts)
        << "shutting down after " << max_attempts << " unsuccessful attempts";
  }

  void write_with_retries(std::string_view index, std::string_view doc,
                          std::string_view data) {
    using namespace std;
    int attempts = 5;
    constexpr int wait = 1;
    do {
      auto res = update(index, doc, data);
      if (res.first / 100 == 2)
        break;
      if (res.first < 0)
        cerr << "error receiving response: " << strerror(errno) << endl;
      else
        cerr << "error received bad response: " << res.second << endl;
      disconnect();
      cerr << "disconnected." << endl;
      for (; attempts > 0; --attempts) {
        try {
          cerr << "attempting to reconnect in " << wait << " seconds." << endl;
          this_thread::sleep_for(chrono::seconds(1));
          connect();
          cerr << "connected." << endl;
          break;
        } catch (std::exception &e) {
          cerr << e.what() << endl;
        }
      }
    } while (attempts > 0);
    fmc_runtime_error_unless(attempts)
        << "shutting down after unsuccessful attempts";
  }

private:
  std::pair<int, std::string_view> read_response() {
    using namespace std;
    constexpr size_t BUF_SIZE = 8192;
    constexpr std::string_view http{"HTTP"};
    constexpr std::string_view keyword{"content-length: "};

    int64_t sz = 0;
    response_.erase(0, cur);
    cur = 0;

    auto read_line = [&]() {
      do {
        std::string_view rest = std::string_view(response_).substr(cur);
        auto pos = rest.find_first_of('\n');
        if (pos != std::string::npos)
          return rest.substr(0, pos + 1);
        auto len = response_.size();
        response_.resize(len + BUF_SIZE);
        auto *buf = &response_[len];
        sz = read(sd, buf, BUF_SIZE);
        if (sz <= 0)
          return std::string_view{};
        response_.resize(len + sz);
      } while (true);
    };

    std::string_view line;
    line = read_line();
    cur += line.size();

    if (sz < 0)
      return {-1, std::string_view{}};

    fmc_runtime_error_unless(starts_with(line, http))
        << "unexpected response: " << response_;

    int res = fmc::from_string_view<int>(line.substr(9)).first;

    uint64_t left = 0;
    for (; line != "\r\n"sv; line = read_line(), cur += line.size()) {
      if (sz < 0)
        return {-1, std::string_view{}};
      if (starts_with(line, keyword))
        left =
            fmc::from_string_view<uint64_t>(line.substr(keyword.size())).first;
    }

    while (response_.size() < left + cur) {
      auto len = response_.size();
      response_.resize(left + cur);
      auto *buf = &response_[len];
      sz = read(sd, buf, response_.size() - len);
      if (sz < 0)
        return {-1, std::string_view{}};
      response_.resize(len + sz);
    }

    auto view = std::string_view(response_).substr(0, cur + left);
    cur = view.size();

    return {res, view};
  }

  int sd = -1;
  uint64_t cur = 0;
  std::string request_;
  std::string bulk_;
  std::string response_;
  std::string host_;
  std::string port_;
};

} // namespace elasticsearch

} // namespace fmc
