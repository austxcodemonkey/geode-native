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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTREGEX2_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTREGEX2_H_

#include "fw_dunit.hpp"
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <string>

#define ROOT_NAME "ThinClientRegex2"
#define ROOT_SCOPE DISTRIBUTED_ACK

#define CacheHelperOwner
#include "VerifyMacros.hpp"
#include "CacheHelper.hpp"


#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;

bool isLocalServer = false;

static bool isLocator = false;
static int numberOfLocators = 0;
const char* locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);
#include "LocatorHelper.hpp"

void createRegion(const char* name, bool ackMode,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

const char* regex23 = "Key-[2-3]";
const char* regexWildcard = "Key-.*";
const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pool_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    LOG("StepOne_Pool complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pool_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    LOG("StepTwo_Pool complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->registerRegex(regex23);
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    regPtr1->registerRegex(regexWildcard);
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    createEntry(regionNames[1], keys[0], vals[0]);
    createEntry(regionNames[1], keys[1], vals[1]);
    createEntry(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[1], keys[3], vals[3]);
    LOG("StepFive complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[0], keys[2], vals[2]);
    createEntry(regionNames[0], keys[3], vals[3]);

    SLEEP(1000);  // sleep for sometime to let updates come

    verifyEntry(regionNames[1], keys[0], vals[0]);
    verifyEntry(regionNames[1], keys[1], vals[1]);
    verifyEntry(regionNames[1], keys[2], vals[2]);
    verifyEntry(regionNames[1], keys[3], vals[3]);

    LOG("StepSix complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    SLEEP(1000);  // sleep for sometime to let updates come

    verifyEntry(regionNames[0], keys[1], vals[1]);
    verifyEntry(regionNames[0], keys[2], vals[2]);

    doNetSearch(regionNames[0], keys[0], vals[0]);
    doNetSearch(regionNames[0], keys[3], vals[3]);

    updateEntry(regionNames[1], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[1], nvals[1]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    updateEntry(regionNames[1], keys[3], nvals[3]);

    LOG("StepSeven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepEight)
  {
    SLEEP(1000);  // sleep for sometime to let updates come

    verifyEntry(regionNames[1], keys[0], nvals[0]);
    verifyEntry(regionNames[1], keys[1], nvals[1]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    verifyEntry(regionNames[1], keys[3], nvals[3]);

    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[0], keys[2], nvals[2]);
    updateEntry(regionNames[0], keys[3], nvals[3]);

    LOG("StepEight complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepNine)
  {
    SLEEP(1000);  // sleep for sometime to let updates come

    verifyEntry(regionNames[0], keys[0], vals[0]);
    verifyEntry(regionNames[0], keys[1], nvals[1]);
    verifyEntry(regionNames[0], keys[2], nvals[2]);
    verifyEntry(regionNames[0], keys[3], vals[3]);

    LOG("StepNine complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
    LOG("cleanProc 1...");
    cleanProc();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  {
    LOG("cleanProc 2...");
    cleanProc();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    LOG("closing Server1...");
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTREGEX2_H_
