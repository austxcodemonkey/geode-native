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

namespace {

const int __1K__ = 1024;
const int __4K__ = 4 * __1K__;
const int __1M__ = __1K__ * __1K__;

const char* testLogFileName = "LoggingTest.log";

const char* __1KStringLiteral =
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

class LoggingTest : public testing::Test {
  void scrubTestLogFiles() {
    if (boost::filesystem::exists(testLogFileName)) {
      boost::filesystem::remove(testLogFileName);
    }

    auto base = boost::filesystem::path(testLogFileName).stem();
    auto ext = boost::filesystem::path(testLogFileName).extension();
    for (auto i = 1;; i++) {
      auto rolledLogFileName =
          base.string() + "." + std::to_string(i) + ext.string();
      if (boost::filesystem::exists(rolledLogFileName)) {
        boost::filesystem::remove(rolledLogFileName);
      } else {
        break;
      }
    }
  }

  virtual void SetUp() { scrubTestLogFiles(); }

  virtual void TearDown() { scrubTestLogFiles(); }
};

/**
 * Verify we can initialize the logger with any combination of level,
 * filename, file size limit, and disk space limit
 */
TEST_F(LoggingTest, logInit) {
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

TEST_F(LoggingTest, logToFileAtEachLevel) {
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

TEST_F(LoggingTest, verifyFileSizeLimit) {
  ASSERT_NO_THROW(
      LogInit(apache::geode::client::LogLevel::Debug, testLogFileName, __1K__));
  for (auto i = 0; i < 4; i++) {
    LogDebug(__1KStringLiteral);
  }
  LogClose();

  // Original file should still be around
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));

  // Check for 'rolled' log files.  With a 1KB file size limit and each logged
  // string having a length of 1K chars, we should have at least one less
  // rolled log file than the number of strings logged, i.e. 3 rolled files
  // for 4 strings in this case.  spdlog rolled files look like
  // <<basename>>.<<#>>.<<extension>>, so for LoggingTest.log we should find
  // LoggingTest.1.log, LoggingTest.2.log, etc.
  auto base = boost::filesystem::path(testLogFileName).stem();
  auto ext = boost::filesystem::path(testLogFileName).extension();

  // File size limit is treated as a "soft" limit.  If the last message in the
  // log puts the file size over the limit, the file is rolled and the message
  // is preserved intact, rather than truncated or split across files.  We'll
  // assume the file size never exceeds 110% of the specified limit.
  auto adjustedFileSizeLimit =
      static_cast<uint32_t>(static_cast<uint64_t>(__1K__) * 11 / 10);

  for (auto i = 1; i < 4; i++) {
    auto rolledLogFileName =
        base.string() + "." + std::to_string(i) + ext.string();

    ASSERT_TRUE(boost::filesystem::exists(rolledLogFileName));
    ASSERT_TRUE(adjustedFileSizeLimit >
                boost::filesystem::file_size(rolledLogFileName));
  }
}

TEST_F(LoggingTest, verifyDiskSpaceLimit) {
  const int NUMBER_OF_ITERATIONS = 100;
  const int DISK_SPACE_LIMIT = NUMBER_OF_ITERATIONS * __1K__;

  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::Debug,
                          testLogFileName, __1K__, DISK_SPACE_LIMIT));
  for (auto i = 0; i < NUMBER_OF_ITERATIONS; i++) {
    LogDebug(__1KStringLiteral);
  }
  LogClose();

  // Original file should still be around
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));

  auto size = boost::filesystem::file_size(testLogFileName);
  auto base = boost::filesystem::path(testLogFileName).stem();
  auto ext = boost::filesystem::path(testLogFileName).extension();
  for (auto i = 1;; i++) {
    auto rolledLogFileName =
        base.string() + "." + std::to_string(i) + ext.string();
    if (boost::filesystem::exists(rolledLogFileName)) {
      size += boost::filesystem::file_size(rolledLogFileName);
    } else {
      break;
    }
  }

  ASSERT_TRUE(size <= DISK_SPACE_LIMIT);
}

TEST_F(LoggingTest, logToStream) {
  ASSERT_NO_THROW(LogInit(apache::geode::client::LogLevel::Debug));
  LogError("This is an ERROR level message");
  LogWarning("This is an WARNING level message");
  LogInfo("This is an INFO level message");
  LogConfig("This is an CONFIG level message");
  LogFine("This is an FINE level message");
  LogFiner("This is an FINER level message");
  LogFinest("This is an FINEST level message");
  LogDebug("This is an DEBUG level message");
}
}  // namespace
