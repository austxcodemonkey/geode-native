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

#pragma once

#ifndef GEODE_TCPCONN_H_
#define GEODE_TCPCONN_H_

#include <boost/asio.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <geode/internal/geode_globals.hpp>

#include "Connector.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {
class APACHE_GEODE_EXPORT TcpConn : public Connector {
  boost::asio::io_context io_context_;
  boost::asio::ip::tcp::socket socket_{io_context_};

  void run(std::chrono::steady_clock::duration timeout);

  const int32_t socket_rw_buffer_size_;

  void close() override;

  void init() override;

  size_t receive(char* buff, size_t len,
                 std::chrono::microseconds wait) override;
  size_t send(const char* buff, size_t len,
              std::chrono::microseconds wait) override;

  virtual uint16_t getPort() override;

 protected:
  uint16_t port_;
  std::string host_;
  std::chrono::microseconds connect_timeout_;

 public:
  TcpConn(const char* hostname, int32_t port, std::chrono::microseconds wait,
          int32_t maxBuffSizePool);
  TcpConn(const char* ipaddr, std::chrono::microseconds wait,
          int32_t maxBuffSizePool);

  ~TcpConn() override;

  virtual void connect();
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCPCONN_H_
