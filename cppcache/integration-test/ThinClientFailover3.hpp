#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTFAILOVER3_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTFAILOVER3_H_

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
#include <vector>

#define ROOT_NAME "ThinClientFailover3"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;

bool isLocalServer = false;
const char *endPoints = CacheHelper::getTcrEndpoints(isLocalServer, 3);

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2
static bool isLocator = false;
// static int numberOfLocators = 0;
const char *locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
#include "LocatorHelper.hpp"


std::vector<char *> storeEndPoints(const char *points) {
  std::vector<char *> endpointNames;
  if (points != nullptr) {
    char *ep = strdup(points);
    char *token = strtok(ep, ",");
    while (token) {
      endpointNames.push_back(token);
      token = strtok(nullptr, ",");
    }
  }
  ASSERT(endpointNames.size() == 3, "There should be 3 end points");
  return endpointNames;
}

std::vector<char *> endpointNames = storeEndPoints(endPoints);
const char *keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char *vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char *nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char *regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1);
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetupClient1PooledLocator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__");
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TEST_POOL1__");
    LOG("SetupClient1PooledLocator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetupClient2PooledLocator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__");
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TEST_POOL1__");
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Client1CreateEntries)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[1], keys[2], vals[2]);
    LOG("Client1CreateEntries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Client2CreateEntries)
  {
    doNetSearch(regionNames[0], keys[0], vals[0]);
    doNetSearch(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[1], keys[3], vals[3]);
    LOG("Client2CreateEntries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2and3)
  {
    if (isLocalServer) CacheHelper::initServer(2);
    LOG("SERVER2 started");
    if (isLocalServer) CacheHelper::initServer(3);
    LOG("SERVER3 started");
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
DUNIT_TASK_DEFINITION(CLIENT1, Client1UpdateEntries)
  {
    doNetSearch(regionNames[0], keys[1], vals[1]);
    doNetSearch(regionNames[1], keys[3], vals[3]);
    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    LOG("Client1UpdateEntries complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, Client2UpdateEntries)
  {
    doNetSearch(regionNames[0], keys[0], vals[0]);
    doNetSearch(regionNames[1], keys[2], vals[2]);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[1], keys[3], nvals[3]);
    LOG("Client2UpdateEntries complete.");
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

DUNIT_TASK_DEFINITION(SERVER2, CloseServer2and3)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
    if (isLocalServer) {
      CacheHelper::closeServer(3);
      LOG("SERVER3 stopped");
    }
  }
END_TASK_DEFINITION

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTFAILOVER3_H_
