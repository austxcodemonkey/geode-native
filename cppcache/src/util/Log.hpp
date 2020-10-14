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

const std::shared_ptr<spdlog::logger>& getCurrentLogger();

APACHE_GEODE_EXPORT void LoggyMcLogFace(const char*, ...);
APACHE_GEODE_EXPORT void LoggyMcLogFace(const std::string&, ...);

APACHE_GEODE_EXPORT void LogClose();
APACHE_GEODE_EXPORT void LogInit(
    apache::geode::client::LogLevel = apache::geode::client::LogLevel::Config,
    std::string logFilename = "", uint32_t logFileSizeLimit = 0,
    uint32_t logDiskSpaceLimit = 0);
APACHE_GEODE_EXPORT void LogSetLevel(apache::geode::client::LogLevel);
APACHE_GEODE_EXPORT apache::geode::client::LogLevel LogLevelFromString(
    const std::string&);
APACHE_GEODE_EXPORT std::string StringFromLogLevel(
    apache::geode::client::LogLevel);

void LogDebug(const char* format, ...);
void LogDebug(const std::string& msg);
void LogFinest(const char* format, ...);
void LogFinest(const std::string& msg);
void LogFiner(const char* format, ...);
void LogFiner(const std::string& msg);
void LogFine(const char* format, ...);
void LogFine(const std::string& msg);
void LogConfig(const char* format, ...);
void LogConfig(const std::string& msg);
void LogInfo(const char* format, ...);
void LogInfo(const std::string& msg);
void LogWarning(const char* format, ...);
void LogWarning(const std::string& msg);
void LogError(const char* format, ...);
void LogError(const std::string& msg);

#define LOGDEBUG LogDebug
#define LOGFINEST LogFinest
#define LOGFINER LogFiner
#define LOGFINE LogFine
#define LOGCONFIG LogConfig
#define LOGINFO LogInfo
#define LOGWARN LogWarning
#define LOGERROR LogError

#endif  // GEODE_LOG_H_
