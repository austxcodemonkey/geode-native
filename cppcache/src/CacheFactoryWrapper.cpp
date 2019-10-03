#include "geode/CacheFactoryWrapper.hpp"

#include "geode/AuthInitializeWrapper.hpp"
#include "geode/CacheWrapper.hpp"
#include "util/Log.hpp"

using apache::geode::client::CacheFactory;

void* CreateCacheFactory() {
  CacheFactoryWrapper* cacheFactory = new CacheFactoryWrapper();
  LOGDEBUG("%s: cachefactory=%p", __FUNCTION__, cacheFactory);
  return cacheFactory;
}

void* CacheFactory_CreateCache(void* factory) {
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  CacheWrapper* cache = cacheFactory->createCache();
  LOGDEBUG("%s: factory=%p, cache=%p", __FUNCTION__, factory, cache);
  return cache;
}

const char* CacheFactory_GetVersion(void* factory) {
  LOGDEBUG("%s: factory=%p", __FUNCTION__, factory);
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  return cacheFactory->getVersion();
}

const char* CacheFactory_GetProductDescription(void* factory) {
  LOGDEBUG("%s: factory=%p", __FUNCTION__, factory);
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  return cacheFactory->getProductDescription();
}

void CacheFactory_SetPdxIgnoreUnreadFields(void* factory,
                                           bool pdxIgnoreUnreadFields) {
  auto cacheFactory = static_cast<CacheFactory*>(factory);
  auto ignoreUnreadFields = pdxIgnoreUnreadFields ? "true" : "false";
  LOGDEBUG("%s: factory=%p, ignoreUnreadFields=%s", __FUNCTION__, factory,
           ignoreUnreadFields);
  cacheFactory->setPdxIgnoreUnreadFields(pdxIgnoreUnreadFields);
}

void CacheFactory_SetAuthInitialize(void* factory,
                                    void (*getCredentials)(void*),
                                    void (*close)()) {
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  LOGDEBUG("%s: factory=%p, getCredentials=%p, close=%p", __FUNCTION__, factory,
           getCredentials, close);
  cacheFactory->setAuthInitialize(getCredentials, close);
}

void CacheFactory_SetPdxReadSerialized(void* factory, bool pdxReadSerialized) {
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  auto readSerialized = pdxReadSerialized ? "true" : "false";
  LOGDEBUG("%s: factory=%p, readSerialized=%s", __FUNCTION__, factory,
           readSerialized);
  cacheFactory->setPdxReadSerialized(pdxReadSerialized);
}

void CacheFactory_SetProperty(void* factory, const char* key,
                              const char* value) {
  LOGDEBUG("%s: factory=%p, (k, v)=(%s, %s)", __FUNCTION__, factory,
           std::string(key).c_str(), std::string(value).c_str());
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
  cacheFactory->setProperty(key, value);
}

void DestroyCacheFactory(void* factory) {
  LOGDEBUG("%s: factory=%p", __FUNCTION__, factory);
  auto cacheFactory = static_cast<CacheFactoryWrapper*>(factory);
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
