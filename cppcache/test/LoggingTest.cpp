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

const int __1K__ = 1024;
const int __4K__ = 4 * __1K__;
const int __1M__ = __1K__ * __1K__;

const char* testLogFileName = "LoggingTest.log";

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
  ASSERT_THROW(LogInit(apache::geode::client::LogLevel::Config, "", __4K__),
               apache::geode::client::IllegalArgumentException);
  ASSERT_THROW(LogInit(apache::geode::client::LogLevel::Config, "", 0, __4K__),
               apache::geode::client::IllegalArgumentException);

  // Specify a disk space limit smaller than the file size limit
  ASSERT_THROW(
      LogInit(apache::geode::client::LogLevel::Config, "", __1M__, __4K__),
      apache::geode::client::IllegalArgumentException);
}

TEST(LoggingTest, logToFileAtEachLevel) {
  ASSERT_NO_THROW(
      LogInit(apache::geode::client::LogLevel::Debug, testLogFileName));
  LogDebug("This is a debug string");
  LogDebug("This is a formatted debug string (%d)", __1M__);
  LogClose();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);
  ASSERT_FALSE(boost::filesystem::exists(testLogFileName));

  ASSERT_NO_THROW(
      LogInit(apache::geode::client::LogLevel::Finest, testLogFileName));
  LogFinest("This is a 'finest' string");
  LogFinest("This is a formatted 'finest' string (%d)", __1M__);
  LogClose();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(
      LogInit(apache::geode::client::LogLevel::Finer, testLogFileName));
  LogFiner("This is a 'finer' string");
  LogFiner("This is a formatted 'finer' string (%d)", __1M__);
  LogClose();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(
      LogInit(apache::geode::client::LogLevel::Fine, testLogFileName));
  LogFine("This is a 'fine' string");
  LogFine("This is a formatted 'fine' string (%d)", __1M__);
  LogClose();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(
      LogInit(apache::geode::client::LogLevel::Config, testLogFileName));
  LogConfig("This is a 'config' string");
  LogConfig("This is a formatted 'config' string (%d)", __1M__);
  LogClose();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(
      LogInit(apache::geode::client::LogLevel::Info, testLogFileName));
  LogInfo("This is a 'finer' string");
  LogInfo("This is a formatted 'finer' string (%d)", __1M__);
  LogClose();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(
      LogInit(apache::geode::client::LogLevel::Warning, testLogFileName));
  LogWarning("This is a 'warning' string");
  LogWarning("This is a formatted 'warning' string (%d)", __1M__);
  LogClose();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(
      LogInit(apache::geode::client::LogLevel::Error, testLogFileName));
  LogError("This is a 'error' string");
  LogError("This is a formatted 'error' string (%d)", __1M__);
  LogClose();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);
}
