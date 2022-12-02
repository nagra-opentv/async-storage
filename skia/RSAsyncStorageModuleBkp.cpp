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

    //taskRunner_->dispatch( [this](){ 
     // outfile_.open("file.json", ios::out | ios::trunc );
       //openFile and parse amnd store in Globaldynamic;
    //});
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
            //cb({});
            multiGet(args, cb);
          }),
      Method(
          "multiSet",
          [this] (dynamic args, CxxModule::Callback cb) {
            //cb({});
            multiSet(args, cb);
          }),
      Method(
          "multiRemove",
          [this] (dynamic args, CxxModule::Callback cb) {
            //cb({});
            multiRemove(args, cb);
          }),
      Method(
          "mergeItem",
          [this] (dynamic args, CxxModule::Callback cb) {
            RNS_LOG_NOT_IMPL;
            //cb({});
            mergeItem(args, cb);
          }),
      Method(
          "getAllKeys",
          [this] (dynamic args, CxxModule::Callback cb) {
            RNS_LOG_NOT_IMPL;
            //cb({});
            getAllKeys(args, cb);
          }),
      Method(
          "clear",
          [this] (dynamic args, CxxModule::Callback cb) {
            RNS_LOG_NOT_IMPL;
            //cb({});
            clear(args, cb);
          }),

  };
}


void RSAsyncStorageModule::multiGet(dynamic args, CxxModule::Callback cb) {
 // taskRunner_->dispatch( [args,cb](){
  // dynamic errors = folly::dynamic::object();
  // dynamic result = folly::dynamic::array;
  // for (auto& itemArray : args[0]) 
  // { 
  //    if(appLocalDataFile[itemArray[0]] != nullptr){
  //     // result[{"result", {itemArray[0].asString(), itemArray[1].asString()}}];
  //     result.push_back(itemArray[0].asString());
  //     result.push_back(appLocalDataFile[itemArray[0]].asString());
  //    }
  //    else{
  //     errors["message"] = "not found the get Item";
  //     errors["key"] = args[0][0].asString();
  //      //result["errors", {{"message", "not found"}, "key", itemArray[0].asString()}];
  //    }
  // }
     // if(appLocalDataFile[args[0][0]] != nullptr){
     //  RNS_LOG_ERROR("RSAsyncStorage::GetItem "<<appLocalDataFile[args[0][0]]);
     //  // result[{"result", {itemArray[0].asString(), itemArray[1].asString()}}];
     //  result.push_back(args[0][0].asString());
     //  result.push_back(appLocalDataFile[args[0][0]].asString());
     // }
     // else{
     //  errors["message"] = "not found the get Item";
     //  errors["key"] = args[0][0].asString();
     //   //result["errors", {{"message", "not found"}, "key", itemArray[0].asString()}];
     // }  
  cb({});
  //cb({folly::dynamic::array(errors, folly::dynamic::array(result))});
    // cb(folly::dynamic::array(errors),folly::dynamic::array(result));
  //});
}

void RSAsyncStorageModule::storeinFile(string key, string value){
  
}


void RSAsyncStorageModule::multiSet(dynamic args, CxxModule::Callback cb) {

  taskRunner_->dispatch( [this,args,cb](){
  for (auto& itemArray : args[0]) {
      appLocalDataFile[itemArray[0].asString()] = itemArray[1].asString();
    }
        if(isWriteScheduled_)
      return;
    RNS_LOG_ERROR("Schedule filewrite" );
    taskRunner_->scheduleDispatch( [&](){
       RNS_LOG_ERROR("writing to file");
      folly::json::serialization_opts opts;
      string str = folly::json::serialize(appLocalDataFile, opts);
      outfile_.open("file.json", ios::out | ios::trunc );
      if(!outfile_.is_open()) {
        cb({});
        return;
      }
      // truncate file
      fs::resize_file(filename, 0);
      file.seekp(0);
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
  taskRunner_->dispatch( [this,args,cb](){
    appLocalDataFile.erase(args[0][0].asString());
    folly::json::serialization_opts opts;
    string str = folly::json::serialize(appLocalDataFile, opts);

     outfile_.open("file.json", ios::out | ios::trunc );
      outfile_<<str;
      outfile_.close();
    cb({});
  });
  cb({});
}

void RSAsyncStorageModule::mergeItem(dynamic args, CxxModule::Callback cb) {
  RNS_LOG_NOT_IMPL;
  RNS_LOG_ERROR("RSAsyncStorage::mergeItem");
  cb({});
}

void RSAsyncStorageModule::getAllKeys(dynamic args, CxxModule::Callback cb) {
  RNS_LOG_NOT_IMPL;
  RNS_LOG_ERROR("RSAsyncStorage::getAllKeys");
  cb({});
}

void RSAsyncStorageModule::clear(dynamic args, CxxModule::Callback cb) {
  RNS_LOG_NOT_IMPL;
  RNS_LOG_ERROR("RSAsyncStorage::clear");
  cb({});
}
 void RSAsyncStorageModule::asyncWorkerThread() {
  taskRunner_->run();
}
}
}
