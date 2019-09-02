//
// Created by Blake Bender on 8/30/19.
//

#ifndef NATIVECLIENT_REGIONFACTORYWRAPPER_H
#define NATIVECLIENT_REGIONFACTORYWRAPPER_H
#pragma once

#include "geode/Cache.hpp"
#include "geode/PoolManagerWrapper.hpp"
#include "geode/RegionFactory.hpp"

extern "C" {
void APACHE_GEODE_EXPORT DestroyRegionFactory(void* cache);
};

class RegionFactoryWrapper {
 public:
  RegionFactoryWrapper(apache::geode::client::RegionFactory regionFactory);

 private:
  apache::geode::client::RegionFactory regionFactory_;
};

#endif  // NATIVECLIENT_REGIONFACTORYWRAPPER_H
