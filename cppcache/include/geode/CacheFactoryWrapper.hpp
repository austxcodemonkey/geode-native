//
// Created by Blake Bender on 8/30/19.
//

#ifndef NATIVECLIENT_CACHEFACTORYWRAPPER_H
#define NATIVECLIENT_CACHEFACTORYWRAPPER_H
#pragma once

#include "geode/AuthInitializeWrapper.hpp"
#include "geode/CacheFactory.hpp"
#include "geode/CacheWrapper.hpp"

extern "C" {

APACHE_GEODE_EXPORT void* CacheFactory_CreateCache(void* factory);

APACHE_GEODE_EXPORT const char*  CacheFactory_GetVersion(void* factory);

APACHE_GEODE_EXPORT const char*
CacheFactory_GetProductDescription(void* factory);

APACHE_GEODE_EXPORT void  CacheFactory_SetPdxIgnoreUnreadFields(
    void* factory, bool pdxIgnoreUnreadFields);

APACHE_GEODE_EXPORT void  CacheFactory_SetAuthInitialize(
    void* factory, void (*getCredentials)(void*), void (*close)());

APACHE_GEODE_EXPORT void
CacheFactory_SetPdxReadSerialized(void* factory, bool pdxReadSerialized);

APACHE_GEODE_EXPORT void  CacheFactory_SetProperty(void* factory,
                                                  const char* key,
                                                  const char* value);

APACHE_GEODE_EXPORT void  DestroyCacheFactory(void* factory);
}

class CacheFactoryWrapper {
 public:
  CacheFactoryWrapper() = default;

  const char* getVersion();

  const char* getProductDescription();

  void setPdxIgnoreUnreadFields(bool pdxIgnoreUnreadFields);

  void setAuthInitialize(void (*getCredentials)(void*), void (*close)());
  void setPdxReadSerialized(bool pdxReadSerializedjj);

  void setProperty(const std::string& key, const std::string& value);

  CacheWrapper* createCache();

 private:
  apache::geode::client::CacheFactory cacheFactory_;
  std::shared_ptr<AuthInitializeWrapper> authInit_;
};
#endif  // NATIVECLIENT_CACHEFACTORYWRAPPER_H
