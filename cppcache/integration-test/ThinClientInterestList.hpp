#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTINTERESTLIST_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTINTERESTLIST_H_

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
#define ROOT_NAME "ThinClientInterestList.hpp"

#include "fw_dunit.hpp"
#define CacheHelperOwner
#include "VerifyMacros.hpp"

#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <string>

#include "CacheHelper.hpp"


#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;

bool isLocalServer = false;

static bool isLocator = false;
static int numberOfLocators = 1;
const char* locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);
#include "LocatorHelper.hpp"

const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pool_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pool_Locator)
  {
    initClient(true);
    // Client2 has notifications enabled.
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    LOG("StepTwo_Pool complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[1], keys[2], vals[2]);
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);

    auto keyPtr1 = CacheableKey::create(keys[1]);
    auto keyPtr3 = CacheableKey::create(keys[3]);

    std::vector<std::shared_ptr<CacheableKey>> keys0, keys1;
    keys0.push_back(keyPtr1);
    keys1.push_back(keyPtr3);
    regPtr0->registerKeys(keys0);
    regPtr1->registerKeys(keys1);

    //  createEntry( regionNames[0], keys[1] );
    //  createEntry( regionNames[1], keys[3] );

    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    doNetSearch(regionNames[0], keys[0], vals[0]);
    doNetSearch(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[1], keys[3], vals[3]);
    auto keyPtr0 = CacheableKey::create(keys[0]);
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    std::vector<std::shared_ptr<CacheableKey>> keys0;
    keys0.push_back(keyPtr0);
    regPtr0->registerKeys(keys0);
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    verifyCreated(regionNames[0], keys[1]);
    verifyCreated(regionNames[1], keys[3]);
    verifyEntry(regionNames[0], keys[1], vals[1]);
    verifyEntry(regionNames[1], keys[3], vals[3]);

    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[2], nvals[2]);

    auto keyPtr3 = CacheableKey::create(keys[3]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    std::vector<std::shared_ptr<CacheableKey>> keys1;
    keys1.push_back(keyPtr3);
    regPtr1->unregisterKeys(keys1);
    LOG("StepFive complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
  {
    // verifyCreated( regionNames[0], keys[0]);
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[1], keys[2], vals[2]);
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    auto keyPtr1 = CacheableKey::create(keys[1]);
    auto keyPtr3 = CacheableKey::create(keys[3]);
    regPtr0->destroy(keyPtr1);
    regPtr1->destroy(keyPtr3);
    LOG("StepSix complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    verifyDestroyed(regionNames[0], keys[1]);
    // verifyDestroyed( regionNames[1], keys[3]);
    verifyEntry(regionNames[1], keys[3], vals[3]);
    LOG("StepSeven complete.");
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

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTINTERESTLIST_H_
