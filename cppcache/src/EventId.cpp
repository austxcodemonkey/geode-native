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
#include "EventId.hpp"

#include <inttypes.h>

#include <atomic>
#include <cstring>

#include <geode/DataInput.hpp>

#include "ClientProxyMembershipID.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

const std::string disclaimer(
    "This is a private (debugging) build for BMO.  It is UNSUPPORTED for "
    "production use.");

class EventIdTSS {
 private:
  static std::atomic<int64_t> s_eidThrId;

  int64_t m_eidThrTSS;
  int64_t m_eidSeqTSS;

  ~EventIdTSS() = default;
  EventIdTSS(const EventIdTSS&) = delete;
  EventIdTSS& operator=(const EventIdTSS&) = delete;

 public:
  // this should get called just once per thread due to first access to TSS
  EventIdTSS() {
    m_eidThrTSS = ++s_eidThrId;
    m_eidSeqTSS = 0;
  }

  inline int64_t getEidThr() { return m_eidThrTSS; }

  inline int64_t getAndIncEidSeq() { return m_eidSeqTSS++; }

  inline int64_t getSeqNum() { return m_eidSeqTSS - 1; }

  static thread_local EventIdTSS s_eventId;

};  // class EventIdTSS

std::atomic<int64_t> EventIdTSS::s_eidThrId;
thread_local EventIdTSS EventIdTSS::s_eventId;

void EventId::toData(DataOutput& output) const {
  //  This method is always expected to write out nonstatic distributed
  //  memberid.
  LOGDEBUG("%s(%p) - called", __FUNCTION__, this);
  output.writeBytes(reinterpret_cast<const int8_t*>(m_eidMem), m_eidMemLen);
  output.writeArrayLen(18);
  char longCode = 3;
  output.write(static_cast<uint8_t>(longCode));
  output.writeInt(m_eidThr);
  output.write(static_cast<uint8_t>(longCode));
  output.writeInt(m_eidSeq);
  output.writeInt(m_bucketId);
  output.write(m_breadcrumbCounter);
}

void EventId::fromData(DataInput& input) {
  LOGDEBUG("%s(%p) - called", __FUNCTION__, this);
  // TODO: statics being assigned; not thread-safe??
  m_eidMemLen = input.readArrayLength();
  input.readBytesOnly(reinterpret_cast<int8_t*>(m_eidMem), m_eidMemLen);
  input.readArrayLength();  // ignore arrayLen
  m_eidThr = getEventIdData(input, input.read());
  m_eidSeq = getEventIdData(input, input.read());
  m_bucketId = input.readInt32();
  m_breadcrumbCounter = input.read();
}

const char* EventId::getMemId() const { return m_eidMem; }

int32_t EventId::getMemIdLen() const { return m_eidMemLen; }

int64_t EventId::getThrId() const { return m_eidThr; }

int64_t EventId::getSeqNum() const { return m_eidSeq; }

int64_t EventId::getEventIdData(DataInput& input, char numberCode) {
  int64_t retVal = 0;
  LOGDEBUG("%s(%p) - called", __FUNCTION__, this);

  //  Read number based on numeric code written by java server.
  if (numberCode == 0) {
    return input.read();
  } else if (numberCode == 1) {
    retVal = input.readInt16();
  } else if (numberCode == 2) {
    int32_t intVal;
    intVal = input.readInt32();
    retVal = intVal;
  } else if (numberCode == 3) {
    int64_t longVal;
    longVal = input.readInt64();
    retVal = longVal;
  }

  return retVal;
}

std::shared_ptr<Serializable> EventId::createDeserializable() {
  LOGDEBUG("%s - called", __FUNCTION__);
  return std::make_shared<EventId>(false);
  // use false since we dont want to inc sequence
  // (for de-serialization)
}

EventId::EventId(char* memId, uint32_t memIdLen, int64_t thr, int64_t seq) {
  LOGDEBUG("%s(%p) - memId=%s, memIdLen=%d, thr=%lld, seq=%lld", __FUNCTION__,
           this, memId, memIdLen, thr, seq);
  // TODO: statics being assigned; not thread-safe??
  std::memcpy(m_eidMem, memId, memIdLen);
  m_eidMemLen = memIdLen;
  m_eidThr = thr;
  m_eidSeq = seq;
  m_bucketId = -1;
  m_breadcrumbCounter = 0;
}

EventId::EventId(bool doInit, uint32_t reserveSize,
                 bool fullValueAfterDeltaFail)
    : /* adongre
       * CID 28934: Uninitialized scalar field (UNINIT_CTOR)
       */
      m_eidMemLen(0),
      m_eidThr(0),
      m_eidSeq(0),
      m_bucketId(-1),
      m_breadcrumbCounter(0) {
  LOGDEBUG("%s(%p) - doInit=%s, reserveSize=%d, fullValueAfterDeltaFail=%s",
           __FUNCTION__, this, doInit ? "true" : "false", reserveSize,
           fullValueAfterDeltaFail ? "true" : "false");
  Exception ex("Debugging - used to dump callstack");
  LOGDEBUG("%s(%p) - callstack\n%s", __FUNCTION__, this,
           ex.getStackTrace().c_str());
  if (!doInit) return;

  if (fullValueAfterDeltaFail) {
    /// need to send old sequence id
    initFromTSS_SameThreadIdAndSameSequenceId();
  } else {
    initFromTSS();
  }

  for (uint32_t i = 0; i < reserveSize; i++) {
    EventIdTSS::s_eventId.getAndIncEidSeq();
  }
}

void EventId::initFromTSS() {
  m_eidThr = EventIdTSS::s_eventId.getEidThr();
  m_eidSeq = EventIdTSS::s_eventId.getAndIncEidSeq();
  LOGDEBUG("%s(%p) - called, tid=%lld, seqid=%lld", __FUNCTION__, this,
           m_eidThr, m_eidSeq);
}

void EventId::initFromTSS_SameThreadIdAndSameSequenceId() {
  m_eidThr = EventIdTSS::s_eventId.getEidThr();
  m_eidSeq = EventIdTSS::s_eventId.getSeqNum();
  LOGDEBUG("%s(%p) - called, tid=%lld, seqid=%lld", __FUNCTION__, this,
           m_eidThr, m_eidSeq);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
