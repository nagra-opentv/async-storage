/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once

#include <cxxreact/CxxModule.h>

using namespace std;
namespace facebook {
namespace xplat {

class RSAsyncStorage : public module:::CxxModule {
 public:
  RSAsyncStorage();
  virtual auto getConstants() -> std::map<std::string, folly::dynamic>;
  virtual auto getMethods() -> std::vector<Method>;
  virtual std::string getName();

 private:
  void getItem(std::string key, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock);
  void setItem(std::string key, std::string value, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock);
  void removeItem(std::string key, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock);
};
#ifdef __cplusplus
extern "C" {
#endif

xplat::module::CxxModule* RNAsyncStorageModuleCls(void) {
  return new RSAsyncStorage();
}

#ifdef __cplusplus
}
#endif
}
}
