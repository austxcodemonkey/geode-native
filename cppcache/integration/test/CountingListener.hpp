#pragma once

#ifndef GEODE_INTEGRATION_TEST_COUNTINGLISTENER_H_
#define GEODE_INTEGRATION_TEST_COUNTINGLISTENER_H_

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

#include <string>
#include <util/Log.hpp>

#include <geode/EntryEvent.hpp>

namespace apache {
namespace geode {
namespace client {
namespace testing {

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheListener;
using apache::geode::client::EntryEvent;
using apache::geode::client::RegionEvent;

class CountingListener : public CacheListener {
 private:
  int creates_;
  int updates_;
  int invalidates_;
  int destroys_;
  int clears_;
  bool listenerInvoked_;
  bool callbackCalled_;
  std::shared_ptr<CacheableKey> lastKey_;
  std::shared_ptr<Cacheable> lastValue_;
  std::shared_ptr<CacheableKey> callbackArg_;
  bool ignoreTimeout_;
  bool quiet_;

 public:
  CountingListener()
      : CacheListener(),
        creates_(0),
        updates_(0),
        invalidates_(0),
        destroys_(0),
        clears_(0),
        listenerInvoked_(false),
        callbackCalled_(false),
        lastKey_(),
        lastValue_(),
        callbackArg_(nullptr),
        ignoreTimeout_(false),
        quiet_(false) {}

  ~CountingListener() noexcept override = default;

  void beQuiet(bool v) { quiet_ = v; }

  int expectCreates(int expected) {
    int tries = 0;
    while ((creates_ < expected) && (tries < 200)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      tries++;
    }
    return creates_;
  }

  int getCreates() { return creates_; }

  int expectUpdates(int expected) {
    int tries = 0;
    while ((updates_ < expected) && (tries < 200)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      tries++;
    }
    return updates_;
  }
  void resetListnerInvocation() {
    listenerInvoked_ = false;
    callbackCalled_ = false;
  }
  int getUpdates() { return updates_; }

  int getInvalidates() { return invalidates_; }
  int getDestroys() { return destroys_; }
  bool isListenerInvoked() { return listenerInvoked_; }
  void setCallBackArg(const std::shared_ptr<CacheableKey>& callbackArg) {
    callbackArg_ = callbackArg;
  }
  std::shared_ptr<CacheableKey> getLastKey() { return lastKey_; }

  std::shared_ptr<Cacheable> getLastValue() { return lastValue_; }
  bool isCallBackArgCalled() { return callbackCalled_; }
  void checkcallbackArg(const EntryEvent& event) {
    if (!listenerInvoked_) listenerInvoked_ = true;
    if (callbackArg_ != nullptr) {
      auto callbkArg =
          std::dynamic_pointer_cast<CacheableKey>(event.getCallbackArgument());
      if (strcmp(callbackArg_->toString().c_str(),
                 callbkArg->toString().c_str()) == 0) {
        callbackCalled_ = true;
      }
    }
  }

  int getClears() { return clears_; }

  void afterCreate(const EntryEvent& event) override;

  void afterUpdate(const EntryEvent& event) override;

  void afterInvalidate(const EntryEvent& event) override;

  void afterDestroy(const EntryEvent& event) override;

  void afterRegionClear(const RegionEvent& event) override {
    CacheListener::afterRegionClear(event);
  }

  void afterRegionInvalidate(const RegionEvent&) override {}

  void afterRegionDestroy(const RegionEvent&) override {}
};

void CountingListener::afterCreate(const EntryEvent& event) {
  creates_++;
  LOGDEBUG("CountingListener::afterCreate called m_creates = %d ", creates_);
  lastKey_ = event.getKey();
  lastValue_ = event.getNewValue();
  checkcallbackArg(event);

  auto val =
      std::dynamic_pointer_cast<CacheableInt32>(event.getNewValue())->value();
  if (!quiet_) {
    char buf[1024];
    sprintf(buf, "CountingListener create - key = \"%s\", value = %d`",
            lastKey_->toString().c_str(), val);
    LOGDEBUG(buf);
  }
  std::string keyString(lastKey_->toString().c_str());
  if ((!ignoreTimeout_) && (keyString.find("timeout") != std::string::npos)) {
    // Sleep 10 seconds to force a timeout
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }
}

void CountingListener::afterUpdate(const EntryEvent& event) {
  updates_++;
  lastKey_ = event.getKey();
  lastValue_ = event.getNewValue();
  checkcallbackArg(event);
  auto strPtr = std::dynamic_pointer_cast<CacheableString>(event.getNewValue());
  if (!quiet_) {
    char buf[1024];
  }
  std::string keyString(lastKey_->toString().c_str());
  if ((!ignoreTimeout_) && (keyString.find("timeout") != std::string::npos)) {
    // Sleep 10 seconds to force a timeout
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }
}
void CountingListener::afterInvalidate(const EntryEvent& event) {
  invalidates_++;
  checkcallbackArg(event);
}
void CountingListener::afterDestroy(const EntryEvent& event) {
  destroys_++;
  checkcallbackArg(event);
}

}  // namespace testing
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_INTEGRATION_TEST_COUNTINGLISTENER_H_
