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

#include <iostream>

namespace apache {
namespace geode {
namespace client {
TcpConn::TcpConn(const char *ipaddr, std::chrono::microseconds wait,
                 int32_t maxBuffSizePool)
    : socket_rw_buffer_size_{maxBuffSizePool},
      port_{static_cast<uint16_t>(atoi(strchr(ipaddr, ':') + 1))},
      host_{ipaddr, static_cast<size_t>(strchr(ipaddr, ':') - ipaddr)},
      connect_timeout_{wait} {
  std::clog << "TcpConn(" << ipaddr << ", " << wait.count() << ", "
            << maxBuffSizePool << ")\n";
  std::clog << "host_ = \"" << host_ << "\", port_ = " << port_ << '\n';
}

TcpConn::TcpConn(const char *hostname, int32_t port,
                 std::chrono::microseconds wait, int32_t maxBuffSizePool)
    : socket_rw_buffer_size_{maxBuffSizePool},
      port_{static_cast<uint16_t>(port)},
      host_{hostname},
      connect_timeout_{wait} {
  std::clog << "TcpConn(" << hostname << ", " << port << ", " << wait.count()
            << ", " << maxBuffSizePool << ")\n";
}

TcpConn::~TcpConn() {
  std::clog << "~TcpConn()\n";
  close();
}

void TcpConn::connect() {
  std::clog << "TcpConn::connect()\n";

  auto endpoints = boost::asio::ip::tcp::resolver(io_context_)
                       .resolve(host_, std::to_string(port_));
  boost::system::error_code error;

  boost::asio::async_connect(
      socket_, endpoints,
      [&](const boost::system::error_code &result_error,
          const boost::asio::ip::tcp::endpoint &) { error = result_error; });

  run(connect_timeout_);

  if (error) {
    std::cerr << "TcpConn::connect: Error " << error << '\n';

    throw std::system_error(error);
  }
}

void TcpConn::close() {
  std::clog << "TcpConn::close()\n";

  socket_.close();
  io_context_.run();
}

void TcpConn::run(std::chrono::steady_clock::duration timeout) {
  std::clog << "TcpConn::run(" << timeout.count() << ")\n";
  io_context_.restart();
  io_context_.run_for(timeout);

  if (!io_context_.stopped()) {
    std::clog << "TcpConn::run: context stopped\n";
    close();
  }
}

void TcpConn::init() {
  std::clog << "TcpConn::init()\n";
  connect();

  std::clog << "socket_.set_option(::boost::asio::ip::tcp::no_delay{true});\n";
  socket_.set_option(::boost::asio::ip::tcp::no_delay{true});
  std::clog << "socket_.set_option(::boost::asio::socket_base::send_buffer_"
               "size{socket_rw_buffer_size_});\n";
  socket_.set_option(
      ::boost::asio::socket_base::send_buffer_size{socket_rw_buffer_size_});
  std::clog << "socket_.set_option(::boost::asio::socket_base::receive_buffer_"
               "size{socket_rw_buffer_size_});\n";
  socket_.set_option(
      ::boost::asio::socket_base::receive_buffer_size{socket_rw_buffer_size_});
}

size_t TcpConn::receive(char *buff, size_t len,
                        std::chrono::microseconds wait) {
  std::clog << "TcpConn::receive(" << static_cast<void *>(buff) << ", " << len
            << ", " << wait.count() << ")\n";
  boost::system::error_code error;
  std::size_t n = 0;

  boost::asio::async_read(
      socket_, boost::asio::buffer(buff, len),
      [&](const boost::system::error_code &result_error, std::size_t result_n) {
        error = result_error;
        n = result_n;
      });

  run(wait);

  if (error && boost::asio::error::eof != error) {
    std::clog << "TcpConn::receive: " << n << " bytes\n";
    std::cerr << "TcpConn::receive: Error " << error << '\n';
    throw std::system_error(error);
  }

  std::clog << "TcpConn::receive: " << n << " bytes\n";
  return n;
}

size_t TcpConn::send(const char *buff, size_t len,
                     std::chrono::microseconds wait) {
  std::clog << "TcpConn::send(" << static_cast<const void *>(buff) << ", "
            << len << ", " << wait.count() << ")\n";
  boost::system::error_code error;
  std::size_t n = 0;

  boost::asio::async_write(
      socket_, boost::asio::buffer(buff, len),
      [&](const boost::system::error_code &result_error, std::size_t result_n) {
        error = result_error;
        n = result_n;
      });

  run(wait);

  if (error && boost::asio::error::eof != error) {
    std::clog << "TcpConn::send: " << n << " bytes\n";
    std::cerr << "TcpConn::send: Error " << error << '\n';
    throw std::system_error(error);
  }

  std::clog << "TcpConn::send: " << n << " bytes\n";
  return n;
}

//  Return the local port for this TCP connection.
uint16_t TcpConn::getPort() {
  std::clog << "TcpConn::getPort() -> " << socket_.local_endpoint().port()
            << '\n';
  return socket_.local_endpoint().port();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
