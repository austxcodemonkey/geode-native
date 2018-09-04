#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTNOTIFICATION_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTNOTIFICATION_H_

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

#define ROOT_NAME "ThinClientNotification"
#define ROOT_SCOPE DISTRIBUTED_ACK

#define CacheHelperOwner
#include "VerifyMacros.hpp"
#include "CacheHelper.hpp"

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;

#include "locator_globals.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#include "LocatorHelper.hpp"

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

void destroyRegion(const char* name) {
  LOG("destroyRegion() entered.");
  auto regPtr = getHelper()->getRegion(name);
  regPtr->localDestroyRegion();
  LOG("Region destroyed.");
}

const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1)
  { initClient(true); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateClient2)
  { initClient(true); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateRegions1_PoolLocators)
  {
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->registerAllKeys(false, false, false);
    regPtr = getHelper()->getRegion(regionNames[1]);
    regPtr->registerAllKeys(false, false, false);
    LOG("CreateRegions1_PoolLocators complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CreateRegions2_PoolLocators)
  {
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    auto regPtr = getHelper()->getRegion(regionNames[0]);
    regPtr->registerAllKeys(false, false, false);
    regPtr = getHelper()->getRegion(regionNames[1]);
    regPtr->registerAllKeys(false, false, false);
    LOG("CreateRegions2_PoolLocators complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateEvenValues)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[1], keys[2], vals[2]);
    LOG("CreateEvenValues complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, FindEvenAndCreateOddValues)
  {
    doNetsearch(regionNames[0], keys[0], vals[0]);
    doNetsearch(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[1], keys[3], vals[3]);
    LOG("FindEvenAndCreateOddValues complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, FindOddAndUpdateEvenValues)
  {
    doNetsearch(regionNames[0], keys[1], vals[1]);
    doNetsearch(regionNames[1], keys[3], vals[3]);
    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    LOG("FindOddAndUpdateEvenValues complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyEvenAndUpdateOddValues)
  {
    LOG("VerifyEvenAndUpdateOddValues started");
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[1], keys[3], nvals[3]);
    LOG("VerifyEvenAndUpdateOddValues complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyOddExistenceAndValuesAndInvalidateEven)
  {
    verifyInvalid(regionNames[0], keys[1]);
    verifyInvalid(regionNames[1], keys[3]);
    verifyEntry(regionNames[0], keys[1], nvals[1]);
    verifyEntry(regionNames[1], keys[3], nvals[3]);
    invalidateEntry(regionNames[0], keys[0]);
    invalidateEntry(regionNames[1], keys[2]);
    LOG("VerifyFirstTwoExistenceAndValuesAndInvalidate complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyEvenValuesAndInvalidateOdd)
  {
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    invalidateEntry(regionNames[0], keys[1]);
    invalidateEntry(regionNames[1], keys[3]);
    LOG("VerifyEvenValuesAndInvalidateOdd complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyOddAndDestroyEven)
  {
    verifyEntry(regionNames[0], keys[1], nvals[1]);
    verifyEntry(regionNames[1], keys[3], nvals[3]);
    destroyEntry(regionNames[0], keys[0]);
    destroyEntry(regionNames[1], keys[2]);
    LOG("VerifyOddAndDestroyEven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, VerifyEvenDestructionAndDestroyOdd)
  {
    verifyDestroyed(regionNames[0], keys[0]);
    verifyDestroyed(regionNames[1], keys[2]);
    destroyEntry(regionNames[0], keys[1]);
    destroyEntry(regionNames[1], keys[3]);
    LOG("VerifyEvenDestructionAndDestroyOdd complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, VerifyOddDestructionAndDestroyRegions)
  {
    verifyDestroyed(regionNames[0], keys[1]);
    verifyDestroyed(regionNames[1], keys[3]);
    destroyRegion(regionNames[0]);
    destroyRegion(regionNames[1]);
    LOG("VerifyOddDestructionAndDestroyRegions complete.");
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

void doThinClientNotification() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator);

  CALL_TASK(CreateClient1);
  CALL_TASK(CreateClient2);

  CALL_TASK(CreateRegions1_PoolLocators);
  CALL_TASK(CreateRegions2_PoolLocators);

  CALL_TASK(CreateEvenValues);
  CALL_TASK(FindEvenAndCreateOddValues);
  CALL_TASK(FindOddAndUpdateEvenValues);
  CALL_TASK(VerifyEvenAndUpdateOddValues);
  CALL_TASK(VerifyOddExistenceAndValuesAndInvalidateEven);
  CALL_TASK(VerifyEvenValuesAndInvalidateOdd);
  CALL_TASK(VerifyOddAndDestroyEven);
  CALL_TASK(VerifyEvenDestructionAndDestroyOdd);
  CALL_TASK(VerifyOddDestructionAndDestroyRegions);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);
  CALL_TASK(CloseLocator1);
}

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTNOTIFICATION_H_
