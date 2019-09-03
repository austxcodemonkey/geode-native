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

void APACHE_GEODE_EXPORT Region_PutString(void* region, const char* key,
                                          const char* value);

const char* APACHE_GEODE_EXPORT Region_GetString(void* region, const char* key);
};

class RegionWrapper {
 public:
  RegionWrapper(std::shared_ptr<apache::geode::client::Region> region);

  void PutString(const std::string& key, const std::string& value);

  const char* GetString(const std::string& key);

 private:
  std::shared_ptr<apache::geode::client::Region> region_;
  std::string lastValue_;
};

#endif  // NATIVECLIENT_REGIONWRAPPER_H
