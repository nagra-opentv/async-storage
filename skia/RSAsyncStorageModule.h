/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once

#include <cxxreact/CxxModule.h>

#include "ReactSkia/utils/RnsUtils.h"

using namespace std;
namespace facebook {
namespace xplat {

class RSAsyncStorageModule : public module::CxxModule {
 public:
  RSAsyncStorageModule();
  virtual auto getConstants() -> std::map<std::string, folly::dynamic>;
  virtual auto getMethods() -> std::vector<Method>;
  std::string getName();
};
#ifdef __cplusplus
extern "C" {
#endif

xplat::module::CxxModule* RNAsyncStorageModuleCls(void) {
  return new RSAsyncStorageModule();
}

#ifdef __cplusplus
}
#endif
}
}
