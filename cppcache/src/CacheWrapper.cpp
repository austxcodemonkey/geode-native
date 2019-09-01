#include "geode/CacheWrapper.hpp"

#include <iostream>

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

CacheWrapper::CacheWrapper(apache::geode::client::Cache cache)
    : cache_(std::move(cache)) {}

bool CacheWrapper::getPdxIgnoreUnreadFields() {
  return cache_.getPdxIgnoreUnreadFields();
}

bool CacheWrapper::getPdxReadSerialized() {
  return cache_.getPdxReadSerialized();
}
