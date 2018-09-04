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
#include "fw_dunit.hpp"
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>

#include <string>

#define ROOT_NAME "testThinClientHADistOps"

#define CacheHelperOwner
#include "VerifyMacros.hpp"
#include "CacheHelper.hpp"


#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::Properties;

bool isLocalServer = false;

static bool isLocator = false;
const char* locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
#include "LocatorHelper.hpp"
static int clientWithRedundancy = 0;
void initClient(int redundancyLevel) {
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    if (clientWithRedundancy > 0) config->insert("grid-client", "true");
    clientWithRedundancy += 1;
    cacheHelper = new CacheHelper(redundancyLevel, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

static int clientWithNothing = 0;
void initClient() {
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    if (clientWithNothing > 1) config->insert("grid-client", "true");
    clientWithNothing += 1;
    cacheHelper = new CacheHelper(true, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

static int clientWithXml = 0;
void initClient(const char* clientXmlFile) {
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    if (clientWithXml > 2) config->insert("grid-client", "true");
    clientWithXml += 1;
    cacheHelper = new CacheHelper(nullptr, clientXmlFile, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
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

void createRegion(const char* name, bool ackMode,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  char* endpoints = nullptr;
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          endpoints, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

void doGetAgain(const char* name, const char* key, const char* value) {
  LOG("doGetAgain() entered.");
  fprintf(stdout,
          "get for entry -- key: %s  expecting value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  fprintf(stdout, "get  region name%s\n", regPtr->getName().c_str());
  fflush(stdout);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
      regPtr->get(keyPtr));  // force a netsearch

  if (checkPtr != nullptr) {
    LOG("checkPtr is not null");
    char buf[1024];
    sprintf(buf, "In doGetAgain, get returned %s for key %s",
            checkPtr->value().c_str(), key);
    LOG(buf);
  } else {
    LOG("checkPtr is nullptr");
  }
  verifyEntry(name, key, value);
  LOG("GetAgain complete.");
}

const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1)
  { initClient(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitClient2)
  { initClient(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1PoolAndRegions)
  {
    getHelper()->createPoolWithLocators("__TESTPOOL1_", locatorsG, true, 1);

    getHelper()->createRegionAndAttachPool(regionNames[0], USE_ACK,
                                           "__TESTPOOL1_", true);
    getHelper()->createRegionAndAttachPool(regionNames[1], NO_ACK,
                                           "__TESTPOOL1_", true);
    /* createPooledRegion( regionNames[0], USE_ACK, locatorsG,
     "__TESTPOOL1_" );
     createPooledRegion( regionNames[1], NO_ACK, locatorsG, "__TESTPOOL2_"
     );*/

    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[1], keys[2], vals[2]);

    LOG("CreateClient1PoolAndRegions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateClient2PoolAndRegions)
  {
    getHelper()->createPoolWithLocators("__TESTPOOL1_", locatorsG, true, 1);

    getHelper()->createRegionAndAttachPool(regionNames[0], USE_ACK,
                                           "__TESTPOOL1_", true);
    getHelper()->createRegionAndAttachPool(regionNames[1], NO_ACK,
                                           "__TESTPOOL1_", true);

    /* createPooledRegion( regionNames[0], USE_ACK,  locatorsG,
     "__TESTPOOL1_" );
     createPooledRegion( regionNames[1], NO_ACK,  locatorsG, "__TESTPOOL2_"
     );*/

    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[1], keys[3], vals[3]);

    LOG("CreateClient2PoolAndRegions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, RegisterClient1Keys)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);
    auto reg1 = getHelper()->getRegion(regionNames[1]);
    auto vec0 = reg0->serverKeys();
    auto vec1 = reg1->serverKeys();
    ASSERT(vec0.size() == 2, "Should have 2 keys in first region.");
    ASSERT(vec1.size() == 2, "Should have 2 keys in second region.");
    std::string key0, key1;
    key0 = vec0[0]->toString().c_str();
    key1 = vec0[1]->toString().c_str();
    ASSERT(key0 != key1, "The two keys should be different in first region.");
    ASSERT(key0 == keys[0] || key0 == keys[1],
           "Unexpected key in first region.");
    ASSERT(key1 == keys[0] || key1 == keys[1],
           "Unexpected key in first region.");

    key0 = vec1[0]->toString().c_str();
    key1 = vec1[1]->toString().c_str();
    ASSERT(key0 != key1, "The two keys should be different in second region.");
    ASSERT(key0 == keys[2] || key0 == keys[3],
           "Unexpected key in second region.");
    ASSERT(key1 == keys[2] || key1 == keys[3],
           "Unexpected key in second region.");

    doNetsearch(regionNames[0], keys[1], vals[1]);
    doNetsearch(regionNames[1], keys[3], vals[3]);

    auto keyPtr1 = CacheableKey::create(keys[1]);
    auto keyPtr3 = CacheableKey::create(keys[3]);

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);

    std::vector<std::shared_ptr<CacheableKey>> keys0, keys1;
    keys0.push_back(keyPtr1);
    keys1.push_back(keyPtr3);
    regPtr0->registerKeys(keys0);
    regPtr1->registerKeys(keys1);
    LOG("RegisterClient1Keys complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, RegisterClient2Keys)
  {
    doNetsearch(regionNames[0], keys[0], vals[0]);
    doNetsearch(regionNames[1], keys[2], vals[2]);

    auto keyPtr0 = CacheableKey::create(keys[0]);
    auto keyPtr2 = CacheableKey::create(keys[2]);

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);

    std::vector<std::shared_ptr<CacheableKey>> keys0, keys1;
    keys0.push_back(keyPtr0);
    keys1.push_back(keyPtr2);
    regPtr0->registerKeys(keys0);
    regPtr1->registerKeys(keys1);
    LOG("RegisterClient2Keys complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, UpdateClient1Entries)
  {
    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    LOG("UpdateClient1Entries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyAndUpdateClient2Entries)
  {
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);

    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[1], keys[3], nvals[3]);
    LOG("VerifyAndUpdateClient2Entries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyClient1Entries)
  {
    verifyEntry(regionNames[0], keys[1], nvals[1]);
    verifyEntry(regionNames[1], keys[3], nvals[3]);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, DoAbsolutelyNothing)
  {}
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, DestroyAllKeys)
  {
    destroyEntry(regionNames[0], keys[0]);
    destroyEntry(regionNames[0], keys[1]);
    destroyEntry(regionNames[1], keys[2]);
    destroyEntry(regionNames[1], keys[3]);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    /* Starting Server*/
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator_XML);
    CALL_TASK(CreateServer2_With_Locator_XML);

    /* Client init*/
    CALL_TASK(InitClient1);
    CALL_TASK(InitClient2);

    CALL_TASK(CreateClient1PoolAndRegions);
    CALL_TASK(CreateClient2PoolAndRegions);

    CALL_TASK(RegisterClient1Keys);
    CALL_TASK(RegisterClient2Keys);
    CALL_TASK(UpdateClient1Entries);
    CALL_TASK(VerifyAndUpdateClient2Entries);
    CALL_TASK(VerifyClient1Entries);
    CALL_TASK(DoAbsolutelyNothing);
    CALL_TASK(DestroyAllKeys);

    CALL_TASK(CloseServer1);
    CALL_TASK(CloseServer2);

    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);

    CALL_TASK(CloseLocator1);
  }
END_MAIN
