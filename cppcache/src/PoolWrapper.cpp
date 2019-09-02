#include "geode/PoolWrapper.hpp"

#include <iostream>

void DestroyPool(void* pool) {
  std::cout << __FUNCTION__ << std::endl;
  PoolWrapper* poolWrapper = static_cast<PoolWrapper*>(pool);
  delete poolWrapper;
}

PoolWrapper::PoolWrapper(std::shared_ptr<apache::geode::client::Pool> pool)
    : pool_(pool) {}
