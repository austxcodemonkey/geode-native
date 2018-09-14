#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTTXFAILOVER_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTTXFAILOVER_H_

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
#include <geode/CacheTransactionManager.hpp>

#define ROOT_NAME "ThinClientTXFailover"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::TransactionDataNodeHasDepartedException;

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

void createPooledRegionSticky(const char* name, bool ackMode,
                              const char* locators, const char* poolname,
                              bool clientNotificationEnabled = false,
                              bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  auto regPtr = getHelper()->createPooledRegionSticky(
      name, ackMode, locators, poolname, cachingEnable,
      clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}

const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1);
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__");
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TEST_POOL2__");
    LOG("StepOne_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator_Sticky)
  {
    initClient(true);
    createPooledRegionSticky(regionNames[0], USE_ACK, locatorsG,
                             "__TEST_POOL1__");
    createPooledRegionSticky(regionNames[1], NO_ACK, locatorsG,
                             "__TEST_POOL2__");
    LOG("StepOne_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pooled_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__");
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TEST_POOL2__");
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pooled_Locator_Sticky)
  {
    initClient(true);
    createPooledRegionSticky(regionNames[0], USE_ACK, locatorsG,
                             "__TEST_POOL1__");
    createPooledRegionSticky(regionNames[1], NO_ACK, locatorsG,
                             "__TEST_POOL2__");
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[1], keys[2], vals[2]);
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    getHelper()->getCache()->getCacheTransactionManager()->begin();
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
    // doNetSearch( regionNames[0], keys[1], vals[1] );
    // doNetSearch( regionNames[1], keys[3], vals[3] );
    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    LOG("StepFive complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
  {
    try {
      createEntry(regionNames[0], keys[1], vals[1]);
      FAIL("Expected exception when transaction delegate has gone away.");
    } catch (const TransactionDataNodeHasDepartedException& ex) {
      getHelper()->getCache()->getCacheTransactionManager()->rollback();
      printf(
          "Got expected TransactionDataNodeHasDepartedException when "
          "transaction "
          "delegate has gone away: %s\n",
          ex.what());
    }

    LOG("StepSix complete.");
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

void runThinClientFailover(bool isSticky = false) {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator)

  if (isSticky) {
    CALL_TASK(StepOne_Pooled_Locator_Sticky);
    CALL_TASK(StepTwo_Pooled_Locator_Sticky);
  } else {
    CALL_TASK(StepOne_Pooled_Locator);
    CALL_TASK(StepTwo_Pooled_Locator);
  }

  //  CALL_TASK( StepThree );
  CALL_TASK(StepFour);

  CALL_TASK(CreateServer2_With_Locator);

  CALL_TASK(CloseServer1);  // FailOver
  //  CALL_TASK( StepFive );
  CALL_TASK(StepSix);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer2);

  CALL_TASK(CloseLocator1);
}

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTTXFAILOVER_H_
