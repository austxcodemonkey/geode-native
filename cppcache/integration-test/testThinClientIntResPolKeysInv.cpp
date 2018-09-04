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

#define ROOT_NAME "testThinClientIntResPolKeysInv"

#define CacheHelperOwner
#include "VerifyMacros.hpp"
#include "CacheHelper.hpp"


#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;


#include "locator_globals.hpp"
#include "LocatorHelper.hpp"

void createRegion(const char *name, bool ackMode, const char *endpoints,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          endpoints, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}

const char *regex23 = "Key-[2-3]";
const char *regexWildcard = "Key-.*";
const char *keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char *regionNames[] = {"DistRegionAck", "DistRegionNoAck"};
const char *vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char *nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(CLIENT1, StepOne)
  {
    initClient(true);
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    getHelper()->createPooledRegion(regionNames[1], NO_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo)
  {
    initClient(true);
    getHelper()->createPooledRegion(regionNames[0], USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    getHelper()->createPooledRegion(regionNames[1], NO_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    createEntry(regionNames[1], keys[0], vals[0]);
    createEntry(regionNames[1], keys[1], vals[1]);
    createEntry(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[1], keys[3], vals[3]);
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[0], keys[2], vals[2]);
    createEntry(regionNames[0], keys[3], vals[3]);
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFive)
  {
    doNetsearch(regionNames[1], keys[0], vals[0]);
    doNetsearch(regionNames[1], keys[1], vals[1]);
    doNetsearch(regionNames[1], keys[2], vals[2]);
    doNetsearch(regionNames[1], keys[3], vals[3]);
    LOG("StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSix)
  {
    doNetsearch(regionNames[0], keys[0], vals[0]);
    doNetsearch(regionNames[0], keys[1], vals[1]);
    doNetsearch(regionNames[0], keys[2], vals[2]);
    doNetsearch(regionNames[0], keys[3], vals[3]);
    LOG("StepSix complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->registerRegex(regex23);

    verifyInvalid(regionNames[0], keys[1]);
    verifyInvalid(regionNames[0], keys[2]);

    verifyEntry(regionNames[0], keys[0], vals[0]);
    verifyEntry(regionNames[0], keys[3], vals[3]);

    LOG("StepSeven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepEight)
  {
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    // regPtr1->registerRegex(regexWildcard);
    regPtr1->registerAllKeys();

    verifyInvalid(regionNames[1], keys[0]);
    verifyInvalid(regionNames[1], keys[1]);
    verifyInvalid(regionNames[1], keys[2]);
    verifyInvalid(regionNames[1], keys[3]);

    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[0], keys[2], nvals[2]);
    updateEntry(regionNames[0], keys[3], nvals[3]);

    LOG("StepEight complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepNine)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->unregisterRegex(regex23);

    auto keyptr0 = CacheableKey::create(keys[0]);
    auto keyptr1 = CacheableKey::create(keys[1]);
    auto keyptr2 = CacheableKey::create(keys[2]);
    std::vector<std::shared_ptr<CacheableKey>> keylist;
    keylist.push_back(keyptr0);
    keylist.push_back(keyptr1);
    keylist.push_back(keyptr2);

    regPtr0->registerKeys(keylist);

    verifyInvalid(regionNames[0], keys[0]);
    verifyInvalid(regionNames[0], keys[1]);
    verifyInvalid(regionNames[0], keys[2]);

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

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator);
    CALL_TASK(StepOne);
    CALL_TASK(StepTwo);
    CALL_TASK(StepThree);
    CALL_TASK(StepFour);
    CALL_TASK(StepFive);
    CALL_TASK(StepSix);
    CALL_TASK(StepSeven);
    CALL_TASK(StepEight);
    CALL_TASK(StepNine);
    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);
    CALL_TASK(CloseServer1);
    CALL_TASK(CloseLocator1);
  }
END_MAIN
