//
// Created by Blake Bender on 8/30/19.
//

#ifndef NATIVECLIENT_CACHEWRAPPER_H
#define NATIVECLIENT_CACHEWRAPPER_H
#pragma once

#include "geode/Cache.hpp"
#include "geode/CacheFactory.hpp"

extern "C" {
void APACHE_GEODE_EXPORT DestroyCache(void* cache);

bool APACHE_GEODE_EXPORT Cache_GetPdxIgnoreUnreadFields(void* cache);

bool APACHE_GEODE_EXPORT Cache_GetPdxReadSerialized(void* cache);
};

class CacheWrapper {
 public:
  CacheWrapper(apache::geode::client::Cache cache);

  bool getPdxIgnoreUnreadFields();

  bool getPdxReadSerialized();

 private:
  apache::geode::client::Cache cache_;
  friend class apache::geode::client::Cache;
};

#endif  // NATIVECLIENT_CACHEWRAPPER_H
