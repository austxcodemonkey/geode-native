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

#ifndef GEODE_TCPSSLCONN_H_
#define GEODE_TCPSSLCONN_H_

#include <ace/DLL.h>

#include "../../cryptoimpl/Ssl.hpp"
#include "TcpConn.hpp"

namespace apache {
namespace geode {
namespace client {

class TcpSslConn : public TcpConn {
 private:
  Ssl* m_ssl;
  ACE_DLL m_dll;
  uint16_t m_sniPort;
  std::string m_sniHostname;
  std::string m_pubkeyfile;
  std::string m_privkeyfile;
  std::string m_pemPassword;

  typedef void* (*gf_create_SslImpl)(ACE_HANDLE, const char*, const char*,
                                     const char*);
  typedef void (*gf_destroy_SslImpl)(void*);

  Ssl* getSSLImpl(ACE_HANDLE sock, const char* pubkeyfile,
                  const char* privkeyfile);

 protected:
  size_t socketOp(SockOp op, char* buff, size_t len,
                  std::chrono::microseconds waitDuration) override;

  void createSocket(ACE_HANDLE sock) override;

 public:
  TcpSslConn(
             std::chrono::microseconds waitSeconds, int32_t maxBuffSizePool,
             const std::string& sniProxyHostname, uint16_t sniProxyPort,
             const std::string& pubkeyfile, const std::string& privkeyfile,
             const std::string& pemPassword)
      : TcpConn(sniProxyHostname.c_str(), sniProxyPort, waitSeconds, maxBuffSizePool),
        m_ssl(nullptr),
        m_sniPort(sniProxyPort),
        m_sniHostname(sniProxyHostname),
        m_pubkeyfile(pubkeyfile),
        m_privkeyfile(privkeyfile),
        m_pemPassword(pemPassword) {}

  TcpSslConn(const std::string& hostname, uint16_t port,
             std::chrono::microseconds connect_timeout, int32_t maxBuffSizePool,
             const std::string& pubkeyfile, const std::string& privkeyfile,
             const std::string& pemPassword)
      : TcpConn(hostname.c_str(), port, connect_timeout, maxBuffSizePool),
        m_ssl(nullptr),
        m_sniPort(0),
        m_sniHostname(""),
        m_pubkeyfile(pubkeyfile),
        m_privkeyfile(privkeyfile),
        m_pemPassword(pemPassword) {}

  TcpSslConn(const char* ipaddr, std::chrono::microseconds waitSeconds,
             int32_t maxBuffSizePool, const char* pubkeyfile,
             const char* privkeyfile, const char* pemPassword)
      : TcpConn(ipaddr, waitSeconds, maxBuffSizePool),
        m_ssl(nullptr),
        m_sniPort(0),
        m_sniHostname(""),
        m_pubkeyfile(pubkeyfile),
        m_privkeyfile(privkeyfile),
        m_pemPassword(pemPassword) {}



  virtual ~TcpSslConn() override {}

 private:
  void initSsl(const std::string& pubkeyfile, const std::string& privkeyfile,
               const std::string& pemPassword,
               const std::string& sniHostname = "", const uint16_t sniPort = 0);
  // Close this tcp connection
  void close() override;

  // Listen
  void listen(ACE_INET_Addr addr, std::chrono::microseconds waitSeconds =
                                      DEFAULT_READ_TIMEOUT) override;

  // connect
  void connect() override;

  void setOption(int32_t level, int32_t option, void* val,
                 size_t len) override {
    if (m_ssl->setOption(level, option, val, static_cast<int32_t>(len)) == -1) {
      int32_t lastError = ACE_OS::last_error();
      LOGERROR("Failed to set option, errno: %d: %s", lastError,
               ACE_OS::strerror(lastError));
    }
  }

  uint16_t getPort() override;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCPSSLCONN_H_
