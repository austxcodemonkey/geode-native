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

#include <geode/internal/geode_globals.hpp>
#include <geode/util/LogLevel.hpp>

APACHE_GEODE_EXPORT void LoggyMcLogFace(const char*, ...) {}
APACHE_GEODE_EXPORT void LoggyMcLogFace(const std::string&, ...) {}

APACHE_GEODE_EXPORT void LogClose() {}
APACHE_GEODE_EXPORT void LogInit(...) {}
APACHE_GEODE_EXPORT void LogSetLevel(apache::geode::client::LogLevel) {}
APACHE_GEODE_EXPORT apache::geode::client::LogLevel logLevelFromString(
    const std::string&) {
  return apache::geode::client::LogLevel::None;
}
APACHE_GEODE_EXPORT std::string stringFromLogLevel(
    apache::geode::client::LogLevel) {
  return "Unknown";
}
