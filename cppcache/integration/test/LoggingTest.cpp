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

#include <framework/Cluster.h>
#include <framework/Framework.h>
#include <framework/Gfsh.h>

#include <future>
#include <iostream>
#include <random>
#include <thread>
#include <util/Log.hpp>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableString;
using apache::geode::client::Log;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;
using apache::geode::client::ScopeTrace;
using std::chrono::minutes;

/**
 */
TEST(LoggingTest, LoggingScopeTest) {
  Log::init(apache::geode::client::LogLevel::All, "loggingTest.log");
  LOG_SCOPE("LoggingScopeTest");
}

}  // namespace
