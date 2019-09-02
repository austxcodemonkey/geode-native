#include "geode/PoolFactoryWrapper.hpp"

#include <iostream>

void* PoolFactory_CreatePool(void* poolFactory, const char* name) {
  std::cout << __FUNCTION__ << std::endl;
  PoolFactoryWrapper* poolFactoryWrapper =
      static_cast<PoolFactoryWrapper*>(poolFactory);
  return poolFactoryWrapper->CreatePool(name);
}

void PoolFactory_AddLocator(void* poolFactory, const char* hostname,
                            const uint16_t port) {
  std::cout << __FUNCTION__ << std::endl;
  PoolFactoryWrapper* poolFactoryWrapper =
      static_cast<PoolFactoryWrapper*>(poolFactory);
  poolFactoryWrapper->AddLocator(hostname, port);
}

void DestroyPoolFactory(void* poolFactory) {
  std::cout << __FUNCTION__ << std::endl;
  PoolFactoryWrapper* poolFactoryWrapper =
      static_cast<PoolFactoryWrapper*>(poolFactory);
  delete poolFactoryWrapper;
}

PoolFactoryWrapper::PoolFactoryWrapper(
    apache::geode::client::PoolFactory poolFactory)
    : poolFactory_(poolFactory) {
  std::cout << "PoolFactoryWrapper::" << __FUNCTION__ << std::endl;
}

PoolWrapper* PoolFactoryWrapper::CreatePool(const char* name) {
  return new PoolWrapper(poolFactory_.create(name));
}

void PoolFactoryWrapper::AddLocator(const std::string& hostname,
                                    const uint16_t port) {
  poolFactory_.addLocator(hostname, port);
}
