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

#include <util/Log.hpp>

#include <gtest/gtest.h>

#include <geode/AuthenticatedView.hpp>
#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "boost/filesystem.hpp"

using apache::geode::client::CacheClosedException;
using apache::geode::client::CacheFactory;
using apache::geode::client::LogLevel;
using apache::geode::client::RegionShortcut;

const int __1KB__ = 1024;
const int __4KB__ = 4 * __1KB__;
const int __1MB__ = __1KB__ * __1KB__;

/**
 * Verify we can initialize the logger with any combination of level, filename,
 * file size limit, and disk space limit
 */
TEST(LoggingTest, logInit) {
  // Check all valid levels
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::None));
  LogClose();
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::Error));
  LogClose();
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::Warning));
  LogClose();
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::Info));
  LogClose();
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::Default));
  LogClose();
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::Config));
  LogClose();
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::Fine));
  LogClose();
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::Finer));
  LogClose();
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::Finest));
  LogClose();
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::Debug));
  LogClose();
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::All));
  LogClose();

  // Init with valid filename
  ASSERT_NO_THROW(
      LogInit(apache::geode::client::LogLevel::Config, "LoggingTest.log"));
  LogClose();

  // Init with invalid filename
  ASSERT_THROW(LogInit(apache::geode::client::LogLevel::Config, "#?$?%.log"),
               apache::geode::client::IllegalArgumentException);

  // Specify disk or file limit without a filename
  ASSERT_THROW(LogInit(apache::geode::client::LogLevel::Config, "", __4KB__),
               apache::geode::client::IllegalArgumentException);
  ASSERT_THROW(LogInit(apache::geode::client::LogLevel::Config, "", 0, __4KB__),
               apache::geode::client::IllegalArgumentException);

  // Specify a disk space limit smaller than the file size limit
  ASSERT_THROW(
      LogInit(apache::geode::client::LogLevel::Config, "", __1MB__, __4KB__),
      apache::geode::client::IllegalArgumentException);
}
