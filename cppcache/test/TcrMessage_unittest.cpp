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

#include <iostream>

#include "gtest/gtest.h"

#include <geode/CqState.hpp>
#include <geode/CacheFactory.hpp>
#include <TcrMessage.hpp>
#include "ByteArrayFixture.hpp"
#include "CacheImpl.hpp"
#include "SerializationRegistry.hpp"

namespace {

using namespace apache::geode::client;

class TestCacheImpl : public CacheImpl {
public:
 TestCacheImpl(Cache* cache, const std::shared_ptr<Properties>& dsProps,
            bool ignorePdxUnreadFields, bool readPdxSerialized,
            const std::shared_ptr<AuthInitialize>& authInitialize)
   : CacheImpl(cache, dsProps, ignorePdxUnreadFields, readPdxSerialized, authInitialize) {
 }

  std::shared_ptr<SerializationRegistry> getSerializationRegistry() const override {
    return std::make_shared<SerializationRegistry>();
  }
};

class DataOutputUnderTest : public DataOutput {
 public:
  using DataOutput::DataOutput;
  DataOutputUnderTest(CacheImpl* cacheImpl) : DataOutput(cacheImpl, nullptr) {}
};

#define EXPECT_MESSAGE_EQ(e, a) EXPECT_PRED_FORMAT2(assertMessageEqual, e, a)

class TcrMessageTest : public ::testing::Test, protected ByteArrayFixture {
 public:
  ::testing::AssertionResult assertMessageEqual(
      const char *expectedStr, const char *bytesStr, const char *expected,
      const apache::geode::client::TcrMessage &msg) {
    apache::geode::client::ByteArray bytes(
        reinterpret_cast<const uint8_t *>(msg.getMsgData()),
        static_cast<const std::size_t>(msg.getMsgLength()));
    return ByteArrayFixture::assertByteArrayEqual(expectedStr, bytesStr,
                                                  expected, bytes);
  }
protected:
  std::unique_ptr<TestCacheImpl> m_testCacheImpl;

  void SetUp() {
    auto dsProperties = std::make_shared<Properties>();
    std::shared_ptr<AuthInitialize> auth;
    m_testCacheImpl.reset(new TestCacheImpl((Cache*)nullptr, dsProperties, false,
                      false, auth));
  }
};

TEST_F(TcrMessageTest, intializeDefaultConstructor) {
  TcrMessageReply message(true, nullptr);

  EXPECT_EQ(TcrMessage::INVALID, message.getMessageType());
}

TEST_F(TcrMessageTest, testConstructor1MessageDataContentWithDESTROY_REGION) {
  const Region *region = nullptr;
  const std::shared_ptr<Serializable> aCallbackArgument = nullptr;
  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageDestroyRegion message(
      std::move(dout), region,
      aCallbackArgument, messageResponseTimeout, connectionDM);

  EXPECT_EQ(TcrMessage::DESTROY_REGION, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000000B0000003800000003\\h{8}"
      "000000001300494E56414C49445F524547494F4E5F4E414D450000001200030000000000"
      "0000010300000000000000\\h{2}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest, testConstructor1MessageDataContentWithCLEAR_REGION) {
  const Region *region = nullptr;
  const std::shared_ptr<Serializable> aCallbackArgument = nullptr;
  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;
  DataOutputUnderTest dout(m_testCacheImpl.get());

  TcrMessageClearRegion message(
      std::move(dout), region,
      aCallbackArgument, messageResponseTimeout, connectionDM);

  EXPECT_MESSAGE_EQ(
      "000000240000003800000003\\h{8}"
      "000000001300494E56414C49445F524547494F4E5F4E414D450000001200030000000000"
      "0000010300000000000000\\h{2}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest, testQueryConstructorMessageDataCotent) {
  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;
  DataOutputUnderTest dout(m_testCacheImpl.get());

  TcrMessageCloseCQ message(
      std::move(dout),
      "myRegionName", messageResponseTimeout, connectionDM);

  EXPECT_EQ(TcrMessage::CLOSECQ_MSG_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002D0000003100000003FFFFFFFF000000000C006D79526567696F6E4E616D650000"
      "0012000300000000000000010300000000000000\\h{2}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest, testQueryConstructorWithQUERY) {
  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;
  DataOutputUnderTest dout(m_testCacheImpl.get());

  TcrMessageQuery message(
      std::move(dout),
      "aRegionName", messageResponseTimeout, connectionDM);

  EXPECT_EQ(TcrMessage::QUERY, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000220000003000000003FFFFFFFF000000000B0061526567696F6E4E616D65000000"
      "12000300000000000000010300000000000000\\h{2}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest, testQueryConstructorWithSTOPCQ_MSG_TYPE) {
  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;
  DataOutputUnderTest dout(m_testCacheImpl.get());

  TcrMessageStopCQ message(
      std::move(dout),
      "aRegionName", messageResponseTimeout, connectionDM);

  EXPECT_EQ(TcrMessage::STOPCQ_MSG_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002C0000003000000003FFFFFFFF000000000B0061526567696F6E4E616D65000000"
      "12000300000000000000010300000000000000\\h{2}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest, testQueryConstructorWithCLOSECQ_MSG_TYPE) {
  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;
  DataOutputUnderTest dout(m_testCacheImpl.get());

  TcrMessageCloseCQ message(
      std::move(dout),
      "aRegionName", messageResponseTimeout, connectionDM);

  EXPECT_EQ(TcrMessage::CLOSECQ_MSG_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002D0000003000000003FFFFFFFF000000000B0061526567696F6E4E616D65000000"
      "12000300000000000000010300000000000000\\h{2}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest,
       testParameterizedQueryConstructorWithQUERY_WITH_PARAMETERS) {
  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;
  const std::shared_ptr<Serializable> aCallbackArgument = nullptr;
  auto paramList = CacheableVector::create();
  DataOutputUnderTest dout(m_testCacheImpl.get());

  TcrMessageQueryWithParameters message(
      std::move(dout),
      "aRegionName", aCallbackArgument, paramList, messageResponseTimeout,
      connectionDM);

  EXPECT_EQ(TcrMessage::QUERY_WITH_PARAMETERS, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000500000002B00000004FFFFFFFF000000000B0061526567696F6E4E616D65000000"
      "04000000000000000004000000000F0000000400000003E8",
      message);
}

//TEST_F(TcrMessageTest, testConstructorWithCONTAINS_KEY) {
//  TcrMessageContainsKey message(
//      std::move(dout),
//      static_cast<const Region *>(nullptr),
//      CacheableString::create(
//          "mykey"),  // static_cast<const
//                     // std::shared_ptr<CacheableKey>>(nullptr),
//      static_cast<const std::shared_ptr<Serializable>>(nullptr),
//      true,  // isContainsKey
//      static_cast<ThinClientBaseDM *>(nullptr));
//  EXPECT_EQ(TcrMessage::CONTAINS_KEY, message.getMessageType());
//
//  EXPECT_MESSAGE_EQ(
//      "000000260000002E00000003FFFFFFFF000000001300494E56414C49445F524547494F4E"
//      "5F4E414D4500000008015700056D796B6579000000040000000000",
//      message);
//}

TEST_F(TcrMessageTest, testConstructorWithGETDURABLECQS_MSG_TYPE) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageGetDurableCqs message(
      std::move(dout),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GETDURABLECQS_MSG_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ("000000690000000600000001FFFFFFFF00000000010000", message);
}

TEST_F(TcrMessageTest, testConstructor2WithREQUEST) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageRequest message(
      std::move(dout),
      static_cast<const Region *>(nullptr),
      CacheableString::create(
          "mykey"),  // static_cast<const
                     // std::shared_ptr<CacheableKey>>(nullptr),
      static_cast<const std::shared_ptr<Serializable>>(nullptr),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REQUEST, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000000000002500000002FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000008015700056D796B6579",
      message);
}

TEST_F(TcrMessageTest, testConstructor2WithDESTROY) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageDestroy message(
      std::move(dout),
      static_cast<const Region *>(nullptr), CacheableString::create("mykey"),
      static_cast<const std::shared_ptr<CacheableKey>>(nullptr),
      static_cast<const std::shared_ptr<Serializable>>(nullptr),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::DESTROY, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000090000004800000005FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000008015700056D796B6579000000010129000000010129000000120003"
      "000000000000000103000000000000000\\h{1}",
      message);
}

TEST_F(TcrMessageTest, testConstructor2WithINVALIDATE) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageInvalidate message(
      std::move(dout),
      static_cast<const Region *>(nullptr),
      CacheableString::create(
          "mykey"),  // static_cast<const
                     // std::shared_ptr<CacheableKey>>(nullptr),
      static_cast<const std::shared_ptr<Serializable>>(nullptr),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::INVALIDATE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000530000003C00000003FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000008015700056D796B6579000000120003000000000000000103000000"
      "000000000\\h{1}",
      message);
}

TEST_F(TcrMessageTest, testConstructor3WithPUT) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessagePut message(
      std::move(dout),
      static_cast<const Region *>(nullptr), CacheableString::create("mykey"),
      CacheableString::create("myvalue"),
      static_cast<const std::shared_ptr<Serializable>>(nullptr),
      false,  // isDelta
      static_cast<ThinClientBaseDM *>(nullptr),
      false,  // isMetaRegion
      false,  // fullValueAfterDeltaFail
      "myRegionName");

  EXPECT_EQ(TcrMessage::PUT, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000070000005A00000007FFFFFFFF000000000C006D79526567696F6E4E616D650000"
      "0001012900000004000000000000000008015700056D796B657900000002013500000000"
      "0A015700076D7976616C7565000000120003000000000000000103000000000000000\\h"
      "{1}",
      message);
}

TEST_F(TcrMessageTest, testConstructor4) {
  TcrMessageReply message(false,  // decodeAll
                          static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::INVALID, message.getMessageType());
}

TEST_F(TcrMessageTest, testConstructor5WithREGISTER_INTERST_LIST) {
  std::vector<std::shared_ptr<CacheableKey>> keys;
  keys.push_back(CacheableString::create("mykey"));

  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageRegisterInterestList message(
      std::move(dout),
      static_cast<const Region *>(nullptr), keys,
      false,  // isDurable
      false,  // isCacheingEnabled
      false,  // receiveValues
      InterestResultPolicy::NONE, static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REGISTER_INTEREST_LIST, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000180000004200000006FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000003010125000000000100000000000A0141015700056D796B65790000"
      "0001000100000002000000",
      message);
}

TEST_F(TcrMessageTest, testConstructor5WithUNREGISTER_INTERST_LIST) {
  std::vector<std::shared_ptr<CacheableKey>> keys;
  keys.push_back(CacheableString::create("mykey"));
  DataOutputUnderTest dout(m_testCacheImpl.get());

  TcrMessageUnregisterInterestList message(
      std::move(dout),
      static_cast<const Region *>(nullptr), keys,
      false,  // isDurable
      false,  // receiveValues
      InterestResultPolicy::NONE, static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::UNREGISTER_INTEREST_LIST, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000190000003A00000005FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000001000000000001000000000004000000000100000008015700056D79"
      "6B6579",
      message);
}

TEST_F(TcrMessageTest, testConstructorGET_FUNCTION_ATTRIBUTES) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageGetFunctionAttributes message(
      std::move(dout),
      std::string("myFunction"), static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GET_FUNCTION_ATTRIBUTES, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000005B0000000F00000001FFFFFFFF000000000A006D7946756E6374696F6E",
      message);
}

TEST_F(TcrMessageTest, testConstructorKEY_SET) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageKeySet message(
      std::move(dout),
      std::string("myFunctionKeySet"),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::KEY_SET, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000280000001500000001FFFFFFFF0000000010006D7946756E6374696F6E4B657953"
      "6574",
      message);
}

TEST_F(TcrMessageTest, testConstructor6WithCREATE_REGION) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageCreateRegion message(
      std::move(dout),
      "str1",  // TODO: what does this parameter do?!
      "str2",  // TODO: what does this parameter do?!
      false,  // isDurable
      false,  // receiveValues
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::CREATE_REGION, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000001D0000001200000002FFFFFFFF00000000040073747231000000040073747232",
      message);
}

TEST_F(TcrMessageTest, testConstructor6WithREGISTER_INTEREST) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageRegisterInterest message(
      std::move(dout),
      "str1",  // TODO: what does this parameter do?!
      "str2",  // TODO: what does this parameter do?!
      InterestResultPolicy::NONE,
      false,  // isDurable
      false,  // isCacheingEnabled
      false,  // receiveValues
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REGISTER_INTEREST, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000140000003600000007FFFFFFFF0000000004007374723100000004000000000100"
      "0000030101250000000001000000000004007374723200000001000100000002000000",
      message);
}

TEST_F(TcrMessageTest, testConstructor6WithUNREGISTER_INTEREST) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageUnregisterInterest message(
      std::move(dout),
      "str1",  // TODO: what does this parameter do?!
      "str2",  // TODO: what does this parameter do?!
      InterestResultPolicy::NONE,
      false,  // isDurable
      false,  // receiveValues
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::UNREGISTER_INTEREST, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000160000002700000005FFFFFFFF0000000004007374723100000004000000000100"
      "0000040073747232000000010000000000010000",
      message);
}

TEST_F(TcrMessageTest, testConstructorGET_PDX_TYPE_BY_ID) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageGetPdxTypeById message(
      std::move(dout), 42,
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GET_PDX_TYPE_BY_ID, message.getMessageType());

  EXPECT_MESSAGE_EQ("0000005C0000000900000001FFFFFFFF0000000004000000002A",
                    message);
}

TEST_F(TcrMessageTest, testConstructorGET_PDX_ENUM_BY_ID) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageGetPdxEnumById message(
      std::move(dout), 42,
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GET_PDX_ENUM_BY_ID, message.getMessageType());

  EXPECT_MESSAGE_EQ("000000620000000900000001FFFFFFFF0000000004000000002A",
                    message);
}

TEST_F(TcrMessageTest, testConstructorGET_PDX_ID_FOR_TYPE) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  std::shared_ptr<Cacheable> myPtr(CacheableString::createDeserializable());
  TcrMessageGetPdxIdForType message(
      std::move(dout), myPtr,
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GET_PDX_ID_FOR_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ("0000005D0000000700000001FFFFFFFF0000000002010000",
                    message);
}

TEST_F(TcrMessageTest, testConstructorADD_PDX_TYPE) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  std::shared_ptr<Cacheable> myPtr(CacheableString::createDeserializable());
  TcrMessageAddPdxType message(
      std::move(dout), myPtr,
      static_cast<ThinClientBaseDM *>(nullptr), 42);

  EXPECT_EQ(TcrMessage::ADD_PDX_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000005E0000001000000002FFFFFFFF000000000201000000000004000000002A",
      message);
}

TEST_F(TcrMessageTest, testConstructorGET_PDX_ID_FOR_ENUM) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageGetPdxIdForEnum message(
      std::move(dout),
      static_cast<std::shared_ptr<Cacheable>>(nullptr),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GET_PDX_ID_FOR_ENUM, message.getMessageType());

  EXPECT_MESSAGE_EQ("000000610000000600000001FFFFFFFF00000000010129", message);
}

TEST_F(TcrMessageTest, testConstructorADD_PDX_ENUM) {
  std::shared_ptr<Cacheable> myPtr(CacheableString::createDeserializable());
  DataOutputUnderTest dout(m_testCacheImpl.get());

  TcrMessageAddPdxEnum message(
      std::move(dout),
      static_cast<std::shared_ptr<Cacheable>>(nullptr),
      static_cast<ThinClientBaseDM *>(nullptr), 42);

  EXPECT_EQ(TcrMessage::ADD_PDX_ENUM, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000600000000F00000002FFFFFFFF0000000001012900000004000000002A",
      message);
}

TEST_F(TcrMessageTest, testConstructorEventId) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageRequestEventValue message(
      std::move(dout),
      static_cast<std::shared_ptr<EventId>>(nullptr));

  EXPECT_EQ(TcrMessage::REQUEST_EVENT_VALUE, message.getMessageType());

  EXPECT_MESSAGE_EQ("000000440000000600000001FFFFFFFF00000000010129", message);
}

TEST_F(TcrMessageTest, testConstructorREMOVE_USER_AUTH) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  DataOutputUnderTest dout2(m_testCacheImpl.get());
  TcrMessageRemoveUserAuth message(
      std::move(dout), true,
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REMOVE_USER_AUTH, message.getMessageType());

  EXPECT_MESSAGE_EQ("0000004E0000000600000001FFFFFFFF00000000010001", message);

  TcrMessageRemoveUserAuth message2(
      std::move(dout2), false,
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REMOVE_USER_AUTH, message2.getMessageType());

  EXPECT_MESSAGE_EQ("0000004E0000000600000001FFFFFFFF00000000010000", message2);
}

TEST_F(TcrMessageTest, testConstructorUSER_CREDENTIAL_MESSAGE) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageUserCredential message(
      std::move(dout),
      static_cast<std::shared_ptr<Properties>>(nullptr),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::USER_CREDENTIAL_MESSAGE, message.getMessageType());
  // this message is currently blank so this should change it if the impl
  // changes
  EXPECT_MESSAGE_EQ("", message);
}

TEST_F(TcrMessageTest, testConstructorGET_CLIENT_PARTITION_ATTRIBUTES) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageGetClientPartitionAttributes message(
      std::move(dout),
      "testClientRegion");

  EXPECT_EQ(TcrMessage::GET_CLIENT_PARTITION_ATTRIBUTES,
            message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000490000001500000001FFFFFFFF00000000100074657374436C69656E7452656769"
      "6F6E",
      message);
}

TEST_F(TcrMessageTest, testConstructorGET_CLIENT_PR_METADATA) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageGetClientPrMetadata message(
      std::move(dout),
      "testClientRegionPRMETA");

  EXPECT_EQ(TcrMessage::GET_CLIENT_PR_METADATA, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000470000001B00000001FFFFFFFF00000000160074657374436C69656E7452656769"
      "6F6E50524D455441",
      message);
}
TEST_F(TcrMessageTest, testConstructorSIZE) {
  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageSize message(
      std::move(dout),
      "testClientRegionSIZE");

  EXPECT_EQ(TcrMessage::SIZE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000510000001900000001FFFFFFFF00000000140074657374436C69656E7452656769"
      "6F6E53495A45",
      message);
}

TEST_F(TcrMessageTest, testConstructorEXECUTE_REGION_FUNCTION_SINGLE_HOP) {
  const Region *region = nullptr;

  auto myHashCachePtr = CacheableHashSet::create();

  std::shared_ptr<Cacheable> myPtr(CacheableString::createDeserializable());

  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageExecuteRegionFunctionSingleHop message(
      std::move(dout),
      "myFuncName", region, myPtr, myHashCachePtr, 2, myHashCachePtr,
      false,  // allBuckets
      std::chrono::milliseconds{1}, static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP,
            message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000004F0000005E00000009FFFFFFFF00000000050002000000010000001300494E5641"
      "4C49445F524547494F4E5F4E414D450000000A006D7946756E634E616D65000000030157"
      "000000000001012900000001000000000004000000000000000004000000000000000002"
      "014200",
      message);

  EXPECT_TRUE(message.hasResult());
}

TEST_F(TcrMessageTest, testConstructorEXECUTE_REGION_FUNCTION) {
  const Region *region = nullptr;

  auto myHashCachePtr = CacheableHashSet::create();
  std::shared_ptr<Cacheable> myCacheablePtr(
      CacheableString::createDeserializable());
  auto myVectPtr = CacheableVector::create();

  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageExecuteRegionFunction testMessage(
      std::move(dout),
      "ExecuteRegion", region, myCacheablePtr, myVectPtr, 2, myHashCachePtr,
      std::chrono::milliseconds{10}, static_cast<ThinClientBaseDM *>(nullptr),
      10);

  EXPECT_EQ(TcrMessage::EXECUTE_REGION_FUNCTION, testMessage.getMessageType());
  // this message is currently blank so this should change it if the impl
  // changes

  EXPECT_MESSAGE_EQ(
      "0000003B0000006100000009FFFFFFFF000000000500020000000A0000001300494E5641"
      "4C49445F524547494F4E5F4E414D450000000D0045786563757465526567696F6E000000"
      "030157000000000001012900000001000A00000004000000000000000004000000000000"
      "000002014200",
      testMessage);

  EXPECT_TRUE(testMessage.hasResult());
}

TEST_F(TcrMessageTest, DISABLED_testConstructorEXECUTE_FUNCTION) {
  std::shared_ptr<Cacheable> myCacheablePtr(
      CacheableString::createDeserializable());

  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageExecuteFunction testMessage(
      std::move(dout),
      "ExecuteFunction", myCacheablePtr, 1,
      static_cast<ThinClientBaseDM *>(nullptr), std::chrono::milliseconds{10});

  EXPECT_EQ(TcrMessage::EXECUTE_FUNCTION, testMessage.getMessageType());

  EXPECT_TRUE(testMessage.hasResult());

  EXPECT_MESSAGE_EQ(
      "0000003E0000002600000003FFFFFFFF000000000500010000000A0000000F0045786563"
      "75746546756E6374696F6E0000000301570000",
      testMessage);
}

TEST_F(TcrMessageTest, testConstructorEXECUTECQ_MSG_TYPE) {
  std::shared_ptr<Cacheable> myCacheablePtr(
      CacheableString::createDeserializable());

  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageExecuteCq testMessage(
      std::move(dout),
      "ExecuteCQ", "select * from /somewhere", CqState::RUNNING, false,
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::EXECUTECQ_MSG_TYPE, testMessage.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002A0000004000000005FFFFFFFF0000000009004578656375746543510000001800"
      "73656C656374202A2066726F6D202F736F6D657768657265000000040000000001000000"
      "010000000000010001",
      testMessage);
}

TEST_F(TcrMessageTest, testConstructorWithGinormousQueryEXECUTECQ_MSG_TYPE) {
  std::shared_ptr<Cacheable> myCacheablePtr(
      CacheableString::createDeserializable());

  std::ostringstream oss;
  oss << "select * from /somewhere s where s.data.id in SET(";
  // Ensure over 64KiB of query string.
  const int n = (((64 * 1024) + 11) / 12);
  for (int i = 0; i < n; ++i) {
    if (0 < i) {
      oss << ',';
    }
    oss << '\'';
    oss.fill('0');
    oss.width(9);
    oss << i;
    oss << '\'';
  }
  oss << ") and s.type in SET('AAA','BBB','CCC','DDD') limit 60000";

  DataOutputUnderTest dout(m_testCacheImpl.get());
  TcrMessageExecuteCq testMessage(
      std::move(dout),
      "ExecuteCQ", oss.str(), CqState::RUNNING, false,
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::EXECUTECQ_MSG_TYPE, testMessage.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002A0001009900000005FFFFFFFF0000000009004578656375746543510001007100"
      "\\h{131298}000000040000000001000000010000000000010001",
      testMessage);
}

TEST_F(TcrMessageTest, testConstructorEXECUTECQ_WITH_IR_MSG_TYPE) {
  std::shared_ptr<Cacheable> myCacheablePtr(
      CacheableString::createDeserializable());
  DataOutputUnderTest dout(m_testCacheImpl.get());

  TcrMessageExecuteCqWithIr testMessage(
      std::move(dout),
      "ExecuteCQWithIr", "select * from /somewhere", CqState::RUNNING, false,
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE,
            testMessage.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002B0000004600000005FFFFFFFF000000000F004578656375746543515769746849"
      "72000000180073656C656374202A2066726F6D202F736F6D657768657265000000040000"
      "000001000000010000000000010001",
      testMessage);
}

}  // namespace
