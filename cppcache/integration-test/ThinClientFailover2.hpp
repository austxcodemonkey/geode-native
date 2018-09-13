#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTFAILOVER2_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTFAILOVER2_H_

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
#include <ace/High_Res_Timer.h>

#include <ace/OS.h>
#include <string>

#define ThinClientFailover2 "DistOps"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "VerifyMacros.hpp"
#include "CacheHelper.hpp"

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;

/*
 * This test for client failover with client notification.
 */
bool isLocalServer = false;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2
static bool isLocator = false;
static int numberOfLocators = 0;
const char* locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);
#include "LocatorHelper.hpp"

void createRegion(const char* name, bool ackMode, const char* endpoints,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  // ack, caching
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          endpoints, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
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
  // ASSERT( regPtr->containsValueForKey( keyPtr ), "Value should have been
  // found in region." );

  regPtr->invalidate(keyPtr);
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

void destroyRegion(const char* name) {
  LOG("destroyRegion() entered.");
  auto regPtr = getHelper()->getRegion(name);
  regPtr->destroyRegion();
  LOG("Region destroyed.");
}

const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4", "Key-5"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4", "Value-5"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4", "New Value-5"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1);
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pool_Locator)
  {
    char buff[2048];
    char* buf = getcwd(buff, 2048);
    LOG(buf);
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TEST_POOL1__",
                       true);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->registerAllKeys(false, false, false);
    regPtr = getHelper()->getRegion(regionNames[1]);
    regPtr->registerAllKeys(false, false, false);
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pool_Locator)
  {
    char buff[2048];
    char* buf = getcwd(buff, 2048);
    LOG(buf);
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TEST_POOL1__",
                       true);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->registerAllKeys(false, false, false);
    regPtr = getHelper()->getRegion(regionNames[1]);
    regPtr->registerAllKeys(false, false, false);
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[0], keys[4], vals[4]);
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    doNetSearch(regionNames[0], keys[0], vals[0]);
    doNetSearch(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[1], keys[3], vals[3]);
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) CacheHelper::initServer(2);
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);
    auto reg1 = getHelper()->getRegion(regionNames[1]);
    auto vec0 = reg0->serverKeys();
    auto vec1 = reg1->serverKeys();
    ASSERT(vec0.size() == 3, "Should have 3 keys in first region.");
    ASSERT(vec1.size() == 2, "Should have 2 keys in second region.");

    std::string key0, key1, key2;
    key0 = vec0[0]->toString().c_str();
    key1 = vec0[1]->toString().c_str();
    key2 = vec0[2]->toString().c_str();
    ASSERT(key0 != key1, "The two keys should be different in first region.");
    ASSERT(key0 != key2, "The two keys should be different in first region.");
    ASSERT(key1 != key2, "The two keys should be different in first region.");
    ASSERT(key0 == keys[0] || key0 == keys[1] || key0 == keys[4],
           "Unexpected key in first region.");
    ASSERT(key1 == keys[0] || key1 == keys[1] || key1 == keys[4],
           "Unexpected key in first region.");
    ASSERT(key2 == keys[0] || key2 == keys[1] || key2 == keys[4],
           "Unexpected key in first region.");

    key0 = vec1[0]->toString().c_str();
    key1 = vec1[1]->toString().c_str();
    ASSERT(key0 != key1, "The two keys should be different in second region.");
    ASSERT(key0 == keys[2] || key0 == keys[3],
           "Unexpected key in second region.");
    ASSERT(key1 == keys[2] || key1 == keys[3],
           "Unexpected key in second region.");

    doNetSearch(regionNames[0], keys[1], vals[1]);
    doNetSearch(regionNames[1], keys[3], vals[3]);
    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    LOG("StepFive complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
  {
    // verifyInvalid( regionNames[0], keys[0] );
    // verifyInvalid( regionNames[1], keys[2] );
    // This step get the value from region, if key is inavalidate in region,
    // value
    // will get from server.
    // doNetSearch( regionNames[0], keys[4], vals[4] );
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[1], keys[3], nvals[3]);
    LOG("StepSix complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    verifyEntry(regionNames[0], keys[1], nvals[1]);
    verifyEntry(regionNames[1], keys[3], nvals[3]);
    LOG("StepSeven complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, StepEight)
  {
    destroyEntry(regionNames[0], keys[0]);
    destroyEntry(regionNames[1], keys[2]);
    LOG("StepEight complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepNine)
  {
    verifyDestroyed(regionNames[0], keys[0]);
    verifyDestroyed(regionNames[1], keys[2]);
    destroyEntry(regionNames[0], keys[1]);
    destroyEntry(regionNames[1], keys[3]);
    LOG("StepNine complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, StepTen)
  {
    verifyDestroyed(regionNames[0], keys[1]);
    verifyDestroyed(regionNames[1], keys[3]);
    LOG("StepTen complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(SERVER2, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
  }
END_TASK_DEFINITION

void runThinClientFailover2() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator)

  CALL_TASK(StepOne_Pool_Locator);
  CALL_TASK(StepTwo_Pool_Locator);

  CALL_TASK(StepThree);
  CALL_TASK(StepFour);

  CALL_TASK(CreateServer2_With_Locator);

  CALL_TASK(CloseServer1);  // FailOver
  CALL_TASK(StepFive);
  CALL_TASK(StepSix);
  CALL_TASK(StepSeven);
  CALL_TASK(StepEight);
  CALL_TASK(StepNine);
  CALL_TASK(StepTen);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer2);

  CALL_TASK(CloseLocator1);
}

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTFAILOVER2_H_
