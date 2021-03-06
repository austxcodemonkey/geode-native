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


//#include "../../geode_includes.hpp"
//#include "../../../geode_includes.hpp"
#include "ManagedResultCollector.hpp"
//#include "../../../ISerializable.hpp"


//#include "../ISerializable.hpp"
#include "ManagedString.hpp"
#include "SafeConvert.hpp"
#include "../ExceptionTypes.hpp"
#include <string>

using namespace System;
using namespace System::Text;
using namespace System::Reflection;


namespace apache
{
  namespace geode
  {
    namespace client
    {

      apache::geode::client::ResultCollector* ManagedResultCollectorGeneric::create(const char* assemblyPath,
                                                                                    const char* factoryFunctionName)
      {
        try
        {
          String^ mg_assemblyPath =
            Apache::Geode::Client::ManagedString::Get(assemblyPath);
          String^ mg_factoryFunctionName =
            Apache::Geode::Client::ManagedString::Get(factoryFunctionName);
          String^ mg_typeName = nullptr;
          Int32 dotIndx = -1;

          if (mg_factoryFunctionName == nullptr ||
              (dotIndx = mg_factoryFunctionName->LastIndexOf('.')) < 0)
          {
            std::string ex_str = "ManagedResultCollector: Factory function name '";
            ex_str += factoryFunctionName;
            ex_str += "' does not contain type name";
            throw apache::geode::client::IllegalArgumentException(ex_str.c_str());
          }

          mg_typeName = mg_factoryFunctionName->Substring(0, dotIndx);
          mg_factoryFunctionName = mg_factoryFunctionName->Substring(dotIndx + 1);

          Assembly^ assmb = nullptr;
          try
          {
            assmb = Assembly::Load(mg_assemblyPath);
          }
          catch (System::Exception^)
          {
            assmb = nullptr;
          }
          if (assmb == nullptr)
          {
            std::string ex_str = "ManagedResultCollector: Could not load assembly: ";
            ex_str += assemblyPath;
            throw apache::geode::client::IllegalArgumentException(ex_str.c_str());
          }
          Object^ typeInst = assmb->CreateInstance(mg_typeName, true);
          if (typeInst != nullptr)
          {
            MethodInfo^ mInfo = typeInst->GetType()->GetMethod(mg_factoryFunctionName,
                                                               BindingFlags::Public | BindingFlags::Static | BindingFlags::IgnoreCase);
            if (mInfo != nullptr)
            {
              //Apache::Geode::Client::ResultCollector<Object^>^ managedptr = nullptr;
              Object^ userptr = nullptr;
              try
              {
                throw apache::geode::client::UnsupportedOperationException("Not supported");
                /*managedptr = dynamic_cast<Apache::Geode::Client::ResultCollector<Object^>^>(
                  mInfo->Invoke( typeInst, nullptr ) );*/
                userptr = mInfo->Invoke(typeInst, nullptr);
              }
              catch (System::Exception^ ex)
              {
                Apache::Geode::Client::Log::Debug("{0}: {1}", ex->GetType()->Name, ex->Message);
                userptr = nullptr;
              }
              if (userptr == nullptr)
              {
                std::string ex_str = "ManagedResultCollector: Could not create "
                  "object on invoking factory function [";
                ex_str += factoryFunctionName;
                ex_str += "] in assembly: ";
                ex_str += assemblyPath;
                throw apache::geode::client::IllegalArgumentException(ex_str.c_str());
              }
              //TODO::need to pass proper pointer here
              return new ManagedResultCollectorGeneric(/*(Apache::Geode::Client::ResultCollector<Object^>^) managedptr*/nullptr);
            }
            else
            {
              std::string ex_str = "ManagedResultCollector: Could not load "
                "function with name [";
              ex_str += factoryFunctionName;
              ex_str += "] in assembly: ";
              ex_str += assemblyPath;
              throw apache::geode::client::IllegalArgumentException(ex_str.c_str());
            }
          }
          else
          {
            Apache::Geode::Client::ManagedString typeName(mg_typeName);
            std::string ex_str = "ManagedResultCollector: Could not load type [";
            ex_str += typeName.CharPtr;
            ex_str += "] in assembly: ";
            ex_str += assemblyPath;
            throw apache::geode::client::IllegalArgumentException(ex_str.c_str());
          }
        }
        catch (const apache::geode::client::Exception&)
        {
          throw;
        }
        catch (System::Exception^ ex)
        {
          std::string ex_str = "ManagedResultCollector: Got an exception while "
            "loading managed library: " + marshal_as<std::string>(ex->ToString());
          throw apache::geode::client::IllegalArgumentException(ex_str);
        }
        return NULL;
      }

      void ManagedResultCollectorGeneric::addResult(const std::shared_ptr<Cacheable>& result)
      {
        try {
          Object^ rs = Apache::Geode::Client::TypeRegistry::GetManagedValueGeneric<Object^>(result);
          m_managedptr->AddResult(rs);
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          std::string ex_str = "ManagedResultCollector: Got an exception in"
            "addResult: " + marshal_as<std::string>(ex->ToString());
          throw apache::geode::client::IllegalArgumentException(ex_str);
        }
      }

      std::shared_ptr<CacheableVector> ManagedResultCollectorGeneric::getResult(std::chrono::milliseconds timeout)
      {
        try {
          //array<ISerializable^>^ rs = m_managedptr->GetResult(timeout);
          //std::shared_ptr<apache::geode::client::CacheableVector> rsptr = apache::geode::client::CacheableVector::create();
          //for( int index = 0; index < rs->Length; index++ )
          //{
          //  //std::shared_ptr<apache::geode::client::Cacheable> valueptr(Apache::Geode::Client::Serializable::GetUnmanagedValueGeneric<ISerializable^>(rs[ index]));
          //  std::shared_ptr<apache::geode::client::Cacheable> valueptr (SafeMSerializableConvert(rs[ index]));
          //  rsptr->push_back(valueptr);
          //}
          //return rsptr;
          throw apache::geode::client::IllegalStateException("This should not be get callled.");
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          std::string ex_str = "ManagedResultCollector: Got an exception in"
            "getResult: " + marshal_as<std::string>(ex->ToString());
          throw apache::geode::client::IllegalArgumentException(ex_str);
        }
        return nullptr;
      }
      void ManagedResultCollectorGeneric::endResults()
      {
        try {
          m_managedptr->EndResults();
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          std::string ex_str = "ManagedResultCollector: Got an exception in"
            "endResults: " + marshal_as<std::string>(ex->ToString());
          throw apache::geode::client::IllegalArgumentException(ex_str);
        }
      }
      void ManagedResultCollectorGeneric::clearResults()
      {
        try {
          m_managedptr->ClearResults(/*false*/);
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          std::string ex_str = "ManagedResultCollector: Got an exception in"
            "clearResults: " + marshal_as<std::string>(ex->ToString());
          throw apache::geode::client::IllegalArgumentException(ex_str);
        }
      }
    }  // namespace client
  }  // namespace geode
}  // namespace apache
