/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <cxxreact/JsArgumentHelpers.h>

#include "ReactSkia/utils/RnsLog.h"

namespace facebook {
namespace xplat {

RSAsyncStorage::RSAsyncStorage(){}

auto RSAsyncStorage::getConstants() -> std::map<std::string, folly::dynamic> {
  return {};
}

std::string RSAsyncStorage::getName() {
  return "RNCAsyncStorage";
}

auto RSkImageLoaderModule::getMethods() -> std::vector<Method> {
  return {
      Method(
          "getItem",
          [this] (dynamic args, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock){
            getItem(jsArgAsString(args,0),resolveBlock,rejectBlock);
          }),
      Method(
          "setItem",
          [this] (dynamic args, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock) {
            seyItem(jsArgAsString(args, 0), jsArgAsString(args, 1), resolveBlock, rejectBlock);
          }),
      Method(
          "removeItem",
          [this] (dynamic args, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock) {
            removeItem(jsArgAsString(args, 0), resolveBlock, rejectBlock);
          }),
  };
}

void RSAsyncStorage::getItem(std::string key, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock) {
  RNS_LOG_NOT_IMPL;
}

void RSAsyncStorage::setItem(std::string key, std::string value, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock) {
  RNS_LOG_NOT_IMPL;
}

void RSAsyncStorage::removeItem(std::string key, CxxModule::Callback resolveBlock, CxxModule::Callback rejectBlock) {
  RNS_LOG_NOT_IMPL;
}



}
}
