/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <cxxreact/JsArgumentHelpers.h>

#include "ReactSkia/utils/RnsLog.h"
#include "RSAsyncStorageModule.h"
using namespace folly;
namespace facebook {
namespace xplat {

RSAsyncStorageModule::RSAsyncStorageModule(){
}

auto RSAsyncStorageModule::getConstants() -> std::map<std::string, folly::dynamic> {
  return {};
}

std::string RSAsyncStorageModule::getName() {
  return "PlatformLocalStorage";
}

auto RSAsyncStorageModule::getMethods() -> std::vector<Method> {
  return {
      Method(
          "multiGet",
          [] (dynamic args, CxxModule::Callback cb){
            RNS_LOG_NOT_IMPL;
            cb({});
            return;
          }),
      Method(
          "multiSet",
          [] (dynamic args, CxxModule::Callback cb) {
            RNS_LOG_NOT_IMPL;
            cb({});
            return;
          }),
      Method(
          "multiRemove",
          [] (dynamic args, CxxModule::Callback cb) {
            RNS_LOG_NOT_IMPL;
            cb({});
            return;
          }),
      Method(
          "mergeItem",
          [] (dynamic args, CxxModule::Callback cb) {
            RNS_LOG_NOT_IMPL;
            cb({});
            return;
          }),
      Method(
          "getAllKeys",
          [] (dynamic args, CxxModule::Callback cb) {
            RNS_LOG_NOT_IMPL;
            cb({});
            return;
          }),
      Method(
          "clear",
          [] (dynamic args, CxxModule::Callback cb) {
            RNS_LOG_NOT_IMPL;
            cb({});
            return;
          }),

  };
}

}
}
