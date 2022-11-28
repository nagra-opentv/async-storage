/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <cxxreact/JsArgumentHelpers.h>

#include "ReactSkia/utils/RnsLog.h"
#include "RSAsyncStorageModule.h"
#include <folly/json.h>
#include <fstream>
#define ASYNC_SRG_TIMEOUT 5000
using namespace folly;
namespace facebook {
namespace xplat {

RSAsyncStorageModule::RSAsyncStorageModule(){
    taskRunner_ = std::make_unique<RnsShell::TaskLoop>();
    workerThread_=std::thread (&RSAsyncStorageModule::asyncWorkerThread,this);
    taskRunner_->waitUntilRunning();

    taskRunner_->dispatch( [this](){
      outfile_.open("SimpleViewApp.json", ios::out | ios::trunc );
    });
}

RSAsyncStorageModule::~RSAsyncStorageModule(){
  outfile_.close();
  taskRunner_->stop();
    if (workerThread_.joinable() ) {
    workerThread_.join();
  }
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
          [this] (dynamic args, CxxModule::Callback cb){
            multiGet(args, cb);
          }),
      Method(
          "multiSet",
          [this] (dynamic args, CxxModule::Callback cb) {
            multiSet(args, cb);
          }),
      Method(
          "multiRemove",
          [this] (dynamic args, CxxModule::Callback cb) {
            multiRemove(args, cb);
          }),
      Method(
          "mergeItem",
          [this] (dynamic args, CxxModule::Callback cb) {
            RNS_LOG_NOT_IMPL;
            mergeItem(args, cb);
          }),
      Method(
          "getAllKeys",
          [this] (dynamic args, CxxModule::Callback cb) {
            RNS_LOG_NOT_IMPL;
            getAllKeys(args, cb);
          }),
      Method(
          "clear",
          [this] (dynamic args, CxxModule::Callback cb) {
            RNS_LOG_NOT_IMPL;
            clear(args, cb);
          }),

  };
}


void RSAsyncStorageModule::multiGet(dynamic args, CxxModule::Callback cb) {
  RNS_LOG_NOT_IMPL;
  cb({});
}

void RSAsyncStorageModule::multiSet(dynamic args, CxxModule::Callback cb) {

  taskRunner_->dispatch( [this,args,cb](){
    for (auto& itemArray : args[0]) {
      globalDynamic_[itemArray[0].asString()] = itemArray[1].asString();
    }
    if(isWriteScheduled_)
      return;
    RNS_LOG_DEBUG("Scheduled filewrite" );
    taskRunner_->scheduleDispatch( [&](){
      RNS_LOG_DEBUG ("writing to file SimpleViewApp.json");
      folly::json::serialization_opts opts;
      string str = folly::json::serialize(globalDynamic_, opts);
      if(!outfile_.is_open()) {
        dynamic errors = folly::dynamic::object();
        errors["message"] = "not able to do set Item operation";
        errors["key"] = args[0][0].asString();
        cb({folly::dynamic::array(errors)});
        return;
      }
      outfile_ << str;
      outfile_.close();
      isWriteScheduled_ = false;
    }, ASYNC_SRG_TIMEOUT);
    isWriteScheduled_ = true;;
  cb({});
});

}

void RSAsyncStorageModule::multiRemove(dynamic args, CxxModule::Callback cb) {
  RNS_LOG_NOT_IMPL;
  cb({});
}

void RSAsyncStorageModule::mergeItem(dynamic args, CxxModule::Callback cb) {
  RNS_LOG_NOT_IMPL;
  cb({});
}

void RSAsyncStorageModule::getAllKeys(dynamic args, CxxModule::Callback cb) {
  RNS_LOG_NOT_IMPL;
  cb({});
}

void RSAsyncStorageModule::clear(dynamic args, CxxModule::Callback cb) {
  RNS_LOG_NOT_IMPL;
  cb({});
}

void RSAsyncStorageModule::asyncWorkerThread() {
  taskRunner_->run();
}
}
}
