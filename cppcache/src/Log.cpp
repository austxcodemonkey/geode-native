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

APACHE_GEODE_EXPORT void LoggyMcLogFace(const char*, ...) {}
APACHE_GEODE_EXPORT void LoggyMcLogFace(const std::string&, ...) {}

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

  if (logFileSizeLimit) {
    maxFiles = logDiskSpaceLimit / logFileSizeLimit;
  }

  return maxFiles;
}

spdlog::level::level_enum geodeLogLevelToSpdlogLevel(
    apache::geode::client::LogLevel logLevel) {
  auto level = spdlog::level::level_enum::off;
  switch (logLevel) {
    case apache::geode::client::LogLevel::None:
      level = spdlog::level::level_enum::off;
    case apache::geode::client::LogLevel::Error:
      level = spdlog::level::level_enum::err;
    case apache::geode::client::LogLevel::Warning:
      level = spdlog::level::level_enum::warn;
    case apache::geode::client::LogLevel::Info:
      level = spdlog::level::level_enum::info;
    case apache::geode::client::LogLevel::Default:
      level = spdlog::level::level_enum::info;
    case apache::geode::client::LogLevel::Config:
      level = spdlog::level::level_enum::info;
    case apache::geode::client::LogLevel::Fine:
      level = spdlog::level::level_enum::debug;
    case apache::geode::client::LogLevel::Finer:
      level = spdlog::level::level_enum::debug;
    case apache::geode::client::LogLevel::Finest:
      level = spdlog::level::level_enum::debug;
    case apache::geode::client::LogLevel::Debug:
      level = spdlog::level::level_enum::debug;
    case apache::geode::client::LogLevel::All:
      level = spdlog::level::level_enum::debug;
  }

  return level;
}

APACHE_GEODE_EXPORT void LogInit(apache::geode::client::LogLevel logLevel,
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
      currentLevel = logLevel;
      currentLogger = spdlog::stderr_color_mt("console");
      currentLogger->set_level(geodeLogLevelToSpdlogLevel(currentLevel));
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
      auto diskSpaceLimit = logDiskSpaceLimit ? logDiskSpaceLimit : __1G__;
      auto maxFiles =
          calculateMaxFilesForSpaceLimit(diskSpaceLimit, logFileSizeLimit);
      currentLevel = logLevel;
      currentLogger = spdlog::rotating_logger_mt("file", logFilename,
                                                 diskSpaceLimit, maxFiles);
      currentLogger->set_level(geodeLogLevelToSpdlogLevel(currentLevel));
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
          ("Unexpected log localLevelName: " + localLevelName).c_str());
    }
  }

  return level;
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
          ("Unexpected log localLevelName: " + localLevelName).c_str());
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
    case apache::geode::client::LogLevel::Error:
      levelName = "error";
    case apache::geode::client::LogLevel::Warning:
      levelName = "warning";
    case apache::geode::client::LogLevel::Info:
      levelName = "info";
    case apache::geode::client::LogLevel::Default:
      levelName = "default";
    case apache::geode::client::LogLevel::Config:
      levelName = "config";
    case apache::geode::client::LogLevel::Fine:
      levelName = "fine";
    case apache::geode::client::LogLevel::Finer:
      levelName = "finer";
    case apache::geode::client::LogLevel::Finest:
      levelName = "finest";
    case apache::geode::client::LogLevel::Debug:
      levelName = "debug";
    case apache::geode::client::LogLevel::All:
      levelName = "all";
  }
  return levelName;
}

void LogDebug(const char* format, ...) {
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

void LogDebug(const std::string& msg) { LogDebug(msg.c_str()); }

void LogFinest(const char* format, ...) {
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

void LogFinest(const std::string& msg) { LogFinest(msg.c_str()); }

void LogFiner(const char* format, ...) {
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

void LogFiner(const std::string& msg) { LogFiner(msg.c_str()); }

void LogFine(const char* format, ...) {
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

void LogFine(const std::string& msg) { LogFine(msg.c_str()); }

void LogConfig(const char* format, ...) {
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

void LogConfig(const std::string& msg) { LogConfig(msg.c_str()); }

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

void LogInfo(const std::string& msg) { LogInfo(msg.c_str()); }

void LogWarning(const char* format, ...) {
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

void LogWarning(const std::string& msg) { LogWarning(msg.c_str()); }

void LogError(const char* format, ...) {
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

void LogError(const std::string& msg) { LogError(msg.c_str()); }
