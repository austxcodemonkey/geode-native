#include "geode/ClientWrapper.hpp"

#include "geode/Exception.hpp"
#include "geode/ExceptionTypes.hpp"
#include "util/Log.hpp"

static ClientWrapper* clientWrapperSingleton = nullptr;

void* ClientInitialize() {
  if (!clientWrapperSingleton) {
    clientWrapperSingleton = new ClientWrapper();
    LOGDEBUG("%s: clientWrapperSingleton=%p", __FUNCTION__,
             clientWrapperSingleton);
  }

  return clientWrapperSingleton;
}

int ClientUninitialize(void* client) {
  int result = 0;
  if (client != static_cast<void*>(clientWrapperSingleton)) {
    throw apache::geode::client::IllegalStateException(
        "Application attempted to uninitialize an invalid client");
  }

  result = clientWrapperSingleton->checkForLeaks();

  delete clientWrapperSingleton;
  clientWrapperSingleton = nullptr;

  return result;
}

void RegisterClientObject(void* value, const std::string& className) {
  clientWrapperSingleton->AddRecord(value, className);
}

void UnregisterClientObject(void* value) {
  clientWrapperSingleton->RemoveRecord(value);
}

ClientWrapper::ClientWrapper() {}

ClientWrapper::~ClientWrapper() {}

int ClientWrapper::checkForLeaks() {
  int result = 0;
  if (!registry_.empty()) {
    for (auto recordPair : registry_) {
      auto object = recordPair.first;
      auto record = recordPair.second;

      LOGERROR("Leaked object of type \"%s\" (pointer value %p), callstack %s",
               record.className.c_str(), object,
               record.allocationCallstack.c_str());
      result = -1;
    }
  }
  return result;
}

void ClientWrapper::AddRecord(void* value, const std::string& className) {
  LOGERROR("ClientWrapper::%s(%p): Adding value %p, className=%s", __FUNCTION__,
           this, value, className.c_str());
  apache::geode::client::Exception ex("");
  registry_.insert(std::pair<void*, ClientObjectRecord>(
      value, ClientObjectRecord{className, ex.getStackTrace()}));
}

void ClientWrapper::RemoveRecord(void* value) { registry_.erase(value); }
