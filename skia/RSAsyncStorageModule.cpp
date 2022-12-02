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
#define ASYNC_DEFAULT_MAX_CACHE_LIMIT 6*1024*1024 //6,971,520 bytes
#define ASYNC_VALUE_DEFAULT_MAX_CACHE_LIMIT 2*1024*1024 //2,971,520 bytes

using namespace folly;
namespace facebook {
namespace xplat {

RSAsyncStorageModule::RSAsyncStorageModule() {
    taskRunner_ = std::make_unique<RnsShell::TaskLoop>();
    workerThread_=std::thread (&RSAsyncStorageModule::asyncWorkerThread,this);
    taskRunner_->waitUntilRunning();

    taskRunner_->dispatch( [this]() {
      outfile_.open("SimpleViewApp.json", ios::out | ios::in | ios::app);
      if (outfile_) {
        // get length of file:
        outfile_.seekg (0, outfile_.end);
        int length = outfile_.tellg();
        outfile_.seekg (0, outfile_.beg);
        char * buffer = new char [length];
        // read data as a block:
        outfile_.read (buffer,length);
        RNS_LOG_ERROR(buffer);
        string Str(buffer);
        try {
          globalDynamic_ = parseJson(Str);
          totalSize = length;
        }
        catch(exception e) {
          RNS_LOG_ERROR("json parsing failed");
        }
        delete []buffer;
      }else {
        RNS_LOG_ERROR("Cannot open file");
        return;
      }

    });
}

RSAsyncStorageModule::~RSAsyncStorageModule() {
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
            mergeItem(args, cb);
          }),
      Method(
          "getAllKeys",
          [this] (dynamic args, CxxModule::Callback cb) {
            getAllKeys(args, cb);
          }),
      Method(
          "clear",
          [this] (dynamic args, CxxModule::Callback cb) {
            clear(args, cb);
          }),

  };
}


void RSAsyncStorageModule::multiGet(dynamic args, CxxModule::Callback cb) {
  RNS_LOG_NOT_IMPL;
  cb({});
}

void RSAsyncStorageModule::multiSet(dynamic args, CxxModule::Callback cb) {

  taskRunner_->dispatch( [this,args,cb]() {
    int requiredLength = 0;
    for (auto& itemArray : args[0]) {
      int oldLength = 0;
      int newLength = (itemArray[0].asString()).length()+itemArray[1].asString().length()+6;
      // value size cannot be larger than 2 MB
      if((itemArray[1].asString()).length() < ASYNC_VALUE_DEFAULT_MAX_CACHE_LIMIT ) {
        if((globalDynamic_.find(itemArray[0].asString())) != globalDynamic_.items().end()) {
          RNS_LOG_ERROR("FOUND multiGet AGAIN");
          oldLength = (itemArray[0].asString()).length()+(globalDynamic_[itemArray[0]].asString()).length()+6;
          if(oldLength < newLength) {
            requiredLength = newLength - oldLength;
          }else{
            requiredLength = oldLength-newLength;
          }
        }else{
          requiredLength= newLength;
        }
        globalDynamic_[itemArray[0].asString()] = itemArray[1].asString();
      }
    }
    totalSize += requiredLength;
    RNS_LOG_ERROR("totalSize" << totalSize);
    if(isWriteScheduled_)
      return;
    RNS_LOG_DEBUG("Scheduling to write the file");
    taskRunner_->scheduleDispatch( [&](){
      RNS_LOG_DEBUG ("writing to json file");
      string str = toJson(globalDynamic_);
      if(!outfile_.is_open()) {
        dynamic errors = folly::dynamic::object();
        errors["message"] = "not able to set Item operation";
        errors["key"] = args[0][0].asString();
        cb({folly::dynamic::array(errors)});
        return;
      }
      RNS_LOG_ERROR("SIZE OF file" << str.length());
      // Total storage size is capped at 6 MB by default.
      if(totalSize < ASYNC_DEFAULT_MAX_CACHE_LIMIT) {
        filesystem::resize_file("SimpleViewApp.json", 0);
        outfile_.seekp(0);
        outfile_ << str;
        outfile_.flush();
        isWriteScheduled_ = false;
      } else{
        RNS_LOG_WARN("async storage size is greater than 6mb");
        return;
      }
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
