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

using System;
using System.IO;
using Xunit;
using System.Diagnostics;
using System.Threading;
using Xunit.Abstractions;
using System.Collections.Generic;

namespace Apache.Geode.Client.IntegrationTests
{
    public class SimpleCacheListener : ICacheListener<int, string>
    {
        public void AfterCreate(EntryEvent<int, string> ev)
        {
            CreateCount++;
        }

        public void AfterDestroy(EntryEvent<int, string> ev)
        {
            DestroyCount++;
        }

        public void AfterInvalidate(EntryEvent<int, string> ev)
        {
            InvalidateCount++;
        }

        public void AfterRegionClear(RegionEvent<int, string> ev)
        {
        }

        public void AfterRegionDestroy(RegionEvent<int, string> ev)
        {
        }

        public void AfterRegionDisconnected(IRegion<int, string> region)
        {
        }

        public void AfterRegionInvalidate(RegionEvent<int, string> ev)
        {
        }

        public void AfterRegionLive(RegionEvent<int, string> ev)
        {
        }

        public void AfterUpdate(EntryEvent<int, string> ev)
        {
            UpdateCount++;
        }

        public void Close(IRegion<int, string> region)
        {
        }
        
        public int CreateCount { get; set; }
        public int UpdateCount { get; set; }
        public int DestroyCount { get; set; }
        public int InvalidateCount { get; set; }
    }

    [Trait("Category", "Integration")]
    public class RegisterKeysTest : TestBase
    {
        public RegisterKeysTest(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
        }

        IRegion<int, string> SetupCachingProxyRegion(Cache cache, string poolName = "default") {
          var region = cache.CreateRegionFactory(RegionShortcut.CACHING_PROXY)
                            .SetPoolName(poolName)
                            .Create<int, string>("registerKeysTest");

          return region;
        }

        [Fact]
        public void RegisterUnregisterAndTest()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.True(cluster.Start());
                Assert.Equal(0, cluster.Gfsh.create()
                    .region()
                    .withName("registerKeysTest")
                    .withType("REPLICATE")
                    .execute());

                var cache = cluster.CreateCache();
                try
                {
                    var poolFactory =
                        cache.GetPoolManager().CreateFactory().SetSubscriptionEnabled(true);
                    cluster.ApplyLocators(poolFactory);
                    poolFactory.Create("registerKeysPool");
                    var region = SetupCachingProxyRegion(cache);
                    var interested = new List<int> {  123456 };
                    region.GetSubscriptionService().RegisterAllKeys();
                    var listener = new SimpleCacheListener();
                    region.AttributesMutator.SetCacheListener(listener);

                    region.Put(123456, "foo");
                    region.Put(123456, "bar");
                    region.Put(123456, "baz");
                    region.Put(123456, "qux");

                    region.GetSubscriptionService().UnregisterKeys(interested);
                    var stillInterested = region.GetSubscriptionService().GetInterestList();
                    Assert.Equal(stillInterested.Count, 0);

                    Assert.Equal(listener.CreateCount, 1);
                    Assert.Equal(listener.UpdateCount, 3);
                    Assert.Equal(listener.InvalidateCount, 0);
                    Assert.Equal(listener.DestroyCount, 0);

                    region.Put(123456, "quux");
                    Assert.Equal(listener.UpdateCount, 3);
                }
                finally
                {
                    cache.Close();
                }
            }
        }

        [Fact]
        public void DataSerializableQueryHaveCorrectValues()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.Equal(cluster.Start(), true);
                Assert.Equal(0, cluster.Gfsh.deploy()
                    .withJar(Config.JavaobjectJarPath)
                    .execute());
                Assert.Equal(0, cluster.Gfsh.create()
                    .region()
                    .withName("cqTestRegion")
                    .withType("REPLICATE")
                    .execute());

                Assert.Equal(0, cluster.Gfsh.executeFunction()
                    .withId("InstantiateDataSerializable")
                    .withMember("DataSerializableQueryHaveCorrectValues_server_0")
                    .execute());

                var cache = cluster.CreateCache();
                try {
                cache.TypeRegistry.RegisterType(Position.CreateDeserializable, 22);

                var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
                    .SetPoolName("default");

                var region = regionFactory.Create<string, Position>("cqTestRegion");

                Debug.WriteLine("Putting and changing Position objects in the region");
                var order1 = new Position("GOOG", 23);
                var order2 = new Position("IBM", 37);
                var order3 = new Position("PVTL", 101);

                region.Put("order1", order1);
                var Value = region["order1"];

                region.Put("order2", order2);

                order1.SharesOutstanding = 55;
                region.Put("order1", order1);

                order2.SharesOutstanding = 77;
                region.Put("order2", order2);

                order2.SharesOutstanding = 11;
                region.Put("order2", order2);

                region.Remove("order1");

                region.Put("order3", order3);

                var results = region.Query<Position>("SELECT * FROM /cqTestRegion WHERE sharesOutstanding > 50");
                Assert.Equal(results.Size, 1UL);
                Assert.Equal(results[0].SecId, "PVTL");

                region.Clear();
                }
                finally
                {
                    cache.Close();
                }
            }
        }
    }
}
