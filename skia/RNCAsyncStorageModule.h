/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once
#include <fstream>
#include <thread>
#include "rns_shell/platform/linux/TaskLoop.h"
#include <cxxreact/CxxModule.h>
#include "ReactSkia/utils/RnsUtils.h"

#ifndef FILE_PATH
#define FILE_PATH ("SimpleViewApp.json")
#endif

#define EXTRA_CHARACTER_PADDING_LENGTH (6) // padding for commas and colons
#define ASYNC_STORGAE_FILE_WRITE_TIMEOUT (5000) // 5 seconds
#define ASYNC_STORAGE_FILE_MAX_LENGTH (6*1024*1024) //6,971,520 bytes
#define ASYNC_STORAGE_VALUE_MAX_LENGTH (2*1024*1024) //2,971,520 bytes

using namespace std;
using namespace folly;
namespace facebook {
namespace xplat {

class RNCAsyncStorageModule : public module::CxxModule {
 public:
  RNCAsyncStorageModule();
  ~RNCAsyncStorageModule();
  std::map<std::string, folly::dynamic> getConstants() override;
  std::vector<Method> getMethods() override;
  std::string getName() override;
  
 private:
  void multiGet(dynamic args, CxxModule::Callback cb);
  void multiSet(dynamic args, CxxModule::Callback cb);
  void multiRemove(dynamic args, CxxModule::Callback cb);
  void mergeItem(dynamic args, CxxModule::Callback cb);
  void getAllKeys(dynamic args, CxxModule::Callback cb);
  void clear(dynamic args, CxxModule::Callback cb);
  void asyncWorkerThread();
  void writeToFile();
  dynamic appLocalDataFile_ = dynamic::object;
  fstream appLocalFile_;
  bool isWriteScheduled_;
  std::unique_ptr<RnsShell::TaskLoop> taskRunner_{nullptr};
  std::thread workerThread_;
  size_t totalSize_ =0;

};

}//xplat
}//facebook