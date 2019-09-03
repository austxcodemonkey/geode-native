#include <iostream>

#include "geode/RegionFactoryWrapper.hpp"
#include "geode/RegionShortcut.hpp"

using apache::geode::client::RegionFactory;

RegionWrapper::RegionWrapper(
    std::shared_ptr<apache::geode::client::Region> region)
    : region_(region) {}

void RegionWrapper::PutString(const std::string& key,
                              const std::string& value) {
  region_->put(key, value);
}

const char* RegionWrapper::GetString(const std::string& key) {
  auto value = region_->get(key);
  lastValue_ =
      std::dynamic_pointer_cast<apache::geode::client::CacheableString>(value)
          ->value();
  return lastValue_.c_str();
}

void RegionWrapper::Remove(const std::string& key) { region_->remove(key); }

bool RegionWrapper::ContainsValueForKey(const std::string& key) {
  return region_->containsValueForKey(key);
}

void DestroyRegion(void* region) {
  RegionWrapper* regionWrapper = static_cast<RegionWrapper*>(region);
  std::cout << __FUNCTION__ << ": Destroying region wrapper " << regionWrapper
            << std::endl;
  delete regionWrapper;
}

void Region_PutString(void* region, const char* key, const char* value) {
  RegionWrapper* regionWrapper = static_cast<RegionWrapper*>(region);
  std::cout << __FUNCTION__ << std::endl;
  regionWrapper->PutString(key, value);
}

const char* Region_GetString(void* region, const char* key) {
  RegionWrapper* regionWrapper = static_cast<RegionWrapper*>(region);
  std::cout << __FUNCTION__ << std::endl;
  return regionWrapper->GetString(key);
}

void Region_Remove(void* region, const char* key) {
  RegionWrapper* regionWrapper = static_cast<RegionWrapper*>(region);
  std::cout << __FUNCTION__ << std::endl;
  return regionWrapper->Remove(key);
}

bool Region_ContainsValueForKey(void* region, const char* key) {
  RegionWrapper* regionWrapper = static_cast<RegionWrapper*>(region);
  std::cout << __FUNCTION__ << std::endl;
  return regionWrapper->ContainsValueForKey(key);
}
