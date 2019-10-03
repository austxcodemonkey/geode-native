#include "geode/CacheWrapper.hpp"

#include "geode/RegionShortcut.hpp"
#include "util/Log.hpp"

using apache::geode::client::Cache;

void DestroyCache(void* cache) {
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  LOGDEBUG("%s: destroying cache %p", __FUNCTION__, cache);
  delete cacheWrapper;
}

bool Cache_GetPdxIgnoreUnreadFields(void* cache) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  return cacheWrapper->getPdxIgnoreUnreadFields();
}

bool Cache_GetPdxReadSerialized(void* cache) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  return cacheWrapper->getPdxReadSerialized();
}

void* Cache_GetPoolManager(void* cache) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  return cacheWrapper->getPoolManager();
}

void* Cache_CreateRegionFactory(void* cache, int32_t regionType) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  apache::geode::client::RegionShortcut regionShortcut =
      static_cast<apache::geode::client::RegionShortcut>(regionType);
  return cacheWrapper->createRegionFactory(regionShortcut);
}

const char* Cache_GetName(void* cache) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  return cacheWrapper->getName();
}

void Cache_Close(void* cache, bool keepalive) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  cacheWrapper->close(keepalive);
}

bool Cache_IsClosed(void* cache) {
  LOGDEBUG("%s: cache=%p", __FUNCTION__, cache);
  CacheWrapper* cacheWrapper = static_cast<CacheWrapper*>(cache);
  return cacheWrapper->isClosed();
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

const char* CacheWrapper::getName() { return cache_.getName().c_str(); }

void CacheWrapper::close(bool keepalive) { cache_.close(keepalive); }

bool CacheWrapper::isClosed() { return cache_.isClosed(); }
