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

#include "connection.hpp"
#include "cursor.hpp"
#include "glue.hpp"
#include "record.hpp"

Cursor::Cursor(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<Cursor>(info) {
  // NOTE: Caller has to take care about the type of parameters.
  assert(info.Length() == 1);
  assert(info[0].Type() == napi_valuetype::napi_external);

  this->connection_ = info[0].As<Napi::External<Connection>>().Data();
}

void Cursor::SetColumns(Napi::Env, Napi::Value columns) {
  assert(columns.IsArray());
  this->columns_.clear();
  auto columns_array = columns.As<Napi::Array>();
  for (uint32_t column_index = 0; column_index < columns_array.Length();
       ++column_index) {
    Napi::Value column_value = columns_array[column_index];
    assert(column_value.IsString());
    std::string column_name = column_value.As<Napi::String>().Utf8Value();
    this->columns_[column_name] = column_index;
  }
}

Napi::FunctionReference Cursor::constructor;

Napi::Object Cursor::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env, "Cursor",
                  {
                      InstanceMethod("Execute", &Cursor::Execute),
                      InstanceMethod("Begin", &Cursor::Begin),
                      InstanceMethod("Commit", &Cursor::Commit),
                      InstanceMethod("Rollback", &Cursor::Rollback),
                      InstanceMethod("Columns", &Cursor::Columns),
                      InstanceMethod("Stream", &Cursor::Stream),
                  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("Cursor", func);
  return exports;
}

Napi::Value Cursor::Columns(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  auto output_array = Napi::Array::New(env, this->columns_.size());
  for (const auto &[name, index] : this->columns_) {
    output_array[index] = Napi::String::New(env, name.c_str());
  }
  return scope.Escape(napi_value(output_array));
}

Napi::Value Cursor::Execute(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  bool lazy = false;
  if (info.Length() == 3) {
    lazy = info[2].As<Napi::Boolean>().Value();
  }

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  // TODO(gitbuda): Execute Run and Pull on the worker thread.
  // TODO(gitbuda): Define error handling, calls to both Throw and Reject.

  // Send query to the server and fetch columns.
  auto columns = connection_->Run(info);
  if (env.IsExceptionPending()) {
    return env.Null();
  }
  this->SetColumns(env, columns);

  auto data = Napi::Array::New(env);
  if (!lazy) {
    uint32_t index = 0;
    while (true) {
      auto record_data = connection_->Pull(env);
      if (env.IsExceptionPending()) {
        deferred.Reject(
            Napi::Error::New(env, "Fail to fetch data from the server")
                .Value());
        break;
      }
      if (record_data.second == 0) {
        break;
      }
      data[index++] = Record::constructor.New(
          {Napi::External<std::map<std::string, uint32_t>>::New(
               env, &this->columns_),
           record_data.first});
    }
  }
  deferred.Resolve(data);
  return deferred.Promise();
}

Napi::Value Cursor::Begin(const Napi::CallbackInfo &info) {
  return this->connection_->Begin(info.Env());
}

Napi::Value Cursor::Commit(const Napi::CallbackInfo &info) {
  return this->connection_->Commit(info.Env());
}

Napi::Value Cursor::Rollback(const Napi::CallbackInfo &info) {
  return this->connection_->Rollback(info.Env());
}

Napi::Value Cursor::Stream(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  Napi::Function emit = info[0].As<Napi::Function>();

  // On start.
  emit.Call({Napi::String::New(env, "start")});

  while (true) {
    auto record_data = connection_->Pull(env);
    if (env.IsExceptionPending()) {
      return Napi::String::New(env, "Fail to fetch data from the server");
    }
    if (record_data.second == 0) {
      break;
    }
    emit.Call({Napi::String::New(env, "record"),
               Record::constructor.New(
                   {Napi::External<std::map<std::string, uint32_t>>::New(
                        env, &this->columns_),
                    record_data.first})});
  }

  // On end.
  emit.Call({Napi::String::New(env, "end")});

  return Napi::String::New(env, "OK");
}
