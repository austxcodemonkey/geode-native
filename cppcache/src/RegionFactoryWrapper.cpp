#include "geode/RegionFactoryWrapper.hpp"

#include <iostream>

#include "geode/RegionShortcut.hpp"

using apache::geode::client::RegionFactory;

RegionFactoryWrapper::RegionFactoryWrapper(
    apache::geode::client::RegionFactory regionFactory)
    : regionFactory_(std::move(regionFactory)) {}

void DestroyRegionFactory(void* regionFactory) {
  RegionFactoryWrapper* regionFactoryWrapper =
      static_cast<RegionFactoryWrapper*>(regionFactory);
  std::cout << __FUNCTION__ << ": Destroying region factory wrapper "
            << regionFactoryWrapper << std::endl;
  delete regionFactoryWrapper;
}
