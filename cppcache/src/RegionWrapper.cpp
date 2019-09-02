#include <iostream>

#include "geode/RegionFactoryWrapper.hpp"
#include "geode/RegionShortcut.hpp"

using apache::geode::client::RegionFactory;

RegionWrapper::RegionWrapper(
    std::shared_ptr<apache::geode::client::Region> region)
    : region_(region) {}

void DestroyRegion(void* region) {
  RegionWrapper* regionWrapper = static_cast<RegionWrapper*>(region);
  std::cout << __FUNCTION__ << ": Destroying region wrapper " << regionWrapper
            << std::endl;
  delete regionWrapper;
}
