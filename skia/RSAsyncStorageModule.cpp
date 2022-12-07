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

using namespace folly;
namespace facebook {
namespace xplat {

RSAsyncStorageModule::RSAsyncStorageModule() {
  taskRunner_ = std::make_unique<RnsShell::TaskLoop>();
  workerThread_=std::thread (&RSAsyncStorageModule::asyncWorkerThread,this);
  taskRunner_->waitUntilRunning();

  taskRunner_->dispatch( [this]() {
    appLocalFile_.open(FILE_PATH, ios::out | ios::in | ios::app);
    if (appLocalFile_) {
      // get length of file:
      appLocalFile_.seekg (0, appLocalFile_.end);
      int length = appLocalFile_.tellg();
      if(length !=0) {
        appLocalFile_.seekg (0, appLocalFile_.beg);
        char * buffer = new char [length];
        // read data as a block:
        appLocalFile_.read (buffer,length);
        string Str(buffer);
        try {
          appLocalDataFile_ = parseJson(Str);
          totalSize_ = length;
        }
        catch(exception e) {
          RNS_LOG_ERROR("json parsing failed");
        }
        delete []buffer;
      }
    }else {
      RNS_LOG_ERROR("Cannot open file");
      return;
    }

  });
}

RSAsyncStorageModule::~RSAsyncStorageModule() {
  if(appLocalFile_) {
    appLocalFile_.close();
  }
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
  taskRunner_->dispatch( [this,args,cb](){
    dynamic errors = folly::dynamic::array;
    dynamic resultArray = folly::dynamic::array;
    for (auto& itemArray : args[0]) { 
      auto pos = appLocalDataFile_.find(itemArray);
      dynamic errorItem = folly::dynamic::object();
      if(pos == appLocalDataFile_.items().end()){
        errorItem["message"] = "Failed to found the get Item";
        errorItem["key"] = itemArray;
        errors.push_back(errorItem);
      }else{
        dynamic result = folly::dynamic::array;
        result.push_back(itemArray);
        result.push_back(appLocalDataFile_[itemArray]);
        resultArray.push_back(result);
      }
    }
    cb({(errors.empty() ? nullptr : errors), resultArray});
  });  
}

void RSAsyncStorageModule::multiSet(dynamic args, CxxModule::Callback cb) {
  if(!appLocalFile_.is_open()) {
    dynamic errors = folly::dynamic::object();
    errors["message"] = "Failed to write the data";
    errors["key"] = args[0][0].asString();
    cb({folly::dynamic::array(errors)});
    return;
  }
  taskRunner_->dispatch( [this,args,cb]() {
    int requiredLength = 0;
    for (auto& itemArray : args[0]) {
      int oldLength = 0;
      int newLength = (itemArray[0].asString()).length()+itemArray[1].asString().length()+6;
      // value size cannot be larger than 2 MB
      if((itemArray[1].asString()).length() < ASYNC_VALUE_DEFAULT_MAX_CACHE_LIMIT ) {
        if((appLocalDataFile_.find(itemArray[0].asString())) != appLocalDataFile_.items().end()) {
          oldLength = (itemArray[0].asString()).length()+(appLocalDataFile_[itemArray[0]].asString()).length()+6;
          if(oldLength < newLength) {
            requiredLength = newLength - oldLength;
          }else{
            requiredLength = oldLength-newLength;
          }
        }else{
          requiredLength= newLength;
        }
        appLocalDataFile_[itemArray[0].asString()] = itemArray[1].asString();
      }
    }
    totalSize_ += requiredLength;
    if(isWriteScheduled_)
      return;
    RNS_LOG_DEBUG("Scheduling to write the file");
    // Total storage size is capped at 6 MB by default.
    if(totalSize_ < ASYNC_STORAGE_DEFAULT_MAX_CACHE_LIMIT) {
      taskRunner_->scheduleDispatch( [&](){
        writeToFile();        
        isWriteScheduled_ = false;

      }, ASYNC_SRG_TIMEOUT);

    }else{
      RNS_LOG_WARN("async storage size is greater than 6mb");
      return;
    }
    isWriteScheduled_ = true;;
  cb({});
});

}

void RSAsyncStorageModule::writeToFile(){
  string str = toJson(appLocalDataFile_);
  filesystem::resize_file(FILE_PATH, 0);
  appLocalFile_.seekp(0);
  appLocalFile_ << str;
  appLocalFile_.flush();
}

void RSAsyncStorageModule::multiRemove(dynamic args, CxxModule::Callback cb) {
  if(!appLocalFile_.is_open()) {
    dynamic errors = folly::dynamic::object();
    errors["message"] = "Failed to remove the data";
    errors["key"] = args[0][0].asString();
    cb({folly::dynamic::array(errors)});
    return;
  }
  taskRunner_->dispatch( [this,args,cb](){
    dynamic errors = folly::dynamic::object();
    for (auto& itemArray : args[0]) {
      auto pos = appLocalDataFile_.find(itemArray);
      if(pos == appLocalDataFile_.items().end()) {
        errors["message"] = "Failed to remove the data";
        errors["key"] = args[0][0].asString();
      }else {
        appLocalDataFile_.erase(itemArray);
      }
    }
    taskRunner_->scheduleDispatch( [&](){
      writeToFile();
    },ASYNC_SRG_TIMEOUT);
    cb({(errors.empty() ? nullptr : errors)});
  });
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
}//xplat
}//facebook
