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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTFAILOVERREGEX_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTFAILOVERREGEX_H_

#include "fw_dunit.hpp"
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <string>

#define ROOT_NAME "ThinClientFailoverRegex"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "VerifyMacros.hpp"
#include "CacheHelper.hpp"

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;

bool isLocalServer = false;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2
static bool isLocator = false;
// static int numberOfLocators = 0;
const char* locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
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

const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* regkeys[] = {"Key-*1", "Key-*2", "Key-*3", "Key-*4"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pool_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TEST_POOL1__",
                       true);

    createEntry(regionNames[1], keys[1], vals[1]);

    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pool_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TEST_POOL1__",
                       true);

    createEntry(regionNames[1], keys[1], nvals[1]);

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);

    /*
     auto keyPtr0 = CacheableKey::create(keys[0]);
     auto keyPtr2 = CacheableKey::create(keys[2]);

      std::vector<std::shared_ptr<CacheableKey>>  keys0,keys1;
      keys0.push_back(keyPtr0);
      keys1.push_back(keyPtr2);
      regPtr0->registerKeys(keys0, nullptr);
      regPtr1->registerKeys(keys1, nullptr);
    */

    regPtr0->registerRegex(regkeys[0]);
    regPtr1->registerRegex(regkeys[2]);

    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[1], keys[2], vals[2]);

    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    regPtr1->registerRegex(regkeys[1], false);

    doNetSearch(
        regionNames[1], keys[1],
        nvals[1]);  // this should verify due to registerRegex() call before

    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    LOG("StepFour start.");

    verifyCreated(regionNames[0], keys[0]);
    verifyCreated(regionNames[1], keys[2]);
    verifyEntry(regionNames[0], keys[0], vals[0]);
    verifyEntry(regionNames[1], keys[2], vals[2]);

    updateEntry(regionNames[1], keys[1], vals[1]);

    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    regPtr1->unregisterRegex(regkeys[2]);

    /*
     auto regPtr1 = getHelper()->getRegion( regionNames[1] );
     auto keyPtr2 = CacheableKey::create(keys[2]);
      std::vector<std::shared_ptr<CacheableKey>>  keys2;
      keys2.push_back(keyPtr2);
      regPtr1->unregisterKeys(keys2, nullptr);
      */

    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver_notify_subscription2.xml");
    }
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
    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[2], nvals[2]);

    verifyEntry(
        regionNames[1], keys[1],
        vals[1]);  // this should verify due to registerRegex() call before
    SLEEP(1000);

    LOG("StepFive complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
  {
    LOG("StepSix start.");
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[1], keys[2],
                vals[2]);  // this key-regex was unregistered before failover

    updateEntry(regionNames[1], keys[1], nvals[1]);  // for client1 to verify

    LOG("StepSix complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    LOG("StepSeven start.");

    updateEntry(regionNames[0], keys[0], vals[0]);
    updateEntry(regionNames[1], keys[2], vals[2]);

    verifyEntry(regionNames[1], keys[1], nvals[1]);

    LOG("StepSeven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepEight)
  {
    LOG("StepEight start.");
    verifyEntry(regionNames[0], keys[0],
                vals[0]);  // this should verify now (registered after failover)
    verifyEntry(
        regionNames[1], keys[2],
        vals[2]);  // this should verify now (not registered after failover)
    LOG("StepEight complete.");
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

void runThinClientFailOverRegex() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator_XML)

  CALL_TASK(StepOne_Pool_Locator);
  CALL_TASK(StepTwo_Pool_Locator);

  CALL_TASK(StepThree);
  CALL_TASK(StepFour);

  CALL_TASK(CreateServer2_With_Locator_XML);

  CALL_TASK(CloseServer1);
  CALL_TASK(StepFive);
  CALL_TASK(StepSix);
  CALL_TASK(StepSeven);
  CALL_TASK(StepEight);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer2);

  CALL_TASK(CloseLocator1);
}

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTFAILOVERREGEX_H_
