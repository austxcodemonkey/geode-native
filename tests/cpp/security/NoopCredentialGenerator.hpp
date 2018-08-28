#pragma once

#ifndef GEODE_SECURITY_NOOPCREDENTIALGENERATOR_H_
#define GEODE_SECURITY_NOOPCREDENTIALGENERATOR_H_

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

#include "CredentialGenerator.hpp"

namespace apache {
namespace geode {
namespace client {
namespace testframework {
namespace security {

class NoopCredentialGenerator : public CredentialGenerator {
 public:
  NoopCredentialGenerator() : CredentialGenerator(ID_NOOP, "NOOP") {
    ;
    ;
  };

  std::string getInitArgs(std::string workingDir, bool) override {
    std::string additionalArgs;
    char* buildDir = ACE_OS::getenv("BUILDDIR");
    if (buildDir && workingDir.length() == 0) {
      workingDir = std::string(buildDir);
      workingDir += std::string("/framework/xml/Security/");
    }
    additionalArgs = " ";

    return additionalArgs;
  }

  std::string getClientAuthInitLoaderFactory() override {
    return "createNoopAuthInitInstance";
  }
  std::string getClientAuthInitLoaderLibrary() override { return "testobject"; }
  std::string getClientAuthenticator() override {
    return "javaobject.NoopAuthenticator.create";
  }
  std::string getClientAuthorizer() override {
    return "javaobject.NoopAccessor.create";
  }
};

}  // namespace security
}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SECURITY_NOOPCREDENTIALGENERATOR_H_
