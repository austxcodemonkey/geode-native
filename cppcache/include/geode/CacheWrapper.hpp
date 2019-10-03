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
APACHE_GEODE_EXPORT const char* Cache_GetName(void* cache);

APACHE_GEODE_EXPORT void Cache_Close(void* cache, bool keepalive);

APACHE_GEODE_EXPORT bool Cache_IsClosed(void* cache);
}

class CacheWrapper {
 public:
  CacheWrapper(apache::geode::client::Cache cache);

  bool getPdxIgnoreUnreadFields();

  bool getPdxReadSerialized();

  PoolManagerWrapper* getPoolManager();

  RegionFactoryWrapper* createRegionFactory(
      apache::geode::client::RegionShortcut regionShortcut);

  const char* getName();

  void close(bool keepalive);

  bool isClosed();

 private:
  apache::geode::client::Cache cache_;
};

#endif  // NATIVECLIENT_CACHEWRAPPER_H
