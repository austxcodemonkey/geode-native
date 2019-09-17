#include "geode/CacheFactoryWrapper.hpp"

#include <iostream>

#include "geode/AuthInitializeWrapper.hpp"
#include "geode/CacheWrapper.hpp"

using apache::geode::client::CacheFactory;

void* CreateCacheFactory() {
  CacheFactoryWrapper* cacheFactory = new CacheFactoryWrapper();
  std::cout << __FUNCTION__ << ": created factory " << cacheFactory
            << std::endl;
  return cacheFactory;
}

void* CacheFactory_CreateCache(void* factory) {
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  CacheWrapper* cache = cacheFactory->createCache();
  std::cout << __FUNCTION__ << ": created cache wrapper " << cache << std::endl;
  return cache;
}

const char* CacheFactory_GetVersion(void* factory) {
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  return cacheFactory->getVersion();
}

const char* CacheFactory_GetProductDescription(void* factory) {
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  return cacheFactory->getProductDescription();
}

void CacheFactory_SetPdxIgnoreUnreadFields(void* factory,
                                           bool pdxIgnoreUnreadFields) {
  auto cacheFactory = static_cast<CacheFactory*>(factory);
  auto ignoreUnreadFields = pdxIgnoreUnreadFields ? "true" : "false";
  std::cout << __FUNCTION__ << ": " << ignoreUnreadFields << std::endl;
  cacheFactory->setPdxIgnoreUnreadFields(pdxIgnoreUnreadFields);
}

void CacheFactory_SetAuthInitialize(void* factory,
                                    void (*getCredentials)(void*),
                                    void (*close)()) {
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  std::cout << __FUNCTION__ << ": (factory, getCredentials, close) = ("
            << cacheFactory << ", " << reinterpret_cast<void*>(getCredentials)
            << ", " << reinterpret_cast<void*>(close) << ")" << std::endl;
  cacheFactory->setAuthInitialize(getCredentials, close);
}

void CacheFactory_SetPdxReadSerialized(void* factory, bool pdxReadSerialized) {
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  auto readSerialized = pdxReadSerialized ? "true" : "false";
  std::cout << __FUNCTION__ << ": " << readSerialized << std::endl;
  cacheFactory->setPdxReadSerialized(pdxReadSerialized);
}

void CacheFactory_SetProperty(void* factory, const char* key,
                              const char* value) {
  std::cout << __FUNCTION__ << ": (k, v) = (" + std::string(key) << ", "
            << std::string(value) << ")" << std::endl;
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  cacheFactory->setProperty(key, value);
}

void DestroyCacheFactory(void* factory) {
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  std::cout << __FUNCTION__ << ": deleting factory " << cacheFactory
            << std::endl;
  delete cacheFactory;
}

const char* CacheFactoryWrapper::getVersion() {
  return cacheFactory_.getVersion().c_str();
}

const char* CacheFactoryWrapper::getProductDescription() {
  return cacheFactory_.getProductDescription().c_str();
}

void CacheFactoryWrapper::setPdxIgnoreUnreadFields(bool pdxIgnoreUnreadFields) {
  cacheFactory_.setPdxIgnoreUnreadFields(pdxIgnoreUnreadFields);
}

void CacheFactoryWrapper::setAuthInitialize(void (*getCredentials)(void*),
                                            void (*close)()) {
  authInit_ = std::make_shared<AuthInitializeWrapper>(getCredentials, close);
  cacheFactory_.setAuthInitialize(authInit_);
}

void CacheFactoryWrapper::setPdxReadSerialized(bool pdxReadSerialized) {
  cacheFactory_.setPdxReadSerialized(pdxReadSerialized);
}

void CacheFactoryWrapper::setProperty(const std::string& key,
                                      const std::string& value) {
  cacheFactory_.set(key, value);
}

CacheWrapper* CacheFactoryWrapper::createCache() {
  return new CacheWrapper(cacheFactory_.create());
}
