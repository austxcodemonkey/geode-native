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
#include "util/Log.hpp"

#include <spdlog/spdlog.h>

#include <cstdarg>
#include <cstdio>
#include <string>

#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>
#include <geode/util/LogLevel.hpp>

#include "boost/filesystem/path.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

std::shared_ptr<spdlog::logger> currentLogger;
apache::geode::client::LogLevel currentLevel;

APACHE_GEODE_EXPORT void LoggyMcLogFace(const char*, ...) {}
APACHE_GEODE_EXPORT void LoggyMcLogFace(const std::string&, ...) {}

const int __1K__ = 1024;
const int __1M__ = __1K__ * __1K__;
const int __1G__ = __1K__ * __1M__;

uint32_t calculateMaxFilesForSpaceLimit(uint32_t logDiskSpaceLimit,
                                        uint32_t logFileSizeLimit) {
  uint32_t maxFiles = 1;
  uint32_t diskSpaceLimit = logDiskSpaceLimit;

  if (!diskSpaceLimit) {
    // User says it's fine to just fill up their disk with log files, so let's
    // pick a reasonable max disk space of, say, 1GB
    diskSpaceLimit = __1G__;
  }
  if (logFileSizeLimit) {
    maxFiles = logDiskSpaceLimit / logFileSizeLimit;
  }
  return maxFiles;
}

APACHE_GEODE_EXPORT void LogInit(apache::geode::client::LogLevel,
                                 std::string logFilename,
                                 uint32_t logFileSizeLimit,
                                 uint32_t logDiskSpaceLimit) {
  try {
    if (logFilename.empty()) {
      if (logFileSizeLimit || logDiskSpaceLimit) {
        apache::geode::client::IllegalArgumentException ex(
            "Cannot specify a file or disk space size limit without specifying "
            "a "
            "log file name.");
        throw ex;
      }
      currentLogger = spdlog::stderr_color_mt("console");
    } else {
      if (!boost::filesystem::portable_file_name(logFilename)) {
        apache::geode::client::IllegalArgumentException ex(
            std::string("The filename \"") + logFilename +
            "\" is not valid for a log file.");
        throw ex;
      }
      if (logFileSizeLimit > logDiskSpaceLimit) {
        apache::geode::client::IllegalArgumentException ex(
            "File size limit must be smaller than disk space limit for "
            "logging.");
        throw ex;
      }
      auto maxFiles =
          calculateMaxFilesForSpaceLimit(logDiskSpaceLimit, logFileSizeLimit);
      currentLogger = spdlog::rotating_logger_mt("file", logFilename,
                                                 logDiskSpaceLimit, maxFiles);
    }
  } catch (const spdlog::spdlog_ex ex) {
    throw apache::geode::client::IllegalStateException(ex.what());
  }
}

APACHE_GEODE_EXPORT void LogClose() {
  if (currentLogger) {
    spdlog::drop(currentLogger->name());
  }
  currentLogger = nullptr;
}

APACHE_GEODE_EXPORT void LogSetLevel(apache::geode::client::LogLevel) {}

APACHE_GEODE_EXPORT apache::geode::client::LogLevel logLevelFromString(
    const std::string&) {
  return apache::geode::client::LogLevel::None;
}
APACHE_GEODE_EXPORT std::string stringFromLogLevel(
    apache::geode::client::LogLevel) {
  return "Unknown";
}
