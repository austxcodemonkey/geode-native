//
// Created by Blake Bender on 8/30/19.
//

#ifndef NATIVECLIENT_REGIONFACTORYWRAPPER_H
#define NATIVECLIENT_REGIONFACTORYWRAPPER_H
#pragma once

#include "geode/Cache.hpp"
#include "geode/PoolManagerWrapper.hpp"
#include "geode/RegionFactory.hpp"
#include "geode/RegionWrapper.hpp"

extern "C" {
APACHE_GEODE_EXPORT void DestroyRegionFactory(void* cache);

APACHE_GEODE_EXPORT void RegionFactory_SetPoolName(void* regionFactory,
                                                   const char* poolName);

APACHE_GEODE_EXPORT void* RegionFactory_CreateRegion(void* regionFactory,
                                                     const char* regionName);
}

class RegionFactoryWrapper {
 public:
  RegionFactoryWrapper(apache::geode::client::RegionFactory regionFactory);

  void setPoolName(const std::string& poolName);

  RegionWrapper* createRegion(const std::string& regionName);

 private:
  apache::geode::client::RegionFactory regionFactory_;
};

#endif  // NATIVECLIENT_REGIONFACTORYWRAPPER_H
