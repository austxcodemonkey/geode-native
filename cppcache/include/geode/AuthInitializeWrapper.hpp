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

#ifndef AUTHINITIALIZEWRAPPER_H_
#define AUTHINITIALIZEWRAPPER_H_

#include <string>

#include <geode/AuthInitialize.hpp>
#include <geode/Properties.hpp>

extern "C" {
void APACHE_GEODE_EXPORT AuthInitialize_AddProperty(void* properties,
                                                    const char* key,
                                                    const char* value);
}

class AuthInitializeWrapper : public apache::geode::client::AuthInitialize {
 public:
  std::shared_ptr<apache::geode::client::Properties> getCredentials(
      const std::shared_ptr<apache::geode::client::Properties>& securityprops,
      const std::string& /*server*/) override;

  void close() override;

  AuthInitializeWrapper(void (*getCredentials)(void*), void (*close)());

  ~AuthInitializeWrapper() override = default;

 private:
  void (*getCredentials_)(void*);
  void (*close_)();
};

#endif  // AUTHINITIALIZEWRAPPER_H_
