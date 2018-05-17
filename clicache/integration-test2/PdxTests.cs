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
using PdxTests;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{
  [Trait("Category", "Integration")]
  public class PdxTests : IDisposable
  {
    private readonly GeodeServer _geodeServer;
    private readonly Cache _cacheOne;
    private readonly Cache _cacheTwo;
    private readonly IRegion<object, object> _distRegionAckFromCacheOne;
    private readonly IRegion<object, object> _distRegionAckFromCacheTwo;

    private const string Testpool1Name = "__TESTPOOL1_";
    private readonly List<string> _defaultRegionNames = new List<string> {"DistRegionAck"};

    public PdxTests()
    {
      _geodeServer = new GeodeServer()
        .SetRegionNames(_defaultRegionNames)
        .Execute();

      var cacheFactory = new CacheFactory();
      _cacheOne = cacheFactory.Create();
      _cacheTwo = cacheFactory.Create();

      _cacheOne.GetPoolFactory()
        .AddLocator("localhost", _geodeServer.LocatorPort)
        .Create(Testpool1Name);

      _cacheTwo.GetPoolFactory()
        .AddLocator("localhost", _geodeServer.LocatorPort)
        .Create(Testpool1Name);

      _distRegionAckFromCacheOne = _cacheOne.CreateRegionFactory(RegionShortcut.PROXY)
        .SetPoolName(Testpool1Name).Create<object, object>(_defaultRegionNames[0]);

      _distRegionAckFromCacheTwo = _cacheTwo.CreateRegionFactory(RegionShortcut.PROXY)
        .SetPoolName(Testpool1Name).Create<object, object>(_defaultRegionNames[0]);
    }

    public void Dispose()
    {
      _geodeServer.Dispose();
      _cacheOne.Close();
      _cacheTwo.Close();
    }

    [Fact]
    public void PutAndGetOnePdxType()
    {
      _cacheOne.TypeRegistry.RegisterPdxType(PdxType.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(PdxType.CreateDeserializable);

      var expectedPdxType = new PdxType();
      _distRegionAckFromCacheOne.Put(1, expectedPdxType, null);

      var actualPdxTypeFromCacheOne = _distRegionAckFromCacheOne.Get(1, null);
      var actualPdxTypeFromCacheTwo = _distRegionAckFromCacheTwo.Get(1, null);

      Assert.Equal(expectedPdxType, actualPdxTypeFromCacheOne);
      Assert.Equal(expectedPdxType, actualPdxTypeFromCacheTwo);
      Assert.False(_cacheOne.GetPdxReadSerialized());
    }

    [Fact]
    public void PutAndGetManyPdxTypes()
    {
      _cacheOne.TypeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
      _cacheOne.TypeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
      _cacheOne.TypeRegistry.RegisterPdxType(PdxTypes3.CreateDeserializable);
      _cacheOne.TypeRegistry.RegisterPdxType(PdxTypes4.CreateDeserializable);
      _cacheOne.TypeRegistry.RegisterPdxType(PdxTypes5.CreateDeserializable);
      _cacheOne.TypeRegistry.RegisterPdxType(PdxTypes6.CreateDeserializable);
      _cacheOne.TypeRegistry.RegisterPdxType(PdxTypes7.CreateDeserializable);
      _cacheOne.TypeRegistry.RegisterPdxType(PdxTypes8.CreateDeserializable);
      _cacheOne.TypeRegistry.RegisterPdxType(PdxTypes9.CreateDeserializable);
      _cacheOne.TypeRegistry.RegisterPdxType(PortfolioPdx.CreateDeserializable);
      _cacheOne.TypeRegistry.RegisterPdxType(PositionPdx.CreateDeserializable);
      _cacheOne.TypeRegistry.RegisterPdxType(AllPdxTypes.Create);

      _cacheTwo.TypeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(PdxTypes3.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(PdxTypes4.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(PdxTypes5.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(PdxTypes6.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(PdxTypes7.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(PdxTypes8.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(PdxTypes9.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(PortfolioPdx.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(PositionPdx.CreateDeserializable);
      _cacheTwo.TypeRegistry.RegisterPdxType(AllPdxTypes.Create);

      var expectedPdxType1 = new PdxTypes1();
      _distRegionAckFromCacheOne.Put(11, expectedPdxType1, null);
      var actualPdxType1FromCacheOne = (PdxTypes1) _distRegionAckFromCacheOne.Get(11, null);
      var actualPdxType1FromCacheTwo = (PdxTypes1) _distRegionAckFromCacheTwo.Get(11, null);
      Assert.Equal(expectedPdxType1, actualPdxType1FromCacheOne);
      Assert.Equal(expectedPdxType1, actualPdxType1FromCacheTwo);

      var expectedPdxType2 = new PdxTypes2();
      _distRegionAckFromCacheOne.Put(12, expectedPdxType2, null);
      var actualPdxType2FromCacheOne = (PdxTypes2) _distRegionAckFromCacheOne.Get(12, null);
      var actualPdxType2FromCacheTwo = (PdxTypes2) _distRegionAckFromCacheTwo.Get(12, null);
      Assert.Equal(expectedPdxType2, actualPdxType2FromCacheOne);
      Assert.Equal(expectedPdxType2, actualPdxType2FromCacheTwo);

      var expectedPdxType3 = new PdxTypes3();
      _distRegionAckFromCacheOne.Put(13, expectedPdxType3, null);
      var actualPdxType3FromCacheOne = (PdxTypes3) _distRegionAckFromCacheOne.Get(13, null);
      var actualPdxType3FromCacheTwo = (PdxTypes3) _distRegionAckFromCacheTwo.Get(13, null);
      Assert.Equal(expectedPdxType3, actualPdxType3FromCacheOne);
      Assert.Equal(expectedPdxType3, actualPdxType3FromCacheTwo);

      var expectedPdxType4 = new PdxTypes4();
      _distRegionAckFromCacheOne.Put(14, expectedPdxType4, null);
      var actualPdxType4FromCacheOne = (PdxTypes4) _distRegionAckFromCacheOne.Get(14, null);
      var actualPdxType4FromCacheTwo = (PdxTypes4) _distRegionAckFromCacheTwo.Get(14, null);
      Assert.Equal(expectedPdxType4, actualPdxType4FromCacheOne);
      Assert.Equal(expectedPdxType4, actualPdxType4FromCacheTwo);

      var expectedPdxType5 = new PdxTypes5();
      _distRegionAckFromCacheOne.Put(15, expectedPdxType5, null);
      var actualPdxType5FromCacheOne = (PdxTypes5) _distRegionAckFromCacheOne.Get(15, null);
      var actualPdxType5FromCacheTwo = (PdxTypes5) _distRegionAckFromCacheTwo.Get(15, null);
      Assert.Equal(expectedPdxType5, actualPdxType5FromCacheOne);
      Assert.Equal(expectedPdxType5, actualPdxType5FromCacheTwo);

      var expectedPdxType6 = new PdxTypes6();
      _distRegionAckFromCacheOne.Put(16, expectedPdxType6, null);
      var actualPdxType6FromCacheOne = (PdxTypes6) _distRegionAckFromCacheOne.Get(16, null);
      var actualPdxType6FromCacheTwo = (PdxTypes6) _distRegionAckFromCacheTwo.Get(16, null);
      Assert.Equal(expectedPdxType6, actualPdxType6FromCacheOne);
      Assert.Equal(expectedPdxType6, actualPdxType6FromCacheTwo);

      var expectedPdxType7 = new PdxTypes7();
      _distRegionAckFromCacheOne.Put(17, expectedPdxType7, null);
      var actualPdxType7FromCacheOne = (PdxTypes7) _distRegionAckFromCacheOne.Get(17, null);
      var actualPdxType7FromCacheTwo = (PdxTypes7) _distRegionAckFromCacheTwo.Get(17, null);
      Assert.Equal(expectedPdxType7, actualPdxType7FromCacheOne);
      Assert.Equal(expectedPdxType7, actualPdxType7FromCacheTwo);

      var expectedPdxType8 = new PdxTypes8();
      _distRegionAckFromCacheOne.Put(18, expectedPdxType8, null);
      var actualPdxType8FromCacheOne = (PdxTypes8) _distRegionAckFromCacheOne.Get(18, null);
      var actualPdxType8FromCacheTwo = (PdxTypes8) _distRegionAckFromCacheTwo.Get(18, null);
      Assert.Equal(expectedPdxType8, actualPdxType8FromCacheOne);
      Assert.Equal(expectedPdxType8, actualPdxType8FromCacheTwo);

      var expectedPdxType9 = new PdxTypes9();
      _distRegionAckFromCacheOne.Put(19, expectedPdxType9, null);
      var actualPdxType9FromCacheOne = (PdxTypes9) _distRegionAckFromCacheOne.Get(19, null);
      var actualPdxType9FromCacheTwo = (PdxTypes9) _distRegionAckFromCacheTwo.Get(19, null);
      Assert.Equal(expectedPdxType9, actualPdxType9FromCacheOne);
      Assert.Equal(expectedPdxType9, actualPdxType9FromCacheTwo);

      var expectedPortfolioPdx = new PortfolioPdx(1001, 10);
      _distRegionAckFromCacheOne.Put(20, expectedPortfolioPdx, null);
      var actualPortfolioPdxFromCacheOne = (PortfolioPdx) _distRegionAckFromCacheOne.Get(20, null);
      var actualPortfolioPdxFromCacheTwo = (PortfolioPdx) _distRegionAckFromCacheTwo.Get(20, null);
//      Assert.Equal(expectedPdxTypef, actualPortfolioPdxFromCacheOne);
//      Assert.Equal(expectedPdxTypef, actualPortfolioPdxFromCacheTwo);

      var expectedPortfolioPdx2 = new PortfolioPdx(1001, 10, new[] {"one", "two", "three"});
      _distRegionAckFromCacheOne.Put(21, expectedPortfolioPdx2, null);
      var actualPortfolioPdx2FromCacheOne = (PortfolioPdx) _distRegionAckFromCacheOne.Get(21, null);
      var actualPortfolioPdx2FromCacheTwo = (PortfolioPdx) _distRegionAckFromCacheTwo.Get(21, null);
//      Assert.Equal(expectedPdxTypef2, actualPortfolioPdx2FromCacheOne);
//      Assert.Equal(expectedPdxTypef2, actualPortfolioPdx2FromCacheTwo);

      var expectedPdxType10 = new PdxTypes10();
      _distRegionAckFromCacheOne.Put(22, expectedPdxType10, null);
      var actualPdxType10FromCacheOne = (PdxTypes10) _distRegionAckFromCacheOne.Get(22, null);
      var actualPdxType10FromCacheTwo = (PdxTypes10) _distRegionAckFromCacheTwo.Get(22, null);
      Assert.Equal(expectedPdxType10, actualPdxType10FromCacheOne);
      Assert.Equal(expectedPdxType10, actualPdxType10FromCacheTwo);

      var expectedAllPdxTypes = new AllPdxTypes(true);
      _distRegionAckFromCacheOne.Put(23, expectedAllPdxTypes, null);
      var actualAllPdxTypesFromCacheOne = (AllPdxTypes) _distRegionAckFromCacheOne.Get(23, null);
      var actualAllPdxTypesFromCacheTwo = (AllPdxTypes) _distRegionAckFromCacheTwo.Get(23, null);
      Assert.Equal(expectedAllPdxTypes, actualAllPdxTypesFromCacheOne);
      Assert.Equal(expectedAllPdxTypes, actualAllPdxTypesFromCacheTwo);
    }

    [Fact]
    public void DataOutputAdvance()
    {
      _cacheOne.TypeRegistry.RegisterPdxType(MyClass.Create);
      _cacheOne.TypeRegistry.RegisterPdxType(MyClasses.Create);

      var expectedMyClasses = new MyClasses("1", 1000);
      _distRegionAckFromCacheOne.Put(1, expectedMyClasses, null);
      var acutalMyClasses = _distRegionAckFromCacheOne.Get(1, null);

      Assert.Equal(expectedMyClasses, acutalMyClasses);
    }

    private class MyClasses : IPdxSerializable
    {
      [PdxIdentityField] private string _key;
      private List<object> _children;

      private MyClasses()
      {
      }

      public MyClasses(string key, int nClasses)
      {
        _key = key;
        _children = new List<object>(nClasses);
        for (var i = 0; i < nClasses; i++)
        {
          var my = new MyClass(i);
          _children.Add(my);
        }
      }

      public static IPdxSerializable Create()
      {
        return new MyClasses();
      }

      public override bool Equals(object obj)
      {
        var other = obj as MyClasses;
        if (other == null)
          return false;

        return _children.Count == other._children.Count;
      }

      public override int GetHashCode()
      {
        return base.GetHashCode();
      }

      #region IPdxSerializable Members

      public void FromData(IPdxReader reader)
      {
        _key = reader.ReadString("Key");
        _children = (List<object>) (reader.ReadObject("Children"));
      }

      public void ToData(IPdxWriter writer)
      {
        writer.WriteString("Key", _key);
        writer.WriteObject("Children", _children);
      }

      #endregion
    }

    private class MyClass : IPdxSerializable
    {
      [PdxIdentityField] private string _key;
      private int _secKey;
      private double _shareQuantity;
      private double _cost;
      private double _price;
      private int _settleSecKey;
      private double _settleFxRate;
      private double _valueBasis;
      private double _openDate;
      private double _strategy;

      private MyClass()
      {
      }

      public MyClass(int key)
      {
        _key = key.ToString();
        _secKey = key;
        _shareQuantity = key * 9278;
        _cost = _shareQuantity * 100;
        _price = _cost * 10;
        _settleSecKey = _secKey + 100000;
        _settleFxRate = _price * 1.5;
        _valueBasis = 1.5;
        _openDate = 100000;
        _strategy = 3.6;
      }

      public static IPdxSerializable Create()
      {
        return new MyClass();
      }

      #region IPdxSerializable Members

      public void FromData(IPdxReader reader)
      {
        _key = reader.ReadString("Key");
        _secKey = reader.ReadInt("SecKey");
        _shareQuantity = reader.ReadDouble("ShareQuantity");
        _cost = reader.ReadDouble("Cost");
        _price = reader.ReadDouble("Price");
        _settleSecKey = reader.ReadInt("SettleSecKey");
        _settleFxRate = reader.ReadDouble("SettleFxRate");
        _valueBasis = reader.ReadDouble("ValueBasis");
        _openDate = reader.ReadDouble("OpenDate");
        _strategy = reader.ReadDouble("Strategy");
      }

      public void ToData(IPdxWriter writer)
      {
        writer.WriteString("Key", _key);
        writer.WriteInt("SecKey", _secKey);
        writer.WriteDouble("ShareQuantity", _shareQuantity);
        writer.WriteDouble("Cost", _cost);
        writer.WriteDouble("Price", _price);
        writer.WriteInt("SettleSecKey", _settleSecKey);
        writer.WriteDouble("SettleFxRate", _settleFxRate);
        writer.WriteDouble("ValueBasis", _valueBasis);
        writer.WriteDouble("OpenDate", _openDate);
        writer.WriteDouble("Strategy", _strategy);
      }

      #endregion
    }
  }
}