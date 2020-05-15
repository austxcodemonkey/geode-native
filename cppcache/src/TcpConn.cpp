/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TcpConn.hpp"

#include <iomanip>
#include <iostream>

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include "util/Log.hpp"

namespace {
template <int Level, int Name>
class timeval {
 public:
  // This is not an instance of the template, but of the system provided type
  // to be written to the socket API.
#if defined(_WINDOWS)
  using value_type = DWORD;
#else
  using value_type = ::timeval;
#endif

 private:
  value_type value_{};

 public:
  timeval() {}

  explicit timeval(value_type v) : value_(v) {}

  timeval &operator=(value_type v) {
    value_ = v;
    return *this;
  }

  value_type value() const { return value_; }

  template <typename Protocol>
  int level(const Protocol &) const {
    return Level;
  }

  template <typename Protocol>
  int name(const Protocol &) const {
    return Name;
  }

  template <typename Protocol>
  value_type *data(const Protocol &) {
    return &value_;
  }

  template <typename Protocol>
  const value_type *data(const Protocol &) const {
    return &value_;
  }

  template <typename Protocol>
  std::size_t size(const Protocol &) const {
    return sizeof(value_);
  }

  template <typename Protocol>
  void resize(const Protocol &, std::size_t s) {
    if (s != sizeof(value_))
      throw std::length_error("timeval socket option resize");
  }
};

// Asio doesn't support these socket options directly, but every major platform
// does. Timeout on IO socket operations are supported by the platform directly.
// This means We can all receive without needing to use the timeout interface -
// and more importantly, we can send while holding to per-operation time
// constraints and without blocking indefinitely.
//
// The default timeout is infinite, or by setting the socket option to null,
// which I won't provide - just don't construct a TcpConn with send and
// receieve timeouts.
typedef timeval<SOL_SOCKET, SO_SNDTIMEO> send_timeout;
typedef timeval<SOL_SOCKET, SO_RCVTIMEO> receive_timeout;
}  // namespace

namespace apache {
namespace geode {
namespace client {
TcpConn::TcpConn(const std::string ipaddr,
                 std::chrono::microseconds connect_timeout,
                 int32_t maxBuffSizePool)
    : TcpConn{
          ipaddr.substr(0, ipaddr.find(':')),
          static_cast<uint16_t>(std::stoi(ipaddr.substr(ipaddr.find(':') + 1))),
          connect_timeout, maxBuffSizePool} {}

TcpConn::TcpConn(const std::string host, uint16_t port,
                 std::chrono::microseconds /*connect_timeout*/,
                 int32_t maxBuffSizePool)
    : socket_{io_context_} {
  // We must connect first so we have a valid file descriptor to set options on.
  boost::asio::connect(socket_, boost::asio::ip::tcp::resolver(io_context_)
                                    .resolve(host, std::to_string(port)));

  std::stringstream ss;
  ss << "Connected " << socket_.local_endpoint() << " -> "
     << socket_.remote_endpoint();
  LOGINFO(ss.str());

  socket_.set_option(::boost::asio::ip::tcp::no_delay{true});
  socket_.set_option(
      ::boost::asio::socket_base::send_buffer_size{maxBuffSizePool});
  socket_.set_option(
      ::boost::asio::socket_base::receive_buffer_size{maxBuffSizePool});
}

TcpConn::TcpConn(const std::string ipaddr,
                 std::chrono::microseconds connect_timeout,
                 int32_t maxBuffSizePool, std::chrono::microseconds send_time,
                 std::chrono::microseconds receive_time)
    : TcpConn{ipaddr, connect_timeout, maxBuffSizePool} {
#if defined(_WINDOWS)
  socket_.set_option(::send_timeout{static_cast<DWORD>(send_time.count())});
  socket_.set_option(
      ::receive_timeout{static_cast<DWORD>(receive_time.count())});
#else

  auto send_seconds =
      std::chrono::duration_cast<std::chrono::seconds>(send_time);
  auto send_microseconds =
      send_time % std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::seconds{1});
  socket_.set_option(
      ::send_timeout{{static_cast<int>(send_seconds.count()),
                      static_cast<int>(send_microseconds.count())}});

  auto receive_seconds =
      std::chrono::duration_cast<std::chrono::seconds>(receive_time);
  auto receive_microseconds =
      receive_time % std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::seconds{1});
  socket_.set_option(
      ::receive_timeout{{static_cast<int>(receive_seconds.count()),
                         static_cast<int>(receive_microseconds.count())}});
#endif
}

TcpConn::~TcpConn() {
  std::stringstream ss;
  ss << "Disconnected " << socket_.local_endpoint() << " -> "
     << socket_.remote_endpoint();
  LOGFINE(ss.str());
}

size_t TcpConn::receive(char *buff, size_t len) {
  auto start = std::chrono::system_clock::now();

  return boost::asio::read(socket_, boost::asio::buffer(buff, len),
                           [len, start](boost::system::error_code &ec,
                                        const std::size_t n) -> std::size_t {
                             if (ec && ec != boost::asio::error::eof) {
                               // Quit if we encounter an error.
                               // Defer EOF to timeout.
                               return 0;
                             } else if (start + std::chrono::milliseconds(25) <=
                                        std::chrono::system_clock::now()) {
                               // Sometimes we don't know how much data to
                               // expect, so we're reading into an oversized
                               // buffer without knowing when to quit other than
                               // by timeout. Typically, if we timeout, we also
                               // have an EOF, meaning the connection is likely
                               // broken and will have to be closed. But if we
                               // have bytes, we may have just done a
                               // dumb/blind/hail mary receive, so defer broken
                               // connection handling until the next IO
                               // operation.
                               if (n) {
                                 // This prevents the timeout from being an
                                 // error condition.
                                 ec = boost::system::error_code{};
                               }
                               // But if n == 0 when we timeout, it's just a
                               // broken connection.

                               return 0;
                             }

                             return len - n;
                           });
}

size_t TcpConn::send(const char *buff, size_t len) {
  return boost::asio::write(socket_, boost::asio::buffer(buff, len));
}

//  Return the local port for this TCP connection.
uint16_t TcpConn::getPort() { return socket_.local_endpoint().port(); }

}  // namespace client
}  // namespace geode
}  // namespace apache
