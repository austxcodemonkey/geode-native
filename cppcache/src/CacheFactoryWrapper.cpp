#include "geode/CacheFactoryWrapper.hpp"

#include <iostream>

#include "geode/CacheWrapper.hpp"

using apache::geode::client::CacheFactory;

void* CreateCacheFactory() {
  CacheFactory* cacheFactory = new CacheFactory();
  std::cout << __FUNCTION__ << ": created factory " << cacheFactory
            << std::endl;
  return cacheFactory;
};

void* CacheFactory_CreateCache(void* factory) {
  auto cacheFactory = static_cast<CacheFactory*>(factory);
  CacheWrapper* cacheWrapper = new CacheWrapper(cacheFactory->create());
  std::cout << __FUNCTION__ << ": created cache wrapper " << cacheWrapper
            << std::endl;
  return cacheWrapper;
}

const char* CacheFactory_GetVersion(void* factory) {
  auto cacheFactory = static_cast<CacheFactory*>(factory);
  return cacheFactory->getVersion().c_str();
}

const char* CacheFactory_GetProductDescription(void* factory) {
  auto cacheFactory = static_cast<CacheFactory*>(factory);
  return cacheFactory->getProductDescription().c_str();
}

void CacheFactory_SetPdxIgnoreUnreadFields(void* factory,
                                           bool pdxIgnoreUnreadFields) {
  auto cacheFactory = static_cast<CacheFactory*>(factory);
  auto ignoreUnreadFields = pdxIgnoreUnreadFields ? "true" : "false";
  std::cout << __FUNCTION__ << ": " << ignoreUnreadFields << std::endl;
  cacheFactory->setPdxIgnoreUnreadFields(pdxIgnoreUnreadFields);
}

void CacheFactory_SetAuthInitialize(void* factory, void* authInitialize) {
  CacheFactory* cacheFactory = static_cast<CacheFactory*>(factory);
  std::cout << __FUNCTION__ << ": (factory, authinit) = (" << cacheFactory
            << ", " << authInitialize << ")" << std::endl;
}

void CacheFactory_SetPdxReadSerialized(void* factory, bool pdxReadSerialized) {
  auto cacheFactory = static_cast<CacheFactory*>(factory);
  auto readSerialized = pdxReadSerialized ? "true" : "false";
  std::cout << __FUNCTION__ << ": " << readSerialized << std::endl;
  cacheFactory->setPdxReadSerialized(pdxReadSerialized);
}

void CacheFactory_SetProperty(void* factory, const char* key,
                              const char* value) {
  std::cout << __FUNCTION__ << ": (k, v) = (" + std::string(key) << ", "
            << std::string(value) << ")" << std::endl;
  auto cacheFactory = static_cast<CacheFactory*>(factory);
  cacheFactory->set(key, value);
}

void DestroyCacheFactory(void* factory) {
  CacheFactory* cacheFactory = static_cast<CacheFactory*>(factory);
  std::cout << __FUNCTION__ << ": deleting factory " << cacheFactory
            << std::endl;
  delete cacheFactory;
}
