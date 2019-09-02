//
// Created by Blake Bender on 8/30/19.
//

#ifndef NATIVECLIENT_POOLWRAPPER_H
#define NATIVECLIENT_POOLWRAPPER_H
#pragma once

#include "geode/Pool.hpp"

extern "C" {
void* APACHE_GEODE_EXPORT Pool_CreateFactory(void* pool);

void APACHE_GEODE_EXPORT DestroyPool(void* pool);
};

class PoolWrapper {
 public:
  PoolWrapper(std::shared_ptr<apache::geode::client::Pool> pool);

 private:
  std::shared_ptr<apache::geode::client::Pool> pool_;
};

#endif  // NATIVECLIENT_POOLWRAPPER_H
