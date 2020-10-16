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

const int __1K__ = 1024;
const int __1M__ = __1K__ * __1K__;
const int __1G__ = __1K__ * __1M__;
const int LOG_SCRATCH_BUFFER_SIZE = 16 * __1K__;

const std::shared_ptr<spdlog::logger>& getCurrentLogger() {
  return currentLogger;
}

uint32_t calculateMaxFilesForSpaceLimit(uint32_t logDiskSpaceLimit,
                                        uint32_t logFileSizeLimit) {
  uint32_t maxFiles = 1;

  maxFiles = logDiskSpaceLimit / logFileSizeLimit;
  // Must specify at least 1!
  maxFiles = maxFiles ? maxFiles : 1;

  return maxFiles;
}

spdlog::level::level_enum geodeLogLevelToSpdlogLevel(
    apache::geode::client::LogLevel logLevel) {
  auto level = spdlog::level::level_enum::off;
  switch (logLevel) {
    case apache::geode::client::LogLevel::None:
      level = spdlog::level::level_enum::off;
      break;
    case apache::geode::client::LogLevel::Error:
      level = spdlog::level::level_enum::err;
      break;
    case apache::geode::client::LogLevel::Warning:
      level = spdlog::level::level_enum::warn;
      break;
    case apache::geode::client::LogLevel::Info:
      level = spdlog::level::level_enum::info;
      break;
    case apache::geode::client::LogLevel::Default:
      level = spdlog::level::level_enum::info;
      break;
    case apache::geode::client::LogLevel::Config:
      level = spdlog::level::level_enum::info;
      break;
    case apache::geode::client::LogLevel::Fine:
      level = spdlog::level::level_enum::debug;
      break;
    case apache::geode::client::LogLevel::Finer:
      level = spdlog::level::level_enum::debug;
      break;
    case apache::geode::client::LogLevel::Finest:
      level = spdlog::level::level_enum::debug;
      break;
    case apache::geode::client::LogLevel::Debug:
      level = spdlog::level::level_enum::debug;
      break;
    case apache::geode::client::LogLevel::All:
      level = spdlog::level::level_enum::debug;
      break;
  }

  return level;
}

APACHE_GEODE_EXPORT void LogInit(
    apache::geode::client::LogLevel logLevel, std::string logFilename,
    uint32_t logFileSizeLimit, uint32_t logDiskSpaceLimit) {
  try {
    if (logFilename.empty()) {
      if (logFileSizeLimit || logDiskSpaceLimit) {
        apache::geode::client::IllegalArgumentException ex(
            "Cannot specify a file or disk space size limit without specifying "
            "a "
            "log file name.");
        throw ex;
      }
      currentLevel = logLevel;
      // Check for multiple initialization
      if (currentLogger && currentLogger->name() == "console") {
        return;
      } else {
        LogClose();
      }
      currentLogger = spdlog::stderr_color_mt("console");
      currentLogger->set_level(geodeLogLevelToSpdlogLevel(currentLevel));
    } else {
      if (!boost::filesystem::portable_file_name(logFilename)) {
        apache::geode::client::IllegalArgumentException ex(
            std::string("The filename \"") + logFilename +
            "\" is not valid for a log file.");
        throw ex;
      }
      if (logDiskSpaceLimit && logFileSizeLimit > logDiskSpaceLimit) {
        apache::geode::client::IllegalArgumentException ex(
            "File size limit must be smaller than disk space limit for "
            "logging.");
        throw ex;
      }
      auto diskSpaceLimit =
          logDiskSpaceLimit ? logDiskSpaceLimit : 100 * __1M__;
      // Use 90% of requested disk space limit as actual limit for logger.  This
      // guarantees in the vast majority of cases the disk space number will be
      // treated as an *absolute* limit.  There are a couple of cases where we
      // will exceed the disk space limit. First, if the very last message in
      // all log files is longer than 10% of the file size limit.  For example,
      // if you set a file size limit of 1MB and your last message in the file
      // is longer than 1MB, the file size limit will be exceeded by > 10%.  If
      // every file had this problem, you'd exceed the *adjusted* disk space
      // limit by > 10%, and we'd exceed the disk space limit. The 2nd case is a
      // specific instance of the 1st - if the file size limit is == the disk
      // space limit, you only need one of these "extremely long" log messages,
      // since you only have one log file. Again, though, it also has to be the
      // last message in the file.  Bottom line, it's very unlikely we'll exceed
      // the disk space limit unless someone goes out of their way to do so.
      auto adjustedLimit = 9 * static_cast<uint64_t>(diskSpaceLimit) / 10;
      diskSpaceLimit = static_cast<uint32_t>(adjustedLimit);
      auto fileSizeLimit = logFileSizeLimit ? logFileSizeLimit : diskSpaceLimit;
      auto maxFiles =
          calculateMaxFilesForSpaceLimit(diskSpaceLimit, fileSizeLimit);
      currentLevel = logLevel;
      // Check for multiple initialization
      if (currentLogger && currentLogger->name() == "file") {
        return;
      } else {
        LogClose();
      }
      currentLogger = spdlog::rotating_logger_mt("file", logFilename,
                                                 fileSizeLimit, maxFiles);
      currentLogger->set_level(geodeLogLevelToSpdlogLevel(currentLevel));
    }
  } catch (const spdlog::spdlog_ex& ex) {
    throw apache::geode::client::IllegalStateException(ex.what());
  }
}

APACHE_GEODE_EXPORT void LogClose() {
  if (currentLogger) {
    spdlog::drop(currentLogger->name());
  }
  currentLogger = nullptr;
}

APACHE_GEODE_EXPORT void LogSetLevel(
    apache::geode::client::LogLevel level) {
  currentLevel = level;
  if (currentLogger) {
    currentLogger->set_level(geodeLogLevelToSpdlogLevel(level));
  }
}

APACHE_GEODE_EXPORT apache::geode::client::LogLevel LogLevelFromString(
    const std::string& levelName) {
  auto level = apache::geode::client::LogLevel::None;

  if (levelName.size()) {
    auto localLevelName = levelName;

    std::transform(localLevelName.begin(), localLevelName.end(),
                   localLevelName.begin(), ::tolower);

    if (localLevelName == "none") {
      level = apache::geode::client::LogLevel::None;
    } else if (localLevelName == "error") {
      level = apache::geode::client::LogLevel::Error;
    } else if (localLevelName == "warning") {
      level = apache::geode::client::LogLevel::Warning;
    } else if (localLevelName == "info") {
      level = apache::geode::client::LogLevel::Info;
    } else if (localLevelName == "default") {
      level = apache::geode::client::LogLevel::Default;
    } else if (localLevelName == "config") {
      level = apache::geode::client::LogLevel::Config;
    } else if (localLevelName == "fine") {
      level = apache::geode::client::LogLevel::Fine;
    } else if (localLevelName == "finer") {
      level = apache::geode::client::LogLevel::Finer;
    } else if (localLevelName == "finest") {
      level = apache::geode::client::LogLevel::Finest;
    } else if (localLevelName == "debug") {
      level = apache::geode::client::LogLevel::Debug;
    } else if (localLevelName == "all") {
      level = apache::geode::client::LogLevel::All;
    } else {
      throw apache::geode::client::IllegalArgumentException(
          ("Unexpected log level name: " + localLevelName).c_str());
    }
  }

  return level;
}

APACHE_GEODE_EXPORT std::string StringFromLogLevel(
    apache::geode::client::LogLevel level) {
  std::string levelName = "None";
  switch (level) {
    case apache::geode::client::LogLevel::None:
      levelName = "none";
      break;
    case apache::geode::client::LogLevel::Error:
      levelName = "error";
      break;
    case apache::geode::client::LogLevel::Warning:
      levelName = "warning";
      break;
    case apache::geode::client::LogLevel::Info:
      levelName = "info";
      break;
    case apache::geode::client::LogLevel::Default:
      levelName = "default";
      break;
    case apache::geode::client::LogLevel::Config:
      levelName = "config";
      break;
    case apache::geode::client::LogLevel::Fine:
      levelName = "fine";
      break;
    case apache::geode::client::LogLevel::Finer:
      levelName = "finer";
      break;
    case apache::geode::client::LogLevel::Finest:
      levelName = "finest";
      break;
    case apache::geode::client::LogLevel::Debug:
      levelName = "debug";
      break;
    case apache::geode::client::LogLevel::All:
      levelName = "all";
      break;
  }
  return levelName;
}

APACHE_GEODE_EXPORT void LogDebug(const char* format, ...) {
  if (currentLevel >= apache::geode::client::LogLevel::Debug) {
    char buf[LOG_SCRATCH_BUFFER_SIZE];
    va_list vl;
    va_start(vl, format);
    if (vsnprintf(buf, LOG_SCRATCH_BUFFER_SIZE, format, vl) > 0) {
      if (currentLogger) {
        currentLogger->debug(buf);
      }
    }
  }
}

APACHE_GEODE_EXPORT void LogDebug(const std::string& msg) {
  LogDebug(msg.c_str());
}

APACHE_GEODE_EXPORT void LogFinest(const char* format, ...) {
  if (currentLevel >= apache::geode::client::LogLevel::Finest) {
    char buf[LOG_SCRATCH_BUFFER_SIZE];
    va_list vl;
    va_start(vl, format);
    if (vsnprintf(buf, LOG_SCRATCH_BUFFER_SIZE, format, vl) > 0) {
      if (currentLogger) {
        currentLogger->debug(buf);
      }
    }
  }
}

APACHE_GEODE_EXPORT void LogFinest(const std::string& msg) {
  LogFinest(msg.c_str());
}

APACHE_GEODE_EXPORT void LogFiner(const char* format, ...) {
  if (currentLevel >= apache::geode::client::LogLevel::Finer) {
    char buf[LOG_SCRATCH_BUFFER_SIZE];
    va_list vl;
    va_start(vl, format);
    if (vsnprintf(buf, LOG_SCRATCH_BUFFER_SIZE, format, vl) > 0) {
      if (currentLogger) {
        currentLogger->debug(buf);
      }
    }
  }
}

APACHE_GEODE_EXPORT void LogFiner(const std::string& msg) {
  LogFiner(msg.c_str());
}

APACHE_GEODE_EXPORT void LogFine(const char* format, ...) {
  if (currentLevel >= apache::geode::client::LogLevel::Fine) {
    char buf[LOG_SCRATCH_BUFFER_SIZE];
    va_list vl;
    va_start(vl, format);
    if (vsnprintf(buf, LOG_SCRATCH_BUFFER_SIZE, format, vl) > 0) {
      if (currentLogger) {
        currentLogger->debug(buf);
      }
    }
  }
}

APACHE_GEODE_EXPORT void LogFine(const std::string& msg) {
  LogFine(msg.c_str());
}

APACHE_GEODE_EXPORT void LogConfig(const char* format, ...) {
  if (currentLevel >= apache::geode::client::LogLevel::Config) {
    char buf[LOG_SCRATCH_BUFFER_SIZE];
    va_list vl;
    va_start(vl, format);
    if (vsnprintf(buf, LOG_SCRATCH_BUFFER_SIZE, format, vl) > 0) {
      if (currentLogger) {
        currentLogger->info(buf);
      }
    }
  }
}

APACHE_GEODE_EXPORT void LogConfig(const std::string& msg) {
  LogConfig(msg.c_str());
}

void LogInfo(const char* format, ...) {
  if (currentLevel >= apache::geode::client::LogLevel::Info) {
    char buf[LOG_SCRATCH_BUFFER_SIZE];
    va_list vl;
    va_start(vl, format);
    if (vsnprintf(buf, LOG_SCRATCH_BUFFER_SIZE, format, vl) > 0) {
      if (currentLogger) {
        currentLogger->info(buf);
      }
    }
  }
}

void LogInfo(const std::string& msg) {
  LogInfo(msg.c_str());
}

APACHE_GEODE_EXPORT void LogWarning(const char* format, ...) {
  if (currentLevel >= apache::geode::client::LogLevel::Warning) {
    char buf[LOG_SCRATCH_BUFFER_SIZE];
    va_list vl;
    va_start(vl, format);
    if (vsnprintf(buf, LOG_SCRATCH_BUFFER_SIZE, format, vl) > 0) {
      if (currentLogger) {
        currentLogger->warn(buf);
      }
    }
  }
}

APACHE_GEODE_EXPORT void LogWarning(const std::string& msg) {
  LogWarning(msg.c_str());
}

APACHE_GEODE_EXPORT void LogError(const char* format, ...) {
  if (currentLevel >= apache::geode::client::LogLevel::Error) {
    char buf[LOG_SCRATCH_BUFFER_SIZE];
    va_list vl;
    va_start(vl, format);
    if (vsnprintf(buf, LOG_SCRATCH_BUFFER_SIZE, format, vl) > 0) {
      if (currentLogger) {
        currentLogger->error(buf);
      }
    }
  }
}

APACHE_GEODE_EXPORT void LogError(const std::string& msg) {
  LogError(msg.c_str());
}
