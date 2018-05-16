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
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{
  [Trait("Category", "Integration")]
  public class Bug866Tests : IDisposable
  {
    private readonly GeodeServer _geodeServer;

    private const string _CLIENT1NAME = "Client-1";
    private const string _CLIENT2NAME = "Client-2";
    private const string _TESTPOOL1NAME = "__TESTPOOL1_";
    private readonly List<string> _defaultRegionNames = new List<string> {"DistRegionAck", "DistRegionNoAck"};

    public Bug866Tests()
    {
      _geodeServer = new GeodeServer()
        .SetRegionNames(_defaultRegionNames)
        .SetReadSerialized(false)
        .Execute();
    }

    public void Dispose()
    {
      _geodeServer.Dispose();
    }

    [Fact]
    public void NobodyKnowsWhatBug866IsAnyMoreButThisTestsIt()
    {
      var properties = Properties<string, string>.Create();
      properties.Insert("name", _CLIENT1NAME);
      var cacheFactory = new CacheFactory(properties);
      var cache = cacheFactory.Create();

      var poolFactory = cache.GetPoolFactory()
        .SetSubscriptionEnabled(true)
        .AddLocator("localhost", _geodeServer.LocatorPort);
      poolFactory.Create(_TESTPOOL1NAME);

      var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
        .SetPoolName(_TESTPOOL1NAME)
        .SetCloningEnabled(false)
        .SetCachingEnabled(true);
      regionFactory.Create<string, string>(_defaultRegionNames[0]);

      var distRegionAck = cache.GetRegion<string,string>(_defaultRegionNames[0]);

      distRegionAck.Put("key-0", "AAAAA", null);
      var clientNameOne = distRegionAck["clientName1"];

      Assert.Equal(_CLIENT1NAME, clientNameOne);
    }
  }
}