#include "geode/CacheFactoryWrapper.hpp"

#include <iostream>

using apache::geode::client::CacheFactory;

void* CreateCacheFactory() {
  CacheFactory* cf = new CacheFactory();
  std::cout << __FUNCTION__ << ": created factory " << cf << std::endl;
  return cf;
};

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
  std::cout << __FUNCTION__ << ": " << pdxIgnoreUnreadFields << std::endl;
  cacheFactory->setPdxIgnoreUnreadFields(pdxIgnoreUnreadFields);
}

void CacheFactory_SetAuthInitialize(void* factory, void* authInitialize) {
  //  cacheFactory->setAuthInitialize(authInitialize);
  CacheFactory* cacheFactory = static_cast<CacheFactory*>(factory);
  std::cout << __FUNCTION__ << ": (factory, authinit) = (" << cacheFactory
            << ", " << authInitialize << ")" << std::endl;
}

void CacheFactory_SetPdxReadSerialized(void* factory, bool pdxReadSerialized) {
  std::cout << __FUNCTION__ << ": " << pdxReadSerialized << std::endl;
  auto cacheFactory = static_cast<CacheFactory*>(factory);
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
