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

#ifndef GEODE_TCRENDPOINT_H_
#define GEODE_TCRENDPOINT_H_

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>

#include <ace/Condition_Recursive_Thread_Mutex.h>
#include <ace/Semaphore.h>

#include <geode/internal/geode_base.hpp>
#include <geode/internal/geode_globals.hpp>

#include "ErrType.hpp"
#include "FairQueue.hpp"
#include "Task.hpp"
#include "TcrConnection.hpp"
#include "util/synchronized_set.hpp"

namespace apache {
namespace geode {
namespace client {

class ThinClientRegion;
class TcrMessage;
class ThinClientBaseDM;
class CacheImpl;
class ThinClientPoolHADM;
class ThinClientPoolDM;
class QueryService;

class TcrEndpoint {
 public:
  TcrEndpoint(
      const std::string& name, CacheImpl* cacheImpl,
      ACE_Semaphore& failoverSema, ACE_Semaphore& cleanupSema,
      ACE_Semaphore& redundancySema, ThinClientBaseDM* dm = nullptr,
      bool isMultiUserMode = false);  // TODO: need to look for endpoint case

  /* adongre
   * CID 29000: Non-virtual destructor (VIRTUAL_DTOR)
   */
  virtual ~TcrEndpoint();

  virtual GfErrType registerDM(bool clientNotification,
                               bool isSecondary = false,
                               bool isActiveEndpoint = false,
                               ThinClientBaseDM* distMgr = nullptr);
  // GfErrType registerPoolDM( bool isSecondary, ThinClientPoolHADM* poolDM );

  virtual void unregisterDM(bool clientNotification,
                            ThinClientBaseDM* distMgr = nullptr,
                            bool checkQueueHosted = false);
  // void unregisterPoolDM(  );

  void pingServer(ThinClientPoolDM* poolDM = nullptr);
  void receiveNotification(std::atomic<bool>& isRunning);
  GfErrType send(const TcrMessage& request, TcrMessageReply& reply);
  GfErrType sendRequestConn(const TcrMessage& request, TcrMessageReply& reply,
                            TcrConnection* conn, std::string& failReason);
  GfErrType sendRequestWithRetry(const TcrMessage& request,
                                 TcrMessageReply& reply, TcrConnection*& conn,
                                 bool& epFailure, std::string& failReason,
                                 int maxSendRetries, bool useEPPool,
                                 std::chrono::microseconds requestedTimeout,
                                 bool isBgThread = false);
  GfErrType sendRequestConnWithRetry(const TcrMessage& request,
                                     TcrMessageReply& reply,
                                     TcrConnection*& conn,
                                     bool isBgThread = false);

  void stopNotifyReceiverAndCleanup();
  void stopNoBlock();

  bool inline connected() const { return m_connected; }

  int inline numRegions() const { return m_numRegions; }

  void inline setNumRegions(int numRegions) { m_numRegions = numRegions; }

  inline const std::string& name() const { return m_name; }

  //  setConnectionStatus is now a public method, as it is used by
  //  TcrDistributionManager.
  void setConnectionStatus(bool status);

  inline int getNumRegionListeners() const { return m_numRegionListener; }

  // TODO: for single user mode only
  void setUniqueId(int64_t uniqueId) {
    LOGDEBUG("tcrEndpoint:setUniqueId:: %d ", uniqueId);
    m_isAuthenticated = true;
    m_uniqueId = uniqueId;
  }

  int64_t getUniqueId() {
    LOGDEBUG("tcrEndpoint:getUniqueId:: %d ", m_uniqueId);
    return m_uniqueId;
  }

  bool isAuthenticated() { return m_isAuthenticated; }

  void setAuthenticated(bool isAuthenticated) {
    m_isAuthenticated = isAuthenticated;
  }

  virtual bool isMultiUserMode();
  /*{
    if(m_baseDM != nullptr)
      return this->m_baseDM->isMultiUserMode();
    else
      return false;
  }*/

  void authenticateEndpoint(TcrConnection*& conn);

  ServerQueueStatus getFreshServerQueueStatus(int32_t& queueSize,
                                              bool addToQueue,
                                              TcrConnection*& statusConn);

  //  TESTING: return true or false
  bool inline getServerQueueStatusTEST() {
    return (m_serverQueueStatus == REDUNDANT_SERVER ||
            m_serverQueueStatus == PRIMARY_SERVER);
  }

  // Get cached server queue props.
  int32_t inline getServerQueueSize() { return m_queueSize; }
  ServerQueueStatus getServerQueueStatus() { return m_serverQueueStatus; }

  // Set server queue props.
  void setServerQueueStatus(ServerQueueStatus queueStatus, int32_t queueSize);

  GfErrType createNewConnection(
      TcrConnection*& newConn, bool isClientNotification = false,
      bool isSecondary = false,
      std::chrono::microseconds connectTimeout = DEFAULT_CONNECT_TIMEOUT,
      int32_t timeoutRetries = 1, bool sendUpdateNotification = true,
      bool appThreadRequest = false);

  bool needtoTakeConnectLock();

  GfErrType createNewConnectionWL(TcrConnection*& newConn,
                                  bool isClientNotification, bool isSecondary,
                                  std::chrono::microseconds connectTimeout);

  void setConnected(volatile bool connected = true) { m_connected = connected; }
  virtual ThinClientPoolDM* getPoolHADM() { return nullptr; }
  bool isQueueHosted();
  std::recursive_mutex& getQueueHostedMutex() { return m_notifyReceiverLock; }
  /*
  void sendNotificationCloseMsg();
  */

  void setDM(ThinClientBaseDM* dm) {
    LOGDEBUG("tcrendpoint setDM");
    this->m_baseDM = dm;
  }

  int32_t numberOfTimesFailed() { return m_numberOfTimesFailed; }

  void addConnRefCounter(int count) { m_noOfConnRefs += count; }

  int getConnRefCounter() { return m_noOfConnRefs; }
  virtual uint16_t getDistributedMemberID() { return m_distributedMemId; }
  virtual void setDistributedMemberID(uint16_t memId) {
    m_distributedMemId = memId;
  }

 protected:
  TcrConnection* m_notifyConnection;
  std::unique_ptr<Task<TcrEndpoint>> m_notifyReceiver;
  CacheImpl* m_cacheImpl;
  std::list<Task<TcrEndpoint>*> m_notifyReceiverList;
  std::list<TcrConnection*> m_notifyConnectionList;
  std::timed_mutex m_connectLock;
  std::recursive_mutex m_notifyReceiverLock;
  FairQueue<TcrConnection> m_opConnections;
  volatile int m_maxConnections;
  int m_numRegionListener;
  volatile bool m_needToConnectInLock;
  bool m_isQueueHosted;

  static const char* NC_Notification;

  std::shared_ptr<Properties> getCredentials();
  virtual bool checkDupAndAdd(std::shared_ptr<EventId> eventid);
  virtual void processMarker();
  virtual void triggerRedundancyThread();
  virtual std::shared_ptr<QueryService> getQueryService();
  virtual void sendRequestForChunkedResponse(const TcrMessage& request,
                                             TcrMessageReply& reply,
                                             TcrConnection* conn);
  virtual void closeFailedConnection(TcrConnection*& conn);
  void closeConnection(TcrConnection*& conn);
  virtual void handleNotificationStats(int64_t byteLength);
  virtual void closeNotification();

  virtual bool handleIOException(const std::string& message,
                                 TcrConnection*& conn, bool isBgThread = false);

 private:
  int64_t m_uniqueId;
  ACE_Semaphore& m_failoverSema;
  ACE_Semaphore& m_cleanupSema;
  ACE_Semaphore& m_redundancySema;
  ThinClientBaseDM* m_baseDM;
  std::string m_name;
  std::list<ThinClientBaseDM*> m_distMgrs;
  std::recursive_mutex m_endpointAuthenticationLock;
  std::recursive_mutex m_connectionLock;
  std::recursive_mutex m_distMgrsLock;
  ACE_Semaphore m_notificationCleanupSema;
  synchronized_set<std::unordered_set<uint16_t>> m_ports;
  int32_t m_numberOfTimesFailed;
  int m_numRegions;
  int m_pingTimeouts;
  int m_notifyCount;
  uint32_t m_dupCount;
  bool m_isAuthenticated;
  volatile bool m_msgSent;
  volatile bool m_pingSent;
  bool m_isMultiUserMode;
  volatile bool m_connected;
  bool m_isActiveEndpoint;
  ServerQueueStatus m_serverQueueStatus;
  int32_t m_queueSize;
  std::atomic<int32_t> m_noOfConnRefs;
  uint16_t m_distributedMemId;
  bool m_isServerQueueStatusSet;

  bool compareTransactionIds(int32_t reqTransId, int32_t replyTransId,
                             std::string& failReason, TcrConnection* conn);
  void closeConnections();
  void setRetry(const TcrMessage& request, int& maxSendRetries);
  // number of connections to this endpoint

  // Disallow copy constructor and assignment operator.
  TcrEndpoint(const TcrEndpoint&);
  TcrEndpoint& operator=(const TcrEndpoint&);
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCRENDPOINT_H_
