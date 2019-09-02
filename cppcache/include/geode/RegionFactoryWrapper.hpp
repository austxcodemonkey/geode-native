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
void APACHE_GEODE_EXPORT DestroyRegionFactory(void* cache);

void APACHE_GEODE_EXPORT RegionFactory_SetPoolName(void* regionFactory,
                                                   const char* poolName);

void* APACHE_GEODE_EXPORT RegionFactory_CreateRegion(void* regionFactory,
                                                     const char* regionName);
};

class RegionFactoryWrapper {
 public:
  RegionFactoryWrapper(apache::geode::client::RegionFactory regionFactory);

  void setPoolName(const std::string& poolName);

  RegionWrapper* createRegion(const std::string& regionName);

 private:
  apache::geode::client::RegionFactory regionFactory_;
};

#endif  // NATIVECLIENT_REGIONFACTORYWRAPPER_H
