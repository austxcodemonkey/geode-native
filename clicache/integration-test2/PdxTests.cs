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
using System.Collections.Generic;
using System.IO;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{

  [Trait("Category", "Integration")]
  public class PdxTests : IDisposable
  {
    private GeodeServer geodeServer;
    private CacheXml cacheXml;
    public PdxTests()
    {
      geodeServer = new GeodeServer(new List<string>{"REGION_NAME", "testRegion2", "testRegion1"});
      cacheXml = new CacheXml(new FileInfo("cache.xml"), geodeServer);
    }

    public void Dispose()
    {
      cacheXml.Dispose();
      geodeServer.Dispose();
    }

    [Fact]
    public void NobodyKnowsWhatBug866IsAnyMoreButThisTestsIt()
    {
      var properties = Properties<string, string>.Create();
      properties.Insert("name", "Client-1");
//      properties.Insert("ssl-enabled", "true");
//      properties.Insert("ssl-keystore", "integration-test/keystore/client_keystore.pem");
//      properties.Insert("ssl-truststore", "integration-test/keystore/client_truststore.pem");
      var cacheFactory = new CacheFactory(properties);
      var cache = cacheFactory.Create();

      cache.InitializeDeclarativeCache(cacheXml.File.FullName);

//      var poolFactory = cache.GetPoolFactory();
//      poolFactory.SetSubscriptionEnabled(true);
//      poolFactory.AddLocator("localhost", geodeServer.LocatorPort);
//      poolFactory.Create("default");

      var distRegionAck = cache.GetRegion<string, string>("testRegion1");
//      var distRegionNoAck = cache.GetRegion<object, object>("DistRegionNoAck");

      distRegionAck.Put("key-0", "AAAAA", null);
      var clientNameOne = distRegionAck.Get("clientName1", null);

      Assert.Equal("Client-1", clientNameOne);
    }
  }
}