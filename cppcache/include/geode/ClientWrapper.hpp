//
// Created by Blake Bender on 8/30/19.
//

#ifndef NATIVECLIENT_CLIENTWRAPPER_H
#define NATIVECLIENT_CLIENTWRAPPER_H
#pragma once

#include <map>

#include <geode/internal/geode_base.hpp>

extern "C" {

APACHE_GEODE_EXPORT void *ClientInitialize();

APACHE_GEODE_EXPORT int ClientUninitialize(void *client);
}

void RegisterClientObject(void* value, const std::string& className);
void UnregisterClientObject(void* value);

struct ClientObjectRecord {
  std::string className;
  std::string allocationCallstack;
};

typedef std::map<void*, ClientObjectRecord> ClientObjectRegistry;

class ClientWrapper {
 public:
  ClientWrapper();
  virtual ~ClientWrapper();
  int checkForLeaks();

  void AddRecord(void* value, const std::string& className);
  void RemoveRecord(void* value);

 private:
  ClientObjectRegistry registry_;

};
#endif  // NATIVECLIENT_CLIENTWRAPPER_H
