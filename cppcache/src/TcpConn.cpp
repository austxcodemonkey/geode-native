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
TcpConn::TcpConn(const std::string ipaddr,
                 std::chrono::microseconds connect_timeout,
                 int32_t maxBuffSizePool)
    : TcpConn{
          ipaddr.substr(0, ipaddr.find(':')),
          static_cast<uint16_t>(std::stoi(ipaddr.substr(ipaddr.find(':') + 1))),
          connect_timeout, maxBuffSizePool} {}

TcpConn::TcpConn(const std::string host, uint16_t port,
                 std::chrono::microseconds connect_timeout,
                 int32_t maxBuffSizePool) {
  boost::system::error_code error;

  // Connect first so we have a valid file descriptor to set options.
  boost::asio::async_connect(
      socket_,
      boost::asio::ip::tcp::resolver(io_context_)
          .resolve(host, std::to_string(port)),
      [&](const boost::system::error_code &result_error,
          const boost::asio::ip::tcp::endpoint &) { error = result_error; });

  run(connect_timeout);

  if (error) {
    throw GeodeIOException(error.message());
  }

  socket_.set_option(::boost::asio::ip::tcp::no_delay{true});
  socket_.set_option(
      ::boost::asio::socket_base::send_buffer_size{maxBuffSizePool});
  socket_.set_option(
      ::boost::asio::socket_base::receive_buffer_size{maxBuffSizePool});
}

TcpConn::~TcpConn() {
  io_context_.restart();
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
  socket_.close();
  io_context_.run();
}

void TcpConn::run(std::chrono::steady_clock::duration timeout) {
  io_context_.restart();
  io_context_.run_for(timeout);
}

size_t TcpConn::receive(char *buff, size_t len,
                        std::chrono::microseconds timeout) {
  boost::system::error_code error;
  std::size_t n = 0;

  boost::asio::async_read(
      socket_, boost::asio::buffer(buff, len),
      [&](const boost::system::error_code &result_error, std::size_t result_n) {
        error = result_error;
        n = result_n;
      });

  run(timeout);

  if (error == boost::asio::error::eof) {
  }  // Do nothing, this is normal.
  else if (error == boost::asio::error::try_again) {
    throw TimeoutException(error.message());
  } else if (error) {
    throw GeodeIOException(error.message());
  }

  return n;
}

size_t TcpConn::send(const char *buff, size_t len,
                     std::chrono::microseconds timeout) {
  boost::system::error_code error;
  std::size_t n = 0;

  boost::asio::async_write(
      socket_, boost::asio::buffer(buff, len),
      [&](const boost::system::error_code &result_error, std::size_t result_n) {
        error = result_error;
        n = result_n;
      });

  run(timeout);

  if (error == boost::asio::error::try_again) {
    throw TimeoutException(error.message());
  } else if (error) {
    throw GeodeIOException(error.message());
  }

  return n;
}

//  Return the local port for this TCP connection.
uint16_t TcpConn::getPort() { return socket_.local_endpoint().port(); }

}  // namespace client
}  // namespace geode
}  // namespace apache
