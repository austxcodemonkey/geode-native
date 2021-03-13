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

#include <chrono>
#include <iostream>
#include <thread>

#include <boost/thread/latch.hpp>

#include <gtest/gtest.h>

#include <geode/CqAttributesFactory.hpp>
#include <geode/QueryService.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheRegionHelper.hpp"
#include "CountingListener.hpp"
#include "framework/Cluster.h"

namespace {

using apache::geode::client::Cache;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::Exception;
using apache::geode::client::Pool;
using apache::geode::client::Region;
using apache::geode::client::RegionShortcut;

using apache::geode::client::testing::CountingListener;

const char* REGION_NAME = "DistRegionAck";

Cache createCache() {
  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("statistic-sampling-enabled", "false")
                   .create();

  return cache;
}

typedef struct {
  std::string host;
  int port;
} locatorInfo;

class CachelessTestTask {
 public:
  CachelessTestTask(std::string poolName, std::vector<locatorInfo> locators)
      : poolName_(poolName) {
    cachePtr_ = std::make_shared<Cache>(CacheFactory().create());
    auto poolFactory = cachePtr_->getPoolManager().createFactory();

    for (auto locator : locators) {
      poolFactory.addLocator(locator.host, locator.port);
    }

    poolFactory.setSubscriptionEnabled(true);
    poolFactory.create(poolName_);
  }

  virtual ~CachelessTestTask() = default;

  void createRegion() {
    auto regionFactory =
        cachePtr_->createRegionFactory(RegionShortcut::CACHING_PROXY);
    regionFactory.setPoolName(poolName_);
    if (listener_) {
      regionFactory.setCacheListener(listener_);
    }
    region_ = regionFactory.create(REGION_NAME);
  }

  void setCacheListener(std::shared_ptr<CountingListener> listener) {
    listener_ = listener;
  }

  std::shared_ptr<CountingListener> getCacheListener() { return listener_; }

  void close() {
    ASSERT_TRUE(cachePtr_ != nullptr);
    cachePtr_->close();
  }

  std::shared_ptr<Region> getRegion() { return region_; }

 private:
  std::string poolName_;
  std::shared_ptr<Cache> cachePtr_;
  std::shared_ptr<Region> region_;
  std::shared_ptr<CountingListener> listener_;
};

void testForValueInRegion(const std::shared_ptr<CacheableKey>& key,
                          int expectedValue, std::shared_ptr<Region> region) {
  ASSERT_TRUE(region->containsKey(key));
  auto value = region->get(key);
  int actualValue = std::dynamic_pointer_cast<CacheableInt32>(value)->value();
  ASSERT_EQ(expectedValue, actualValue);
}

TEST(CachelessTest, testCachelessWhateverThatIs) {
  std::string poolName = "__TEST_POOL1__";
  Cluster cluster{LocatorCount{1}, ServerCount{1}};

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName(REGION_NAME)
      .withType("PARTITION")
      .execute();
  std::vector<locatorInfo> locators = {{"localhost", cluster.getLocatorPort()}};

  // Create 4 caches, 2 with listeners and 2 without.  4 copies of region.
  CachelessTestTask taskGenericNoCache(poolName, locators);
  taskGenericNoCache.createRegion();

  // s1p2
  CachelessTestTask taskNoCacheWithListener(poolName, locators);
  taskNoCacheWithListener.setCacheListener(
      std::make_shared<CountingListener>());
  taskNoCacheWithListener.createRegion();

  // s2p1
  CachelessTestTask taskRegionCacheMirror(poolName, locators);
  taskRegionCacheMirror.createRegion();

  // s2p2
  CachelessTestTask taskRegionCache(poolName, locators);
  taskRegionCache.setCacheListener(std::make_shared<CountingListener>());
  taskRegionCache.createRegion();

  // Haven't done anything yet, no one should have received any events
  auto listener = taskNoCacheWithListener.getCacheListener();
  ASSERT_EQ(listener->getCreates(), 0);
  ASSERT_EQ(listener->getUpdates(), 0);
  ASSERT_EQ(listener->getLastKey(), nullptr);
  ASSERT_EQ(listener->getLastValue(), nullptr);

  // Should send 'create' event to the 2 listeners
  taskGenericNoCache.getRegion()->put("key1", 2);

  // Should not send update for key1, because value hasn't changed
  taskRegionCacheMirror.getRegion()->put("key1", 2);

  // Verify value exists in region we didn't call put on
  testForValueInRegion(std::make_shared<CacheableString>("key1"), 2,
                       taskRegionCacheMirror.getRegion());

  // CALL_TASK(CheckEmpty) (s2p2)
  auto region = taskRegionCache.getRegion();
  ASSERT_FALSE(region->containsKey(std::make_shared<CacheableString>("key1")));
  listener = taskRegionCache.getCacheListener();
  ASSERT_EQ(listener->getCreates(), 0);
  ASSERT_EQ(listener->getUpdates(), 0);
  region->put(std::make_shared<CacheableString>("key2"), 1);
  ASSERT_EQ(listener->expectCreates(1), 1);
  ASSERT_EQ(listener->expectUpdates(0), 0);

  // CALL_TASK(CreateKey2Again) (s1p1)
  region = taskGenericNoCache.getRegion();
  ASSERT_FALSE(region->containsKey(std::make_shared<CacheableString>("key2")));
  taskGenericNoCache.getRegion()->put("key2", 2);

  // CALL_TASK(CheckNewValue) (s2p2)
  testForValueInRegion(std::make_shared<CacheableString>("key2"), 1,
                       taskRegionCache.getRegion());
  ASSERT_EQ(taskRegionCache.getCacheListener()->expectCreates(2), 1);

  taskGenericNoCache.close();
  taskNoCacheWithListener.close();
  taskRegionCacheMirror.close();
  taskRegionCache.close();
}

}  // namespace
