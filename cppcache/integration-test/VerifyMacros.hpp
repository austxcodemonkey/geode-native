#pragma once

#ifndef GEODE_INTEGRATION_TEST_VERIFY_MACROS_H_
#define GEODE_INTEGRATION_TEST_VERIFY_MACROS_H_

#include <geode/CacheableKey.hpp>

#include "CacheHelper.hpp"
#include "fw_dunit.hpp"

#ifndef CacheHelperOwner
extern apache::geode::client::CacheHelper* cacheHelper;
extern bool g_isGridClient;
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

apache::geode::client::CacheHelper* getHelper();

void _verifyCreated(const char* name, const char* key, int line);
void _verifyDestroyed(const char* name, const char* key, int line);
void _verifyEntry(const char* name, const char* key, const char* val,
                  bool noKey, bool isCreated = false);
void _verifyEntry(const char* name, const char* key, const char* val, int line);
void _verifyIntEntry(const char* name, const char* key, const int val,
                     bool noKey, bool isCreated = false);
void _verifyIntEntry(const char* name, const char* key, const int val,
                     int line);
void _verifyInvalid(const char* name, const char* key, int line);
void cleanProc();
std::shared_ptr<apache::geode::client::Region> createPooledRegion(
    const char* name, bool ackMode, const char* locators, const char* poolname,
    bool clientNotificationEnabled,
    const std::shared_ptr<apache::geode::client::CacheListener>& listener,
    bool caching = true);
void createEntry(const char* name, const char* key, const char* value,
                 bool verify = true);
void createPooledRegion(const char* name, bool ackMode, const char* locators,
                        const char* poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true);
void doNetSearch(const char* name, const char* key, const char* value);
void doNetSearch(const char* name, const char* key, const char* value,
                 bool checkVal);
void initClient(const bool isthinClient,
                const std::shared_ptr<apache::geode::client::Properties>&
                    configPtr = nullptr);
void initClient(const bool isThinClient, const bool redirectLog);
void updateEntry(const char* name, const char* key, const char* value,
                 bool checkVal = true, bool checkKey = true);

#define verifyCreated(x, y) _verifyCreated(x, y, __LINE__)
#define verifyDestroyed(x, y) _verifyDestroyed(x, y, __LINE__)
#define verifyEntry(x, y, z) _verifyEntry(x, y, z, __LINE__)
#define verifyIntEntry(x, y, z) _verifyIntEntry(x, y, z, __LINE__)
#define verifyInvalid(x, y) _verifyInvalid(x, y, __LINE__)

#endif  // GEODE_INTEGRATION_TEST_VERIFY_MACROS_H_
