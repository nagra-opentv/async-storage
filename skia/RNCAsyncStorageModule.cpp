/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <folly/json.h>
#include <cxxreact/JsArgumentHelpers.h>
#include "ReactSkia/utils/RnsLog.h"
#include "RNCAsyncStorageModule.h"

using namespace folly;
namespace facebook {
namespace xplat {

RNCAsyncStorageModule::RNCAsyncStorageModule() {
  taskRunner_ = std::make_unique<RnsShell::TaskLoop>();
  workerThread_=std::thread (&RNCAsyncStorageModule::asyncWorkerThread,this);
  taskRunner_->waitUntilRunning();

  taskRunner_->dispatch( [this]() {
    appLocalFile_.open(FILE_PATH, ios::out | ios::in | ios::app);
    if (appLocalFile_.is_open()) {
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
        }catch(exception e) {
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

RNCAsyncStorageModule::~RNCAsyncStorageModule() {
  if(appLocalFile_.is_open()) {
    appLocalFile_.close();
  }
  taskRunner_->stop();
  if (workerThread_.joinable() ) {
    workerThread_.join();
  }
}

auto RNCAsyncStorageModule::getConstants() -> std::map<std::string, folly::dynamic> {
  return {};
}

std::string RNCAsyncStorageModule::getName() {
  return "RNCAsyncStorage";
}

auto RNCAsyncStorageModule::getMethods() -> std::vector<Method> {
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


void RNCAsyncStorageModule::multiGet(dynamic args, CxxModule::Callback cb) {
  taskRunner_->dispatch( [this,args,cb](){
    dynamic errors = folly::dynamic::array;
    dynamic resultArray = folly::dynamic::array;
    for (auto& keyString : args[0]) { 
      auto pos = appLocalDataFile_.find(keyString);
      if(pos == appLocalDataFile_.items().end()){
        dynamic errorItem = folly::dynamic::object();
        errorItem["message"] = "Key not found in local storage";
        errorItem["key"] = keyString;
        errors.push_back(errorItem);
      }else{
        dynamic result = folly::dynamic::array;
        result.push_back(pos->first);
        result.push_back(pos->second);
        resultArray.push_back(result);
      }
    }
    cb({(errors.empty() ? nullptr : errors),(resultArray.empty() ? nullptr : resultArray) });
  });
}

void RNCAsyncStorageModule::multiSet(dynamic args, CxxModule::Callback cb) {
  taskRunner_->dispatch( [this,args,cb]() {
    if(!appLocalFile_.is_open()) {
      dynamic errors = folly::dynamic::array;
      for(auto& itemArray : args[0]){
        dynamic errorItem = folly::dynamic::object();
        errorItem["message"] = "Failed to write the data, internal error";
        errorItem["key"] = itemArray[0].asString();
        errors.push_back(errorItem);
      }
      cb({folly::dynamic::array(errors)});
      return;
    }
    int requiredLength = 0;
    bool needsFileWrite = false;
    dynamic errorList = folly::dynamic::array;
    for (auto& itemArray : args[0]) {
      int oldLength = 0;
      int newLength = (itemArray[0].asString()).length()+itemArray[1].asString().length()+EXTRA_CHARACTER_PADDING_LENGTH;
      // value size cannot be larger than 2 MB
      if((itemArray[1].asString()).length() < ASYNC_STORAGE_VALUE_MAX_LENGTH ) {
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
      } else {
         dynamic errorItem = folly::dynamic::object();
         errorItem["message"] = "Failed to write the data, value size is larger than "+to_string(ASYNC_STORAGE_VALUE_MAX_LENGTH) +"mb";
         errorItem["key"] = itemArray[0].asString();
         errorList.push_back(errorItem);
         continue;
      }
      if(( totalSize_ + requiredLength) < ASYNC_STORAGE_FILE_MAX_LENGTH){
        appLocalDataFile_[itemArray[0].asString()] = itemArray[1].asString();
        needsFileWrite = true;
        totalSize_ += requiredLength;
      }else{
        RNS_LOG_WARN("async storage size is greater than "<<ASYNC_STORAGE_FILE_MAX_LENGTH<< "mb");
        return;
      }
    }

    if(needsFileWrite && isWriteScheduled_ == false){
      taskRunner_->scheduleDispatch( [&](){
        writeToFile();
      }, ASYNC_STORGAE_FILE_WRITE_TIMEOUT);
      needsFileWrite = false;
      isWriteScheduled_ = true;
    }
    cb({(errorList.empty() ? nullptr : errorList)});
    return;
  });

}

void RNCAsyncStorageModule::writeToFile(){
  string str = toJson(appLocalDataFile_);
  filesystem::resize_file(FILE_PATH, 0);
  appLocalFile_.seekp(0);
  appLocalFile_ << str;
  appLocalFile_.flush();
  isWriteScheduled_ = false;
}

void RNCAsyncStorageModule::multiRemove(dynamic args, CxxModule::Callback cb) {
  taskRunner_->dispatch( [this,args,cb](){
    if(!appLocalFile_.is_open()) {
      dynamic errors = folly::dynamic::array;
      for(auto& keyString : args[0]){
        dynamic errorItem = folly::dynamic::object();
        errorItem["message"] = "Failed to remove the data, internal error";
        errorItem["key"] = keyString;
        errors.push_back(errorItem);
      }
      cb({folly::dynamic::array(errors)});
      return;
    }
    dynamic errors = folly::dynamic::object();
    for (auto& keyString : args[0]) {
      auto pos = appLocalDataFile_.find(keyString);
      if(pos == appLocalDataFile_.items().end()) {
        errors["message"] = "Failed to remove the data, key not found";
        errors["key"] = keyString;
      }else {
        appLocalDataFile_.erase(pos->first);
      }
    }
    if(isWriteScheduled_ == false){
      taskRunner_->scheduleDispatch( [&](){
        writeToFile();
      },ASYNC_STORGAE_FILE_WRITE_TIMEOUT);
      isWriteScheduled_ = true;
    }
    cb({(errors.empty() ? nullptr : errors)});
  });
}

void RNCAsyncStorageModule::mergeItem(dynamic args, CxxModule::Callback cb) {
  RNS_LOG_NOT_IMPL;
  cb({});
}

void RNCAsyncStorageModule::getAllKeys(dynamic args, CxxModule::Callback cb) {
  taskRunner_->dispatch( [this,args,cb](){
    dynamic resultArray = folly::dynamic::array;
    dynamic errors = folly::dynamic::object();
    if(!appLocalFile_.is_open()) {
      errors["message"] = "Failed to get the keys, internal error";
      errors["key"] = "";
    }else{
      for(auto& appKeydata :appLocalDataFile_.keys()){
        resultArray.push_back(appKeydata);
      }
    }
    cb({(errors.empty() ? nullptr : errors),(resultArray.empty() ? nullptr : resultArray)});
  });
}

void RNCAsyncStorageModule::clear(dynamic args, CxxModule::Callback cb) {
  taskRunner_->dispatch( [this,args,cb](){
    if(!appLocalFile_.is_open()) {
      dynamic errors = folly::dynamic::object();
      errors["message"] = "Failed to clear the data, internal error";
      errors["key"] = "";
      cb({(errors.empty() ? nullptr : errors)});
      return;
    }else{
      appLocalDataFile_.erase(appLocalDataFile_.items().begin(), appLocalDataFile_.items().end());
      if(isWriteScheduled_ == false){
        taskRunner_->scheduleDispatch( [&](){
          writeToFile();
        },ASYNC_STORGAE_FILE_WRITE_TIMEOUT);
        isWriteScheduled_ = true;
      }
    }
    cb({});
  });
}

void RNCAsyncStorageModule::asyncWorkerThread() {
  taskRunner_->run();
}

#ifdef __cplusplus
extern "C" {
#endif
RNS_EXPORT_MODULE(RNCAsyncStorage)
#ifdef __cplusplus
}
#endif

} // namespace xplat
} // namespace facebook
