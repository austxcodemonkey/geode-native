#include "geode/PoolManagerWrapper.hpp"

#include <iostream>

#include "geode/PoolFactoryWrapper.hpp"

void* PoolManager_CreateFactory(void* poolManager) {
  std::cout << __FUNCTION__ << std::endl;
  PoolManagerWrapper* poolManagerWrapper =
      static_cast<PoolManagerWrapper*>(poolManager);
  return poolManagerWrapper->CreatePoolFactory();
}

void DestroyPoolManager(void* poolManager) {
  std::cout << __FUNCTION__ << std::endl;
  PoolManagerWrapper* poolManagerWrapper =
      static_cast<PoolManagerWrapper*>(poolManager);
  delete poolManagerWrapper;
}

PoolManagerWrapper::PoolManagerWrapper(
    apache::geode::client::PoolManager& poolManager)
    : poolManager_(poolManager) {}

PoolFactoryWrapper* PoolManagerWrapper::CreatePoolFactory() {
  std::cout << "PoolManagerWrapper::" << __FUNCTION__ << std::endl;
  return new PoolFactoryWrapper(poolManager_.createFactory());
}
