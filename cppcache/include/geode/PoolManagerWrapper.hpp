//
// Created by Blake Bender on 8/30/19.
//

#ifndef NATIVECLIENT_POOLMANAGERWRAPPER_H
#define NATIVECLIENT_POOLMANAGERWRAPPER_H
#pragma once

#include "geode/Cache.hpp"
#include "geode/CacheFactory.hpp"
#include "geode/PoolFactoryWrapper.hpp"
#include "geode/PoolManager.hpp"

extern "C" {
void* APACHE_GEODE_EXPORT PoolManager_CreateFactory(void* poolManager);

void APACHE_GEODE_EXPORT DestroyPoolManager(void* poolManager);
};

class PoolManagerWrapper {
 public:
  PoolManagerWrapper(apache::geode::client::PoolManager& poolManager);

  PoolFactoryWrapper* CreatePoolFactory();

 private:
  apache::geode::client::PoolManager& poolManager_;
};

#endif  // NATIVECLIENT_POOLMANAGERWRAPPER_H
