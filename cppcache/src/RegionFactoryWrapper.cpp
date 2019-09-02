#include "geode/RegionFactoryWrapper.hpp"

#include <iostream>

#include "geode/RegionWrapper.hpp"

using apache::geode::client::RegionFactory;

RegionFactoryWrapper::RegionFactoryWrapper(
    apache::geode::client::RegionFactory regionFactory)
    : regionFactory_(std::move(regionFactory)) {}

void RegionFactoryWrapper::setPoolName(const std::string& poolName) {
  regionFactory_.setPoolName(poolName);
}

RegionWrapper* RegionFactoryWrapper::createRegion(
    const std::string& regionName) {
  return new RegionWrapper(regionFactory_.create(regionName));
}

void DestroyRegionFactory(void* regionFactory) {
  RegionFactoryWrapper* regionFactoryWrapper =
      static_cast<RegionFactoryWrapper*>(regionFactory);
  std::cout << __FUNCTION__ << ": Destroying region factory wrapper "
            << regionFactoryWrapper << std::endl;
  delete regionFactoryWrapper;
}

void RegionFactory_SetPoolName(void* regionFactory, const char* poolName) {
  RegionFactoryWrapper* regionFactoryWrapper =
      static_cast<RegionFactoryWrapper*>(regionFactory);
  regionFactoryWrapper->setPoolName(poolName);
}

void* RegionFactory_CreateRegion(void* regionFactory, const char* regionName) {
  RegionFactoryWrapper* regionFactoryWrapper =
      static_cast<RegionFactoryWrapper*>(regionFactory);
  return regionFactoryWrapper->createRegion(regionName);
}
