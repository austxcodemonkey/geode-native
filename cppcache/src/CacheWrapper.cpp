#include "geode/CacheWrapper.hpp"

#include <iostream>

#include "geode/RegionShortcut.hpp"

using apache::geode::client::Cache;

void DestroyCache(void* cache) {
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  std::cout << __FUNCTION__ << ": Destroying cache wrapper " << cacheWrapper
            << std::endl;
  delete cacheWrapper;
}

bool Cache_GetPdxIgnoreUnreadFields(void* cache) {
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  return cacheWrapper->getPdxIgnoreUnreadFields();
}

bool Cache_GetPdxReadSerialized(void* cache) {
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  return cacheWrapper->getPdxReadSerialized();
}

void* Cache_GetPoolManager(void* cache) {
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  return cacheWrapper->getPoolManager();
}

void* Cache_CreateRegionFactory(void* cache, int32_t regionType) {
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  apache::geode::client::RegionShortcut regionShortcut =
      static_cast<apache::geode::client::RegionShortcut>(regionType);
  return cacheWrapper->createRegionFactory(regionShortcut);
}

CacheWrapper::CacheWrapper(apache::geode::client::Cache cache)
    : cache_(std::move(cache)) {}

bool CacheWrapper::getPdxIgnoreUnreadFields() {
  return cache_.getPdxIgnoreUnreadFields();
}

bool CacheWrapper::getPdxReadSerialized() {
  return cache_.getPdxReadSerialized();
}

PoolManagerWrapper* CacheWrapper::getPoolManager() {
  return new PoolManagerWrapper(cache_.getPoolManager());
}

RegionFactoryWrapper* CacheWrapper::createRegionFactory(
    apache::geode::client::RegionShortcut regionShortcut) {
  return new RegionFactoryWrapper(cache_.createRegionFactory(regionShortcut));
}
