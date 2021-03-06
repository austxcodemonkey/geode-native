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

#ifndef GEODE_TCRCONNECTION_H_
#define GEODE_TCRCONNECTION_H_

#include <atomic>
#include <chrono>

#include <ace/Semaphore.h>

#include <geode/CacheableBuiltins.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>

#include "Connector.hpp"
#include "DiffieHellman.hpp"
#include "TcrMessage.hpp"
#include "util/synchronized_set.hpp"

#define DEFAULT_TIMEOUT_RETRIES 12
#define PRIMARY_SERVER_TO_CLIENT 101
#define SECONDARY_SERVER_TO_CLIENT 102
#define SUCCESSFUL_SERVER_TO_CLIENT 105
#define UNSUCCESSFUL_SERVER_TO_CLIENT 106
#define CLIENT_TO_SERVER 100
#define REPLY_OK 59
#define REPLY_OK_CS43 58
#define REPLY_REFUSED 60
#define REPLY_INVALID 61
#define REPLY_SSL_ENABLED 21
#define REPLY_AUTHENTICATION_REQUIRED 62
#define REPLY_AUTHENTICATION_FAILED 63
#define REPLY_DUPLICATE_DURABLE_CLIENT 64

#define SECURITY_CREDENTIALS_NONE 0
#define SECURITY_CREDENTIALS_NORMAL 1
#define SECURITY_CREDENTIALS_DHENCRYPT 2
#define SECURITY_MULTIUSER_NOTIFICATIONCHANNEL 3

/** Closes and Deletes connection only if it exists */
#define GF_SAFE_DELETE_CON(x) \
  {                           \
    x->close();               \
    delete x;                 \
    x = nullptr;              \
  }

namespace apache {
namespace geode {
namespace client {

enum ConnErrType {
  CONN_NOERR = 0x0,
  CONN_NODATA = 0x1,
  CONN_TIMEOUT = 0x3,
  CONN_IOERR = 0x4,
  CONN_OTHERERR = 0x8
};

enum ServerQueueStatus {
  NON_REDUNDANT_SERVER = 0,
  REDUNDANT_SERVER = 1,
  PRIMARY_SERVER = 2
};

class TcrEndpoint;
class SystemProperties;
class ThinClientPoolDM;
class TcrConnectionManager;
class APACHE_GEODE_EXPORT TcrConnection {
 public:
  using clock = std::chrono::steady_clock;
  using time_point = clock::time_point;

  /** Create one connection, endpoint is in format of hostname:portno
   * It will do handshake with j-server. There're 2 types of handshakes:
   * 1) handshake for request
   *    send following bytes:
   *    CLIENT_TO_SERVER
   *    REPLY_OK
   *    2 bytes for the length of idenfifier
   *    a string with "hostname:processId" as identifier
   *
   *    if send succeeds, handshake succeeds. Otherwise, construction
   *    fails.
   *
   * 2) handshake for client notification
   *    send following bytes:
   *    SERVER_TO_CLIENT
   *    1 (4 bytes, we can hard-code)
   *    12345 (4 bytes, we can hard-code)
   *
   *    So the total bytes to send are 9
   *    read one byte from server, it should be CLIENT_TO_SERVER
   *    Otherwise, construction fails.
   * @param     ports     List of local ports for connections to endpoint
   * @param     numPorts  Size of ports list
   */
  bool InitTcrConnection(
      TcrEndpoint* endpointObj, const char* endpoint,
      synchronized_set<std::unordered_set<uint16_t>>& ports,
      bool isClientNotification = false, bool isSecondary = false,
      std::chrono::microseconds connectTimeout = DEFAULT_CONNECT_TIMEOUT);

  TcrConnection(const TcrConnectionManager& connectionManager,
                volatile const bool& isConnected)
      : connectionId(0),
        m_connectionManager(&connectionManager),
        m_dh(nullptr),
        m_endpoint(nullptr),
        m_endpointObj(nullptr),
        m_connected(isConnected),
        m_conn(nullptr),
        m_hasServerQueue(NON_REDUNDANT_SERVER),
        m_queueSize(0),
        m_port(0),
        m_chunksProcessSema(0),
        m_isBeingUsed(false),
        m_isUsed(0),
        m_poolDM(nullptr) {}

  /* destroy the connection */
  ~TcrConnection();

  /**
   * send a synchronized request to server.
   *
   * It will send the buffer, then wait to receive 17 bytes and save in
   * msg_header.
   * msg_header[0] is message type.
   * msg_header[1],msg_header[2],msg_header[3],msg_header[4] will be a 4 bytes
   * integer,
   * let's say, msgLen, which specifies the length of next read. byteReads some
   * number of
   * call read again for msgLen bytes, and save the bytes into msg_body.
   * concatenate the msg_header and msg_body into buffer, msg. The msg should be
   * a '0' ended
   * string. i.e. If the msg_header plus msg_body has 100 chars, msg should be a
   * 101 char array
   * to contain the '0' in the end. We need it to get length of the msg.
   * Return the msg.
   *
   * @param      buffer the buffer to send
   * @param      len length of the data to send
   * @param      sendTimeoutSec write timeout in sec
   * @param      recvLen output parameter for length of the received message
   * @param      receiveTimeoutSec read timeout in sec
   * @return     byte arrary of response. '0' ended.
   * @exception  GeodeIOException  if an I/O error occurs (socket failure).
   * @exception  TimeoutException  if timeout happens at any of the 3 socket
   * operation: 1 write, 2 read
   */
  char* sendRequest(
      const char* buffer, size_t len, size_t* recvLen,
      std::chrono::microseconds sendTimeoutSec = DEFAULT_WRITE_TIMEOUT,
      std::chrono::microseconds receiveTimeoutSec = DEFAULT_READ_TIMEOUT_SECS,
      int32_t request = -1);

  /**
   * send a synchronized request to server for REGISTER_INTEREST_LIST.
   *
   * @param      buffer the buffer to send
   *             len length of the data to send
   *             message vector, which will return chunked TcrMessage.
   *             sendTimeoutSec write timeout in sec
   *             receiveTimeoutSec read timeout in sec
   * @exception  GeodeIOException  if an I/O error occurs (socket failure).
   * @exception  TimeoutException  if timeout happens at any of the 3 socket
   * operation: 1 write, 2 read
   */
  void sendRequestForChunkedResponse(
      const TcrMessage& request, size_t len, TcrMessageReply& message,
      std::chrono::microseconds sendTimeoutSec = DEFAULT_WRITE_TIMEOUT,
      std::chrono::microseconds receiveTimeoutSec = DEFAULT_READ_TIMEOUT_SECS);

  /**
   * send an asynchronized request to server. No response is expected.
   * we need to use it to send CLOSE_CONNECTION msg
   *
   * @param      buffer the buffer to send
   *             len length of the data to send
   *             sendTimeoutSec write timeout in sec
   * @return     no return. Because it either succeeds, or throw exception.
   * @exception  GeodeIOException  if an I/O error occurs (socket failure).
   * @exception  TimeoutException  if timeout happens at any of the 3 socket
   * operation: 1 write, 2 read
   */
  void send(const char* buffer, size_t len,
            std::chrono::microseconds sendTimeoutSec = DEFAULT_WRITE_TIMEOUT,
            bool checkConnected = true);

  void send(std::chrono::microseconds& timeSpent, const char* buffer,
            size_t len,
            std::chrono::microseconds sendTimeoutSec = DEFAULT_WRITE_TIMEOUT,
            bool checkConnected = true);

  /**
   * This method is for receiving client notification. It will read 2 times as
   * reading reply in sendRequest()
   *
   * @param      recvLen output parameter for length of the received message
   * @param      receiveTimeoutSec read timeout in sec
   * @return     byte arrary of response. '0' ended.
   * @exception  GeodeIOException  if an I/O error occurs (socket failure).
   * @exception  TimeoutException  if timeout happens at any of the 3 socket
   * operation: 1 write, 2 read
   */
  char* receive(
      size_t* recvLen, ConnErrType* opErr,
      std::chrono::microseconds receiveTimeoutSec = DEFAULT_READ_TIMEOUT_SECS);

  //  readMessage is now public
  /**
   * This method reads a message from the socket connection and returns the byte
   * array of response.
   * @param      recvLen output parameter for length of the received message
   * @param      receiveTimeoutSec read timeout in seconds
   * @param      doHeaderTimeoutRetries retry when header receive times out
   * @return     byte array of response. '0' ended.
   * @exception  GeodeIOException  if an I/O error occurs (socket failure).
   * @exception  TimeoutException  if timeout happens during read
   */
  char* readMessage(size_t* recvLen,
                    std::chrono::microseconds receiveTimeoutSec,
                    bool doHeaderTimeoutRetries, ConnErrType* opErr,
                    bool isNotificationMessage = false, int32_t request = -1);

  /**
   * This method reads an interest list response  message from the socket
   * connection and sets the reply message
   * parameter.
   * @param      reply response message
   * @param      receiveTimeoutSec read timeout in sec
   * @param      doHeaderTimeoutRetries retry when header receive times out
   * @exception  GeodeIOException  if an I/O error occurs (socket failure).
   * @exception  TimeoutException  if timeout happens during read
   */
  void readMessageChunked(TcrMessageReply& reply,
                          std::chrono::microseconds receiveTimeoutSec,
                          bool doHeaderTimeoutRetries);

  /**
   * Send close connection message to the server.
   */
  void close();

  //  Durable clients: return true if server has HA queue.
  ServerQueueStatus inline getServerQueueStatus(int32_t& queueSize) {
    queueSize = m_queueSize;
    return m_hasServerQueue;
  }

  uint16_t inline getPort() { return m_port; }

  TcrEndpoint* getEndpointObject() const { return m_endpointObj; }
  bool isBeingUsed() { return m_isBeingUsed; }
  bool setAndGetBeingUsed(
      volatile bool isBeingUsed,
      bool forTransaction);  // { m_isBeingUsed = isBeingUsed ;}

  // helpers for pool connection manager
  void touch();
  bool hasExpired(const std::chrono::milliseconds& expiryTime);
  bool isIdle(const std::chrono::milliseconds& idleTime);
  time_point getLastAccessed();
  void updateCreationTime();

  int64_t getConnectionId() {
    LOGDEBUG("TcrConnection::getConnectionId() = %d ", connectionId);
    return connectionId;
  }

  void setConnectionId(int64_t id) {
    LOGDEBUG("Tcrconnection:setConnectionId() = %d ", id);
    connectionId = id;
  }

  const TcrConnectionManager& getConnectionManager() {
    return *m_connectionManager;
  }

  std::shared_ptr<CacheableBytes> encryptBytes(
      std::shared_ptr<CacheableBytes> data) {
    if (m_dh != nullptr) {
      return m_dh->encrypt(data);
    } else {
      return data;
    }
  }

  std::shared_ptr<CacheableBytes> decryptBytes(
      std::shared_ptr<CacheableBytes> data) {
    if (m_dh != nullptr) {
      return m_dh->decrypt(data);
    } else {
      return data;
    }
  }

 private:
  int64_t connectionId;
  const TcrConnectionManager* m_connectionManager;
  DiffieHellman* m_dh;
  /**
   * To read Intantiator message(which meant for java client), here we are
   * ignoring it
   */
  void readHandshakeInstantiatorMsg(std::chrono::microseconds connectTimeout);

  /**
   * Packs the override settings bits into bytes - currently a single byte for
   * conflation, remove-unresponsive-client and notify-by-subscription.
   */
  uint8_t getOverrides(const SystemProperties* props);

  /**
   * To read the from stream
   */
  int32_t readHandShakeInt(std::chrono::microseconds connectTimeout);

  /*
   * To read the arraysize
   */
  uint32_t readHandshakeArraySize(std::chrono::microseconds connectTimeout);

  /*
   * This function reads "numberOfBytes" and ignores it.
   */
  void readHandShakeBytes(int numberOfBytes,
                          std::chrono::microseconds connectTimeout);

  /** Create a normal or SSL connection */
  Connector* createConnection(
      const char* ipaddr,
      std::chrono::microseconds waitSeconds = DEFAULT_CONNECT_TIMEOUT,
      int32_t maxBuffSizePool = 0);

  /**
   * Reads bytes from socket and handles error conditions in case of Handshake.
   */
  std::vector<int8_t> readHandshakeData(
      int32_t msgLength, std::chrono::microseconds connectTimeout);

  /**
   * Reads raw bytes (without appending nullptr terminator) from socket and
   * handles error conditions in case of Handshake.
   */
  std::shared_ptr<CacheableBytes> readHandshakeRawData(
      int32_t msgLength, std::chrono::microseconds connectTimeout);
  /**
   * Reads a string from socket and handles error conditions in case of
   * Handshake.
   */
  std::shared_ptr<CacheableString> readHandshakeString(
      std::chrono::microseconds connectTimeout);

  /**
   * Reads a byte array (using initial length) from socket and handles error
   * conditions in case of Handshake.
   */
  std::shared_ptr<CacheableBytes> readHandshakeByteArray(
      std::chrono::microseconds connectTimeout);

  /**
   * Send data to the connection till sendTimeout
   */
  ConnErrType sendData(const char* buffer, size_t length,
                       std::chrono::microseconds sendTimeout,
                       bool checkConnected = true);

  ConnErrType sendData(std::chrono::microseconds& timeSpent, const char* buffer,
                       size_t length, std::chrono::microseconds sendTimeout,
                       bool checkConnected = true);

  /**
   * Read data from the connection till receiveTimeoutSec
   */
  ConnErrType receiveData(char* buffer, size_t length,
                          std::chrono::microseconds receiveTimeoutSec,
                          bool checkConnected = true,
                          bool isNotificationMessage = false);

  const char* m_endpoint;
  TcrEndpoint* m_endpointObj;
  volatile const bool& m_connected;
  Connector* m_conn;
  ServerQueueStatus m_hasServerQueue;
  int32_t m_queueSize;
  uint16_t m_port;

  // semaphore to synchronize with the chunked response processing thread
  ACE_Semaphore m_chunksProcessSema;

  time_point m_creationTime;
  time_point m_lastAccessed;

  // Disallow copy constructor and assignment operator.
  TcrConnection(const TcrConnection&);
  TcrConnection& operator=(const TcrConnection&);
  volatile bool m_isBeingUsed;
  std::atomic<uint32_t> m_isUsed;
  ThinClientPoolDM* m_poolDM;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCRCONNECTION_H_
