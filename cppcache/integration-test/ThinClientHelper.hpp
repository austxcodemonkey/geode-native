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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTHELPER_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTHELPER_H_

#include <chrono>

#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include "testUtils.hpp"
#include "security/typedefs.hpp"
#include "security/CredentialGenerator.hpp"
#include "security/DummyCredentialGenerator.hpp"
#include "security/CredentialGenerator.cpp"

#include <string>

#ifndef ROOT_NAME
#define ROOT_NAME "ThinClientHelper"
#endif

#include "CacheHelper.hpp"

using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheHelper;
using apache::geode::client::CacheListener;
using apache::geode::client::DiskPolicyType;
using apache::geode::client::ExpirationAction;
using apache::geode::client::Pool;
using apache::geode::client::Properties;
using apache::geode::client::RegionAttributesFactory;
using apache::geode::client::Serializable;
using unitTests::TestUtils;

void initGridClient(const bool isthinClient,
                    const std::shared_ptr<Properties>& configPtr = nullptr) {
  static bool s_isGridClient = true;

  s_isGridClient = !s_isGridClient;
  if (cacheHelper == nullptr) {
    auto config = configPtr;
    if (config == nullptr) {
      config = Properties::create();
    }
    config->insert("grid-client", s_isGridClient ? "true" : "false");
    cacheHelper = new CacheHelper(isthinClient, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void initClientWithPool(const bool isthinClient, const char* poolName,
                        const char* locators, const char* serverGroup,
                        const std::shared_ptr<Properties>& configPtr = nullptr,
                        int redundancy = 0, bool clientNotification = false,
                        int subscriptionAckInterval = -1, int connections = -1,
                        int loadConditioningInterval = -1,
                        bool prSingleHop = false, bool threadLocal = false) {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(
        isthinClient, poolName, locators, serverGroup, configPtr, redundancy,
        clientNotification, subscriptionAckInterval, connections,
        loadConditioningInterval, false, prSingleHop, threadLocal);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

/* For HA Clients */
void initClient(int redundancyLevel,
                const std::shared_ptr<Properties>& configPtr = nullptr) {
  if (cacheHelper == nullptr) {
    auto config = configPtr;
    if (config == nullptr) {
      config = Properties::create();
    }
    cacheHelper = new CacheHelper(redundancyLevel, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void initGridClient(int redundancyLevel,
                    const std::shared_ptr<Properties>& configPtr = nullptr) {
  static bool s_isGridClient = true;

  s_isGridClient = !s_isGridClient;
  if (cacheHelper == nullptr) {
    auto config = configPtr;
    if (config == nullptr) {
      config = Properties::create();
    }
    config->insert("grid-client", s_isGridClient ? "true" : "false");
    cacheHelper = new CacheHelper(redundancyLevel, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void netDown() {
  if (cacheHelper != nullptr) {
    TestUtils::getCacheImpl(cacheHelper->cachePtr)->netDown();
  }
}

void revive() {
  if (cacheHelper != nullptr) {
    TestUtils::getCacheImpl(cacheHelper->cachePtr)->revive();
  }
}

void crashClient() {
  if (cacheHelper != nullptr) {
    TestUtils::getCacheImpl(cacheHelper->cachePtr)->setClientCrashTEST();
  }
}

const char* testregex[] = {"Key-*1", "Key-*2", "Key-*3",
                           "Key-*4", "Key-*5", "Key-*6"};
const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4", "Key-5", "Key-6"};
const char* vals[] = {"Value-1", "Value-2", "Value-3",
                      "Value-4", "Value-5", "Value-6"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4", "New Value-5", "New Value-6"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

void createRegion(const char* name, bool ackMode,
                  bool clientNotificationEnabled = false,
                  const std::shared_ptr<CacheListener>& listener = nullptr,
                  bool caching = true) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  // ack, caching
  auto regPtr = getHelper()->createRegion(name, ackMode, caching, listener,
                                          clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}
std::shared_ptr<Region> createOverflowRegion(const char* name, bool,
                                             int lel = 0, bool caching = true) {
  std::string bdb_dir = "BDB";
  std::string bdb_dirEnv = "BDBEnv";
  RegionAttributesFactory regionAttributesFactory;
  regionAttributesFactory.setCachingEnabled(caching);
  regionAttributesFactory.setLruEntriesLimit(lel);
  regionAttributesFactory.setDiskPolicy(DiskPolicyType::OVERFLOWS);

  auto sqLiteProps = Properties::create();
  sqLiteProps->insert("PageSize", "65536");
  sqLiteProps->insert("MaxPageCount", "1073741823");
  std::string sqlite_dir =
      "SqLiteRegionData" +
      std::to_string(static_cast<long long int>(ACE_OS::getpid()));
  sqLiteProps->insert("PersistenceDirectory", sqlite_dir.c_str());
  regionAttributesFactory.setPersistenceManager(
      "SqLiteImpl", "createSqLiteInstance", sqLiteProps);

  auto regionAttributes = regionAttributesFactory.create();
  auto cache = getHelper()->cachePtr;
  CacheImpl* cacheImpl = CacheRegionHelper::getCacheImpl(cache.get());
  std::shared_ptr<Region> regionPtr;
  cacheImpl->createRegion(name, regionAttributes, regionPtr);
  return regionPtr;
}

 std::shared_ptr<Pool> findPool(const char* poolName) {
   LOG("findPool() entered.");
   auto poolPtr = getHelper()->getCache()->getPoolManager().find(poolName);
   ASSERT(poolPtr != nullptr, "Failed to find pool.");
   return poolPtr;
 }
 std::shared_ptr<Pool> createPool(
     const char* poolName, const char* locators, const char* serverGroup,
     int redundancy = 0, bool clientNotification = false,
     std::chrono::milliseconds subscriptionAckInterval =
         std::chrono::milliseconds::zero(),
     int connections = -1, int loadConditioningInterval = -1) {
   LOG("createPool() entered.");

   auto poolPtr = getHelper()->createPool(
       poolName, locators, serverGroup, redundancy, clientNotification,
       subscriptionAckInterval, connections, loadConditioningInterval);
   ASSERT(poolPtr != nullptr, "Failed to create pool.");
   LOG("Pool created.");
   return poolPtr;
 }
 std::shared_ptr<Pool> createPoolAndDestroy(
     const char* poolName, const char* locators, const char* serverGroup,
     int redundancy = 0, bool clientNotification = false,
     std::chrono::milliseconds subscriptionAckInterval =
         std::chrono::milliseconds::zero(),
     int connections = -1) {
   LOG("createPoolAndDestroy() entered.");

   auto poolPtr = getHelper()->createPool(poolName, locators, serverGroup,
                                          redundancy, clientNotification,
                                          subscriptionAckInterval, connections);
   ASSERT(poolPtr != nullptr, "Failed to create pool.");
   poolPtr->destroy();
   LOG("Pool created and destroyed.");
   return poolPtr;
}
// this will create pool even endpoints and locatorhost has been not defined
std::shared_ptr<Pool> createPool2(const char* poolName, const char* locators,
                                  const char* serverGroup,
                                  const char* servers = nullptr,
                                  int redundancy = 0,
                                  bool clientNotification = false) {
  LOG("createPool2() entered.");

  auto poolPtr = getHelper()->createPool2(
      poolName, locators, serverGroup, servers, redundancy, clientNotification);
  ASSERT(poolPtr != nullptr, "Failed to create pool.");
  LOG("Pool created.");
  return poolPtr;
}
std::shared_ptr<Region> createRegionAndAttachPool(
    const std::string& name, bool ack, const std::string& poolName = "",
    bool caching = true,
    const std::chrono::seconds& ettl = std::chrono::seconds::zero(),
    const std::chrono::seconds& eit = std::chrono::seconds::zero(),
    const std::chrono::seconds& rttl = std::chrono::seconds::zero(),
    const std::chrono::seconds& rit = std::chrono::seconds::zero(), int lel = 0,
    ExpirationAction action = ExpirationAction::DESTROY) {
  LOG("createRegionAndAttachPool() entered.");
  auto regPtr = getHelper()->createRegionAndAttachPool(
      name, ack, poolName, caching, ettl, eit, rttl, rit, lel, action);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
  return regPtr;
}

void createEntry(const std::string& name, const char* key, const char* value) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %s in region %s\n", key,
          value, name.c_str());
  fflush(stdout);
  // Create entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  // regPtr->create( keyPtr, valPtr );
  regPtr->put(keyPtr, valPtr);
  LOG("Created entry.");

  verifyEntry(name.c_str(), key, value);
  LOG("Entry created.");
}

void createIntEntry(const char* name, const char* key, const int value,
                    bool onlyCreate = false) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %d in region %s\n", key,
          value, name);
  fflush(stdout);

  // Create entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableInt32::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  if (onlyCreate) {
    ASSERT(!regPtr->containsKey(keyPtr),
           "Key should not have been found in region.");
    ASSERT(!regPtr->containsValueForKey(keyPtr),
           "Value should not have been found in region.");
  }

  regPtr->put(keyPtr, valPtr);
  LOG("Created entry.");

  verifyIntEntry(name, key, value);
  LOG("Entry created.");
}

void invalidateEntry(const char* name, const char* key) {
  LOG("invalidateEntry() entered.");
  fprintf(stdout, "Invalidating entry -- key: %s  in region %s\n", key, name);
  fflush(stdout);
  // Invalidate entry, verify entry is invalidated
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(regPtr->containsKey(keyPtr), "Key should have been found in region.");
  ASSERT(regPtr->containsValueForKey(keyPtr),
         "Value should have been found in region.");

  regPtr->localInvalidate(keyPtr);
  LOG("Invalidate entry.");

  verifyInvalid(name, key);
  LOG("Entry invalidated.");
}

void destroyEntry(const char* name, const char* key) {
  LOG("destroyEntry() entered.");
  fprintf(stdout, "Destroying entry -- key: %s  in region %s\n", key, name);
  fflush(stdout);
  // Destroy entry, verify entry is destroyed
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(regPtr->containsKey(keyPtr), "Key should have been found in region.");

  regPtr->destroy(keyPtr);
  LOG("Destroy entry.");

  verifyDestroyed(name, key);
  LOG("Entry destroyed.");
}

void destroyRegion(const std::string& name) {
  LOG("destroyRegion() entered.");
  auto regPtr = getHelper()->getRegion(name);
  regPtr->localDestroyRegion();
  LOG("Region destroyed.");
}

class RegionOperations {
 public:
  explicit RegionOperations(const char* name)
      : m_regionPtr(getHelper()->getRegion(name)) {}

  void putOp(int keys = 1,
             const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) {
    char keybuf[100];
    char valbuf[100];
    for (int i = 1; i <= keys; i++) {
      sprintf(keybuf, "key%d", i);
      sprintf(valbuf, "value%d", i);
      auto valPtr = CacheableString::create(valbuf);
      m_regionPtr->put(keybuf, valPtr, aCallbackArgument);
    }
  }
  void invalidateOp(
      int keys = 1,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) {
    char keybuf[100];
    char valbuf[100];
    for (int i = 1; i <= keys; i++) {
      sprintf(keybuf, "key%d", i);
      auto valPtr = CacheableString::create(valbuf);
      m_regionPtr->localInvalidate(keybuf, aCallbackArgument);
    }
  }
  void destroyOp(
      int keys = 1,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) {
    char keybuf[100];
    char valbuf[100];
    for (int i = 1; i <= keys; i++) {
      sprintf(keybuf, "key%d", i);
      auto valPtr = CacheableString::create(valbuf);
      m_regionPtr->destroy(keybuf, aCallbackArgument);
    }
  }
  void removeOp(
      int keys = 1,
      const std::shared_ptr<Serializable>& aCallbackArgument = nullptr) {
    char keybuf[100];
    char valbuf[100];
    for (int i = 1; i <= keys; i++) {
      sprintf(keybuf, "key%d", i);
      sprintf(valbuf, "value%d", i);
      auto valPtr = CacheableString::create(valbuf);
      m_regionPtr->remove(keybuf, valPtr, aCallbackArgument);
    }
  }
  std::shared_ptr<Region> m_regionPtr;
};

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTHELPER_H_
