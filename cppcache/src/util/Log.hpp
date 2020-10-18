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

#pragma once

#ifndef GEODE_LOG_H_
#define GEODE_LOG_H_

#include <cstdarg>
#include <cstdio>
#include <string>

#include <geode/internal/geode_globals.hpp>
#include <geode/util/LogLevel.hpp>

#include "spdlog/spdlog.h"

#define _GF_MSG_LIMIT 8192

namespace apache {
namespace geode {
namespace client {

class Exception;

/** Defines methods available to clients that want to write a log message
 * to their Geode system's shared log file.
 * <p>
 * This class must be initialized prior to its use:
 * @ref Log::init
 * <p>
 * For any logged message the log file will contain:
 * <ul>
 * <li> The message's level.
 * <li> The time the message was logged.
 * <li> The id of the connection and thread that logged the message.
 * <li> The message itself which can be a const char* (perhaps with
 * an exception including the exception's stack trace.
 * </ul>
 * <p>
 * A message always has a level.
 * Logging levels are ordered. Enabling logging at a given level also
 * enables logging at higher levels. The higher the level the more
 * important and urgent the message.
 * <p>
 * The levels, in descending order of severity, are:
 * <ul>
 *
 * <li> <code>error</code> (highest severity) is a message level
 * indicating a serious failure.  In general <code>error</code>
 * messages should describe events that are of considerable
 * importance and which will prevent normal program execution. They
 * should be reasonably intelligible to end users and to system
 * administrators.
 *
 * <li> <code>warning</code> is a message level indicating a
 * potential problem.  In general <code>warning</code> messages
 * should describe events that will be of interest to end users or
 * system managers, or which indicate potential problems.
 *
 * <li> <code>info</code> is a message level for informational
 * messages.  Typically <code>info</code> messages should be
 * reasonably significant and should make sense to end users and
 * system administrators.
 *
 * <li> <code>config</code> is a message level for static
 * configuration messages.  <code>config</code> messages are intended
 * to provide a variety of static configuration information, to
 * assist in debugging problems that may be associated with
 * particular configurations.
 *
 * <li> <code>fine</code> is a message level providing tracing
 * information.  In general the <code>fine</code> level should be
 * used for information that will be broadly interesting to
 * developers. This level is for the lowest volume, and most
 * important, tracing messages.
 *
 * <li> <code>finer</code> indicates a moderately detailed tracing
 * message.  This is an intermediate level between <code>fine</code>
 * and <code>finest</code>.
 *
 * <li> <code>finest</code> indicates a very detailed tracing
 * message.  Logging calls for entering, returning, or throwing an
 * exception are traced at the <code>finest</code> level.
 *
 * <li> <code>debug</code> (lowest severity) indicates a highly
 * detailed tracing message.  In general the <code>debug</code> level
 * should be used for the most voluminous detailed tracing messages.
 * </ul>
 *
 * <p>
 * For each level methods exist that will request a message, at that
 * level, to be logged. These methods are all named after their level.
 * <p>
 * For each level a method exists that indicates if messages at that
 * level will currently be logged. The names of these methods are of
 * the form: <em>level</em><code>Enabled</code>.
 *
 *
 */

class APACHE_GEODE_EXPORT Log {
 public:
  /**
   * Returns the current log level.
   */
  static LogLevel logLevel();

  /**
   * Set the current log level.
   */
  static void setLogLevel(LogLevel level);

  /**
   * Initializes logging facility with given level and filenames.
   * This method is called automatically within @ref DistributedSystem::connect
   * with the log-file, log-level, and log-file-size system properties used as
   * arguments
   */
  static void init
      // 0 => use maximum value (currently 1G)
      (LogLevel level, const std::string& logFileName = "",
       uint32_t logFileLimit = 0, uint64_t logDiskSpaceLimit = 0);

  /**
   * closes logging facility (until next init).
   */
  static void close();

  /**
   * returns character string for given log level. The string will be
   * identical to the enum declaration above, except it will be all
   * lower case. Out of range values will throw
   * IllegalArgumentException.
   */
  static std::string logLevelToString(LogLevel level);

  /**
   * returns log level specified by "chars", or throws
   * IllegalArgumentException.  Allowed values are identical to the
   * enum declaration above for LogLevel, but with character case ignored.
   */
  static LogLevel stringToLogLevel(const std::string& chars);

  /**
   * Logs a message.
   * The message level is "error".
   */
  static void error(const char* format, ...);
  static void error(const std::string& msg);

  /**
   * Logs a message.
   * The message level is "warning".
   */
  static void warning(const char* format, ...);
  static void warning(const std::string& msg);

  /**
   * Logs a message.
   * The message level is "info".
   */
  static void info(const char* msg, ...);
  static void info(const std::string& msg);

  /**
   * Logs a message.
   * The message level is "config".
   */
  static void config(const char* msg, ...);
  static void config(const std::string& msg);

  /**
   * Logs a message.
   * The message level is "fine".
   */
  static void fine(const char* msg, ...);
  static void fine(const std::string& msg);

  /**
   * Logs a message.
   * The message level is "finer".
   */
  static void finer(const char* msg, ...);
  static void finer(const std::string& msg);

  /**
   * Logs a message.
   * The message level is "finest".
   */
  static void finest(const char* msg, ...);
  static void finest(const std::string& msg);

  /**
   * Logs a message.
   * The message level is "debug".
   */
  static void debug(const char* msg, ...);
  static void debug(const std::string& msg);

  static void log(LogLevel level, const char* msg);

 private:
  static void writeBanner();
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#define LOGDEBUG apache::geode::client::Log::debug
#define LOGFINEST apache::geode::client::Log::finest
#define LOGFINER apache::geode::client::Log::finer
#define LOGFINE apache::geode::client::Log::fine
#define LOGCONFIG apache::geode::client::Log::config
#define LOGINFO apache::geode::client::Log::info
#define LOGWARN apache::geode::client::Log::warning
#define LOGERROR apache::geode::client::Log::error

#endif  // GEODE_LOG_H_
