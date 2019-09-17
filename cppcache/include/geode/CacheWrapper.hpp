//
// Created by Blake Bender on 8/30/19.
//

#ifndef NATIVECLIENT_CACHEWRAPPER_H
#define NATIVECLIENT_CACHEWRAPPER_H
#pragma once

#include "geode/Cache.hpp"
#include "geode/CacheFactory.hpp"
#include "geode/PoolManagerWrapper.hpp"
#include "geode/RegionFactoryWrapper.hpp"

extern "C" {
APACHE_GEODE_EXPORT void DestroyCache(void* cache);

APACHE_GEODE_EXPORT bool Cache_GetPdxIgnoreUnreadFields(void* cache);

APACHE_GEODE_EXPORT bool Cache_GetPdxReadSerialized(void* cache);

APACHE_GEODE_EXPORT void* Cache_GetPoolManager(void* cache);

APACHE_GEODE_EXPORT void* Cache_CreateRegionFactory(void* cache,
                                                    int32_t regionType);
}

class CacheWrapper {
 public:
  CacheWrapper(apache::geode::client::Cache cache);

  bool getPdxIgnoreUnreadFields();

  bool getPdxReadSerialized();

  PoolManagerWrapper* getPoolManager();

  RegionFactoryWrapper* createRegionFactory(
      apache::geode::client::RegionShortcut regionShortcut);

 private:
  apache::geode::client::Cache cache_;
};

#endif  // NATIVECLIENT_CACHEWRAPPER_H
