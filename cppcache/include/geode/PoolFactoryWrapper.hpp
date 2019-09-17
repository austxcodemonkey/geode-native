//
// Created by Blake Bender on 8/30/19.
//

#ifndef NATIVECLIENT_POOLFACTORYWRAPPER_H
#define NATIVECLIENT_POOLFACTORYWRAPPER_H
#pragma once

#include "geode/Cache.hpp"
#include "geode/CacheFactory.hpp"
#include "geode/PoolManager.hpp"
#include "geode/PoolWrapper.hpp"

extern "C" {
APACHE_GEODE_EXPORT void* PoolFactory_CreatePool(void* poolFactory,
                                                 const char* name);

APACHE_GEODE_EXPORT void PoolFactory_AddLocator(void* poolFactory,
                                                const char* hostname,
                                                const uint16_t port);

APACHE_GEODE_EXPORT void DestroyPoolFactory(void* poolFactory);
}

class PoolFactoryWrapper {
 public:
  PoolFactoryWrapper(apache::geode::client::PoolFactory poolFactory);

  PoolWrapper* CreatePool(const char* name);

  void AddLocator(const std::string& hostname, const uint16_t port);

 private:
  apache::geode::client::PoolFactory poolFactory_;
};

#endif  // NATIVECLIENT_POOLFACTORYWRAPPER_H
