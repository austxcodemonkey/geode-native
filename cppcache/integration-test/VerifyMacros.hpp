#pragma once

#ifndef GEODE_INTEGRATION_TEST_VERIFY_MACROS_H_
#define GEODE_INTEGRATION_TEST_VERIFY_MACROS_H_

#include <geode/CacheableKey.hpp>

#include "CacheHelper.hpp"
#include "fw_dunit.hpp"

#ifndef CacheHelperOwner
extern apache::geode::client::CacheHelper* cacheHelper;
#else
apache::geode::client::CacheHelper* cacheHelper = nullptr;
bool g_isGridClient = false;
#endif

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

apache::geode::client::CacheHelper* getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

#define verifyEntry(x, y, z) _verifyEntry(x, y, z, __LINE__)

void _verifyEntry(const char* name, const char* key, const char* val,
                  bool noKey, bool isCreated = false) {
  // Verify key and value exist in this region, in this process.
  const char* value = val ? val : "";
  char* buf =
      reinterpret_cast<char*>(malloc(1024 + strlen(key) + strlen(value)));
  ASSERT(buf, "Unable to malloc buffer for logging.");
  if (!isCreated) {
    if (noKey) {
      sprintf(buf, "Verify key %s does not exist in region %s", key, name);
    } else if (!val) {
      sprintf(buf, "Verify value for key %s does not exist in region %s", key,
              name);
    } else {
      sprintf(buf, "Verify value for key %s is: %s in region %s", key, value,
              name);
    }
    LOG(buf);
  }
  free(buf);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto keyPtr = apache::geode::client::CacheableKey::create(key);

  // if the region is no ack, then we may need to wait...
  if (!isCreated) {
    if (noKey == false) {  // need to find the key!
      ASSERT(regPtr->containsKey(keyPtr), "Key not found in region.");
    }
    if (val != nullptr) {  // need to have a value!
      ASSERT(regPtr->containsValueForKey(keyPtr), "Value not found in region.");
    }
  }

  // loop up to MAX times, testing condition
  uint32_t MAX = 100;
  //  changed sleep from 10 ms
  uint32_t SLEEP = 1000;  // milliseconds
  uint32_t containsKeyCnt = 0;
  uint32_t containsValueCnt = 0;
  uint32_t testValueCnt = 0;

  for (int i = MAX; i >= 0; i--) {
    if (isCreated) {
      if (!regPtr->containsKey(keyPtr)) {
        containsKeyCnt++;
      } else {
        break;
      }
      ASSERT(containsKeyCnt < MAX, "Key has not been created in region.");
    } else {
      if (noKey) {
        if (regPtr->containsKey(keyPtr)) {
          containsKeyCnt++;
        } else {
          break;
        }
        ASSERT(containsKeyCnt < MAX, "Key found in region.");
      }
      if (val == nullptr) {
        if (regPtr->containsValueForKey(keyPtr)) {
          containsValueCnt++;
        } else {
          break;
        }
        ASSERT(containsValueCnt < MAX, "Value found in region.");
      }

      if (val != nullptr) {
        auto checkPtr =
            std::dynamic_pointer_cast<apache::geode::client::CacheableString>(regPtr->get(keyPtr));

        ASSERT(checkPtr != nullptr, "Value Ptr should not be null.");
        char buf[1024];
        sprintf(buf, "In verify loop, get returned %s for key %s",
                checkPtr->value().c_str(), key);
        LOG(buf);
        if (strcmp(checkPtr->value().c_str(), value) != 0) {
          testValueCnt++;
        } else {
          break;
        }
        ASSERT(testValueCnt < MAX, "Incorrect value found.");
      }
    }
    dunit::sleep(SLEEP);
  }
}

void _verifyEntry(const char* name, const char* key, const char* val,
                  int line) {
  char logmsg[1024];
  sprintf(logmsg, "verifyEntry() called from %d.\n", line);
  LOG(logmsg);
  _verifyEntry(name, key, val, false);
  LOG("Entry verified.");
}

#define verifyDestroyed(x, y) _verifyDestroyed(x, y, __LINE__)

void _verifyDestroyed(const char* name, const char* key, int line) {
  char logmsg[1024];
  sprintf(logmsg, "verifyDestroyed() called from %d.\n", line);
  LOG(logmsg);
  _verifyEntry(name, key, nullptr, true);
  LOG("Entry destroyed.");
}

#define verifyCreated(x, y) _verifyCreated(x, y, __LINE__)

void _verifyCreated(const char* name, const char* key, int line) {
  char logmsg[1024];
  sprintf(logmsg, "verifyCreated() called from %d.\n", line);
  LOG(logmsg);
  _verifyEntry(name, key, nullptr, false, true);
  LOG("Entry created.");
}

#define verifyInvalid(x, y) _verifyInvalid(x, y, __LINE__)

void _verifyInvalid(const char* name, const char* key, int line) {
  char logmsg[1024];
  sprintf(logmsg, "verifyInvalid() called from %d.\n", line);
  LOG(logmsg);
  _verifyEntry(name, key, nullptr, false);
  LOG("Entry invalidated.");
}


void _verifyIntEntry(const char* name, const char* key, const int val,
                     bool noKey, bool isCreated = false) {
  // Verify key and value exist in this region, in this process.
  int value = val;
  char* buf = reinterpret_cast<char*>(malloc(1024 + strlen(key) + 20));
  ASSERT(buf, "Unable to malloc buffer for logging.");
  if (!isCreated) {
    if (noKey) {
      sprintf(buf, "Verify key %s does not exist in region %s", key, name);
    } else if (val == 0) {
      sprintf(buf, "Verify value for key %s does not exist in region %s", key,
              name);
    } else {
      sprintf(buf, "Verify value for key %s is: %d in region %s", key, value,
              name);
    }
    LOG(buf);
  }
  free(buf);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto keyPtr = apache::geode::client::CacheableKey::create(key);

  // if the region is no ack, then we may need to wait...
  if (!isCreated) {
    if (noKey == false) {  // need to find the key!
      ASSERT(regPtr->containsKey(keyPtr), "Key not found in region.");
    }
    if (val != 0) {  // need to have a value!
      // ASSERT( regPtr->containsValueForKey( keyPtr ), "Value not found in
      // region." );
    }
  }

  // loop up to MAX times, testing condition
  uint32_t MAX = 100;
  //  changed sleep from 10 ms
  uint32_t SLEEP = 10;  // milliseconds
  uint32_t containsKeyCnt = 0;
  uint32_t containsValueCnt = 0;
  uint32_t testValueCnt = 0;

  for (int i = MAX; i >= 0; i--) {
    if (isCreated) {
      if (!regPtr->containsKey(keyPtr)) {
        containsKeyCnt++;
      } else {
        break;
      }
      ASSERT(containsKeyCnt < MAX, "Key has not been created in region.");
    } else {
      if (noKey) {
        if (regPtr->containsKey(keyPtr)) {
          containsKeyCnt++;
        } else {
          break;
        }
        ASSERT(containsKeyCnt < MAX, "Key found in region.");
      }
      if (val == 0) {
        if (regPtr->containsValueForKey(keyPtr)) {
          containsValueCnt++;
        } else {
          break;
        }
        ASSERT(containsValueCnt < MAX, "Value found in region.");
      }

      if (val != 0) {
        auto checkPtr =
            std::dynamic_pointer_cast<apache::geode::client::CacheableInt32>(regPtr->get(keyPtr));

        ASSERT(checkPtr != nullptr, "Value Ptr should not be null.");
        char buf[1024];
        sprintf(buf, "In verify loop, get returned %d for key %s",
                checkPtr->value(), key);
        LOG(buf);
        // if ( strcmp( checkPtr->value().c_str(), value ) != 0 ){
        if (checkPtr->value() != value) {
          testValueCnt++;
        } else {
          break;
        }
        ASSERT(testValueCnt < MAX, "Incorrect value found.");
      }
    }
    dunit::sleep(SLEEP);
  }
}

#define verifyIntEntry(x, y, z) _verifyIntEntry(x, y, z, __LINE__)

void _verifyIntEntry(const char* name, const char* key, const int val,
                     int line) {
  char logmsg[1024];
  sprintf(logmsg, "verifyIntEntry() called from %d.\n", line);
  LOG(logmsg);
  _verifyIntEntry(name, key, val, false);
  LOG("Entry verified.");
}

void initClient(const bool isthinClient,
                const std::shared_ptr<apache::geode::client::Properties>& configPtr = nullptr) {
  if (cacheHelper == nullptr) {
    cacheHelper = new apache::geode::client::CacheHelper(isthinClient, configPtr);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void initClient(const bool isThinClient, const bool redirectLog) {
  if (cacheHelper == nullptr) {
    auto config = apache::geode::client::Properties::create();
    if (g_isGridClient) {
      config->insert("grid-client", "true");
    }
    config->insert("log-level", "finer");

    if (redirectLog) {
      config->insert("log-file", apache::geode::client::CacheHelper::unitTestOutputFile());
    }

    initClient(isThinClient, config);
  }
}

void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}

void createPooledRegion(const char *name, bool ackMode, const char *locators,
                        const char *poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  auto regPtr =
      getHelper()->createPooledRegion(name, ackMode, locators, poolname,
                                      cachingEnable, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}

std::shared_ptr<apache::geode::client::Region> createPooledRegion(
    const char* name, bool ackMode, const char* locators, const char* poolname,
    bool clientNotificationEnabled,
    const std::shared_ptr<apache::geode::client::CacheListener>& listener,
    bool caching = true) {
  LOG("createPooledRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);

  if (cacheHelper == nullptr) {
    cacheHelper = new apache::geode::client::CacheHelper(true, poolname, locators, nullptr);
  }

  // ack, caching
  auto regPtr = getHelper()->createPooledRegion(
      name, ackMode, locators, poolname, caching, clientNotificationEnabled,
      std::chrono::seconds(0), std::chrono::seconds(0), std::chrono::seconds(0),
      std::chrono::seconds(0), 0, listener);

  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
  return regPtr;
}

void createEntry(const char *name, const char *key, const char *value, bool verify = true) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Create entry, verify entry is correct
  auto keyPtr = apache::geode::client::CacheableKey::create(key);
  auto valPtr = apache::geode::client::CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  // regPtr->create( keyPtr, valPtr );
  regPtr->put(keyPtr, valPtr);
  LOG("Created entry.");

  if (verify) {
    verifyEntry(name, key, value);
  }
  LOG("Entry created.");
}

void updateEntry(const char* name, const char* key, const char* value,
                 bool checkVal = true, bool checkKey = true) {
  LOG("updateEntry() entered.");
  fprintf(stdout, "Updating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Update entry, verify entry is correct
  auto keyPtr = apache::geode::client::CacheableKey::create(key);
  auto valPtr = apache::geode::client::CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  if (checkKey) {
    ASSERT(regPtr->containsKey(keyPtr),
           "Key should have been found in region.");
  }
  if (checkVal) {
    ASSERT(regPtr->containsValueForKey(keyPtr),
           "Value should have been found in region.");
  }

  regPtr->put(keyPtr, valPtr);
  LOG("Put entry.");

  _verifyEntry(name, key, value, checkVal);
  LOG("Entry updated.");
}

void doNetSearch(const char *name, const char *key, const char *value) {
  LOG("doNetSearch() entered.");
  fprintf(
      stdout,
      "Netsearching for entry -- key: %s  expecting value: %s in region %s\n",
      key, value, name);
  fflush(stdout);
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = apache::geode::client::CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  fprintf(stdout, "netsearch  region %s\n", regPtr->getName().c_str());
  fflush(stdout);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto checkPtr = std::dynamic_pointer_cast<apache::geode::client::CacheableString>(
      regPtr->get(keyPtr));  // force a netsearch

  if (checkPtr != nullptr) {
    LOG("checkPtr is not null");
    char buf[1024];
    sprintf(buf, "In net search, get returned %s for key %s",
            checkPtr->value().c_str(), key);
    LOG(buf);
  } else {
    LOG("checkPtr is nullptr");
  }
  verifyEntry(name, key, value);
  LOG("Netsearch complete.");
}

void doNetSearch(const char* name, const char* key, const char* value,
                 bool checkVal) {
  LOG("doNetSearch() entered.");
  fprintf(
      stdout,
      "Netsearching for entry -- key: %s  expecting value: %s in region %s\n",
      key, value, name);
  fflush(stdout);
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = apache::geode::client::CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  fprintf(stdout, "netsearch  region %s\n", regPtr->getName().c_str());
  fflush(stdout);
  ASSERT(regPtr != nullptr, "Region not found.");

  // ASSERT( !regPtr->containsKey( keyPtr ), "Key should not have been found in
  // region." );
  if (checkVal) {
    ASSERT(!regPtr->containsValueForKey(keyPtr),
           "Value should not have been found in region.");
  }

  auto checkPtr = std::dynamic_pointer_cast<apache::geode::client::CacheableString>(
      regPtr->get(keyPtr));  // force a netsearch

  if (checkPtr != nullptr) {
    LOG("checkPtr is not null");
    char buf[1024];
    sprintf(buf, "In net search, get returned %s for key %s",
            checkPtr->value().c_str(), key);
    LOG(buf);
  } else {
    LOG("checkPtr is nullptr");
  }
  verifyEntry(name, key, value);
  LOG("Netsearch complete.");
}

#endif // GEODE_INTEGRATION_TEST_VERIFY_MACROS_H_
