// Copyright (c) 2016-2020 Memgraph Ltd. [https://memgraph.com]
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cassert>

#include "result.hpp"

#include "glue.hpp"
#include "record.hpp"

#include <chrono>
#include <iostream>
#include <thread>

Result::Result(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<Result>(info) {
  // NOTE: Caller has to take care about the type of parameters.
  assert(info.Length() == 2);
  assert(info[0].Type() == napi_valuetype::napi_external);
  assert(info[1].IsArray());

  this->mg_session_ = info[0].As<Napi::External<mg_session>>().Data();
  auto columns = info[1].As<Napi::Array>();
  for (uint32_t column_index = 0; column_index < columns.Length();
       ++column_index) {
    Napi::Value column_value = columns[column_index];
    assert(column_value.IsString());
    std::string column_name = column_value.As<Napi::String>().Utf8Value();
    this->columns_[column_name] = column_index;
  }
}

Napi::FunctionReference Result::constructor;

Napi::Object Result::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env, "Result",
                  {
                      InstanceMethod("Columns", &Result::Columns),
                      InstanceMethod("Records", &Result::Records),
                  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("Result", func);
  return exports;
}

Napi::Value Result::Columns(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  auto output_array = Napi::Array::New(env, this->columns_.size());
  for (const auto &[name, index] : this->columns_) {
    output_array[index] = Napi::String::New(env, name.c_str());
  }
  return scope.Escape(napi_value(output_array));
}

Napi::Value Result::Records(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  int mg_status;
  // TODO(gitbuda): Figure out how to properly deallocate mg_result if needed.
  mg_result *mg_result;
  uint32_t index = 0;
  auto data = Napi::Array::New(env);
  while ((mg_status = mg_session_pull(this->mg_session_, &mg_result)) == 1) {
    auto row = MgListToNapiArray(env, mg_result_row(mg_result));
    if (!row) {
      // Just return null because the code returning empty row should already
      // throw the JS error.
      return scope.Escape(napi_value(env.Null()));
    }
    data[index++] = Record::constructor.New(
        {Napi::External<std::map<std::string, uint32_t>>::New(env,
                                                              &this->columns_),
         *row});
  }
  deferred.Resolve(data);
  return scope.Escape(napi_value(deferred.Promise()));
}
