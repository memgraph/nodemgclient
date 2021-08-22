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

#include "record.hpp"

#include <cassert>

Record::Record(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<Record>(info) {
  // NOTE: Caller has to take care about the type of parameters.
  assert(info.Length() == 2);
  assert(info[0].Type() == napi_valuetype::napi_external);
  assert(info[1].IsArray());

  this->columns_ =
      info[0].As<Napi::External<std::map<std::string, uint32_t>>>().Data();
  this->values_ = Napi::Persistent(info[1].As<Napi::Array>());
}

Napi::FunctionReference Record::constructor;

Napi::Object Record::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env, "Record",
                  {
                      InstanceMethod("Get", &Record::Get),
                      InstanceMethod("Values", &Record::Values),
                  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("Record", func);
  return exports;
}

Napi::Value Record::Get(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 1) {
    Napi::Error::New(env, "Record.Get accepts exectly one string argument")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  auto maybe_name = info[0];
  if (!maybe_name.IsString()) {
    Napi::Error::New(env, "Record.Get accepts exectly one string argument")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  auto name = maybe_name.As<Napi::String>().Utf8Value();
  return this->values_.Value()[(*this->columns_)[name]];
}

Napi::Value Record::Values(const Napi::CallbackInfo &) {
  return this->values_.Value();
}
