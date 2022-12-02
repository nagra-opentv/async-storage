/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once

#include <cxxreact/CxxModule.h>

#include "ReactSkia/utils/RnsUtils.h"
#include <fstream>
#include <thread>
#include "rns_shell/platform/linux/TaskLoop.h"
using namespace std;
using namespace folly;
namespace facebook {
namespace xplat {

class RSAsyncStorageModule : public module::CxxModule {
private:
  void multiGet(dynamic args, CxxModule::Callback cb);
  void multiSet(dynamic args, CxxModule::Callback cb);
  void multiRemove(dynamic args, CxxModule::Callback cb);
  void mergeItem(dynamic args, CxxModule::Callback cb);
  void getAllKeys(dynamic args, CxxModule::Callback cb);
  void clear(dynamic args, CxxModule::Callback cb);
  void storeinFile(string key, string value);
  void asyncWorkerThread();
  dynamic appLocalDataFile = dynamic::object;
  fstream outfile_;
  //fstream filestr;
  bool isWriteScheduled_;
  /*To fulfill OpenGl requirement of create & rendering to be handled from same thread context*/
    std::unique_ptr<RnsShell::TaskLoop> taskRunner_{nullptr};
    std::thread workerThread_;
 public:
  RSAsyncStorageModule();
  ~RSAsyncStorageModule();
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