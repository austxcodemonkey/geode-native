//
// Created by Blake Bender on 8/30/19.
//

#ifndef NATIVECLIENT_CACHEFACTORYWRAPPER_H
#define NATIVECLIENT_CACHEFACTORYWRAPPER_H
#pragma once

#include "geode/CacheFactory.hpp"

extern "C" {
void* APACHE_GEODE_EXPORT CreateCacheFactory();

void* APACHE_GEODE_EXPORT CacheFactory_CreateCache(void* factory);

const char* APACHE_GEODE_EXPORT CacheFactory_GetVersion(void* factory);

const char* APACHE_GEODE_EXPORT
CacheFactory_GetProductDescription(void* factory);

void APACHE_GEODE_EXPORT CacheFactory_SetPdxIgnoreUnreadFields(
    void* factory, bool pdxIgnoreUnreadFields);

void APACHE_GEODE_EXPORT CacheFactory_SetAuthInitialize(void* factory,
                                                        void* authInitialize);

void APACHE_GEODE_EXPORT
CacheFactory_SetPdxReadSerialized(void* factory, bool pdxReadSerialized);

void APACHE_GEODE_EXPORT CacheFactory_SetProperty(void* factory,
                                                  const char* key,
                                                  const char* value);

void APACHE_GEODE_EXPORT DestroyCacheFactory(void* factory);
};
#endif  // NATIVECLIENT_CACHEFACTORYWRAPPER_H
