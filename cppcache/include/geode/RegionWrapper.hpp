//
// Created by Blake Bender on 8/30/19.
//

#ifndef NATIVECLIENT_REGIONWRAPPER_H
#define NATIVECLIENT_REGIONWRAPPER_H
#pragma once

#include "geode/Cache.hpp"
#include "geode/PoolManagerWrapper.hpp"
#include "geode/RegionFactory.hpp"

extern "C" {
void APACHE_GEODE_EXPORT DestroyRegion(void* region);
};

class RegionWrapper {
 public:
  RegionWrapper(std::shared_ptr<apache::geode::client::Region> region);

 private:
  std::shared_ptr<apache::geode::client::Region> region_;
};

#endif  // NATIVECLIENT_REGIONWRAPPER_H
