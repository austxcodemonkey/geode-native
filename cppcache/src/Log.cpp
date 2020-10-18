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

namespace apache {
namespace geode {
namespace client {

const int __1K__ = 1024;
const int __1M__ = __1K__ * __1K__;
const int __1G__ = __1K__ * __1M__;
const int LOG_SCRATCH_BUFFER_SIZE = 16 * __1K__;

std::shared_ptr<spdlog::logger> currentLogger;
LogLevel currentLevel;

const std::shared_ptr<spdlog::logger>& getCurrentLogger() {
  return currentLogger;
}

uint32_t calculateMaxFilesForSpaceLimit(uint64_t logDiskSpaceLimit,
                                        uint32_t logFileSizeLimit) {
  uint32_t maxFiles = 1;

  maxFiles = static_cast<uint32_t>(logDiskSpaceLimit / logFileSizeLimit);
  // Must specify at least 1!
  maxFiles = maxFiles ? maxFiles : 1;

  return maxFiles;
}

spdlog::level::level_enum geodeLogLevelToSpdlogLevel(LogLevel logLevel) {
  auto level = spdlog::level::level_enum::off;
  switch (logLevel) {
    case LogLevel::None:
      level = spdlog::level::level_enum::off;
      break;
    case LogLevel::Error:
      level = spdlog::level::level_enum::err;
      break;
    case LogLevel::Warning:
      level = spdlog::level::level_enum::warn;
      break;
    case LogLevel::Info:
      level = spdlog::level::level_enum::info;
      break;
    case LogLevel::Default:
      level = spdlog::level::level_enum::info;
      break;
    case LogLevel::Config:
      level = spdlog::level::level_enum::info;
      break;
    case LogLevel::Fine:
      level = spdlog::level::level_enum::debug;
      break;
    case LogLevel::Finer:
      level = spdlog::level::level_enum::debug;
      break;
    case LogLevel::Finest:
      level = spdlog::level::level_enum::debug;
      break;
    case LogLevel::Debug:
      level = spdlog::level::level_enum::debug;
      break;
    case LogLevel::All:
      level = spdlog::level::level_enum::debug;
      break;
  }

  return level;
}

void Log::init(LogLevel logLevel, const std::string& logFilename,
               uint32_t logFileSizeLimit, uint64_t logDiskSpaceLimit) {
  try {
    if (logFilename.empty()) {
      if (logFileSizeLimit || logDiskSpaceLimit) {
        IllegalArgumentException ex(
            "Cannot specify a file or disk space size limit without specifying "
            "a log file name.");
        throw ex;
      }
      currentLevel = logLevel;
      // Check for multiple initialization
      if (currentLogger && currentLogger->name() == "console") {
        return;
      } else {
        close();
      }
      currentLogger = spdlog::stderr_color_mt("console");
      currentLogger->set_level(geodeLogLevelToSpdlogLevel(currentLevel));
    } else {
      if (!boost::filesystem::portable_file_name(logFilename)) {
        IllegalArgumentException ex(std::string("The filename \"") +
                                    logFilename +
                                    "\" is not valid for a log file.");
        throw ex;
      }
      if (logDiskSpaceLimit && logFileSizeLimit > logDiskSpaceLimit) {
        IllegalArgumentException ex(
            "File size limit must be smaller than disk space limit for "
            "logging.");
        throw ex;
      }

      // If disk space limit isn't specified, default to 1GB
      auto diskSpaceLimit = logDiskSpaceLimit ? logDiskSpaceLimit : __1G__;

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
      diskSpaceLimit = 9 * static_cast<uint64_t>(diskSpaceLimit) / 10;
      uint32_t fileSizeLimit = 0;
      if (logFileSizeLimit) {
        fileSizeLimit = logFileSizeLimit;
      } else if (diskSpaceLimit > UINT32_MAX) {
        fileSizeLimit = UINT32_MAX;
      } else {
        fileSizeLimit = static_cast<uint32_t>(diskSpaceLimit);
      }
      auto maxFiles =
          calculateMaxFilesForSpaceLimit(diskSpaceLimit, fileSizeLimit);
      currentLevel = logLevel;

      // Check for multiple initialization
      if (!currentLogger || currentLogger->name() != "file") {
        close();
        currentLogger = spdlog::rotating_logger_mt("file", logFilename,
                                                   fileSizeLimit, maxFiles);
        currentLogger->set_level(geodeLogLevelToSpdlogLevel(currentLevel));
      }
    }
  } catch (const spdlog::spdlog_ex& ex) {
    throw IllegalStateException(ex.what());
  }
}

void Log::close() {
  if (currentLogger) {
    spdlog::drop(currentLogger->name());
  }
  currentLogger = nullptr;
}

LogLevel Log::stringToLogLevel(const std::string& levelName) {
  auto level = LogLevel::None;

  if (levelName.size()) {
    auto localLevelName = levelName;

    std::transform(localLevelName.begin(), localLevelName.end(),
                   localLevelName.begin(), ::tolower);

    if (localLevelName == "none") {
      level = LogLevel::None;
    } else if (localLevelName == "error") {
      level = LogLevel::Error;
    } else if (localLevelName == "warning") {
      level = LogLevel::Warning;
    } else if (localLevelName == "info") {
      level = LogLevel::Info;
    } else if (localLevelName == "default") {
      level = LogLevel::Default;
    } else if (localLevelName == "config") {
      level = LogLevel::Config;
    } else if (localLevelName == "fine") {
      level = LogLevel::Fine;
    } else if (localLevelName == "finer") {
      level = LogLevel::Finer;
    } else if (localLevelName == "finest") {
      level = LogLevel::Finest;
    } else if (localLevelName == "debug") {
      level = LogLevel::Debug;
    } else if (localLevelName == "all") {
      level = LogLevel::All;
    } else {
      throw IllegalArgumentException(
          ("Unexpected log level name: " + localLevelName).c_str());
    }
  }

  return level;
}

std::string Log::logLevelToString(LogLevel level) {
  std::string levelName = "None";
  switch (level) {
    case LogLevel::None:
      levelName = "none";
      break;
    case LogLevel::Error:
      levelName = "error";
      break;
    case LogLevel::Warning:
      levelName = "warning";
      break;
    case LogLevel::Info:
      levelName = "info";
      break;
    case LogLevel::Default:
      levelName = "default";
      break;
    case LogLevel::Config:
      levelName = "config";
      break;
    case LogLevel::Fine:
      levelName = "fine";
      break;
    case LogLevel::Finer:
      levelName = "finer";
      break;
    case LogLevel::Finest:
      levelName = "finest";
      break;
    case LogLevel::Debug:
      levelName = "debug";
      break;
    case LogLevel::All:
      levelName = "all";
      break;
  }
  return levelName;
}

LogLevel Log::logLevel() { return currentLevel; }

void Log::setLogLevel(LogLevel level) {
  currentLevel = level;
  if (currentLogger) {
    currentLogger->set_level(geodeLogLevelToSpdlogLevel(level));
  }
}

void Log::debug(const char* format, ...) {
  if (currentLevel >= LogLevel::Debug) {
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

void Log::debug(const std::string& msg) { debug(msg.c_str()); }

void Log::finest(const char* format, ...) {
  if (currentLevel >= LogLevel::Finest) {
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

void Log::finest(const std::string& msg) { Log::finest(msg.c_str()); }

void Log::finer(const char* format, ...) {
  if (currentLevel >= LogLevel::Finer) {
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

void Log::finer(const std::string& msg) { Log::finer(msg.c_str()); }

void Log::fine(const char* format, ...) {
  if (currentLevel >= LogLevel::Fine) {
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

void Log::fine(const std::string& msg) { Log::fine(msg.c_str()); }

void Log::config(const char* format, ...) {
  if (currentLevel >= LogLevel::Config) {
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

void Log::config(const std::string& msg) { config(msg.c_str()); }

void Log::info(const char* format, ...) {
  if (currentLevel >= LogLevel::Info) {
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

void Log::info(const std::string& msg) { info(msg.c_str()); }

void Log::warning(const char* format, ...) {
  if (currentLevel >= LogLevel::Warning) {
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

void Log::warning(const std::string& msg) { warning(msg.c_str()); }

void Log::error(const char* format, ...) {
  if (currentLevel >= LogLevel::Error) {
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

void Log::error(const std::string& msg) { error(msg.c_str()); }

void Log::log(LogLevel level, const char* msg) {
  switch (level) {
    case LogLevel::None:
      break;
    case LogLevel::Error:
      error(msg);
      break;
    case LogLevel::Warning:
      warning(msg);
      break;
    case LogLevel::Info:
      info(msg);
      break;
    case LogLevel::Default:
    case LogLevel::Config:
      config(msg);
      break;
    case LogLevel::Fine:
      fine(msg);
      break;
    case LogLevel::Finer:
      finer(msg);
      break;
    case LogLevel::Finest:
      finest(msg);
      break;
    case LogLevel::Debug:
    case LogLevel::All:
      debug(msg);
      break;
  }
}
void Log::writeBanner() {}
}  // namespace client
}  // namespace geode
}  // namespace apache
