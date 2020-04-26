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

#include <iostream>

#include "connection.hpp"

Connection::~Connection() {
  if (session_) {
    mg_session_destroy(session_);
  }
}

Napi::FunctionReference Connection::constructor;

Napi::Object Connection::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(
      env, "Connection", {InstanceMethod("execute", &Connection::Execute)});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Connection", func);
  return exports;
}

Connection::Connection(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<Connection>(info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1) {
    Napi::TypeError::New(env,
                         "Wrong number of arguments. One JS object "
                         "containing connection parameters is allowed.")
        .ThrowAsJavaScriptException();
    return;
  }
  if (!info[0].IsObject()) {
    Napi::TypeError::New(env,
                         "Wrong argument type. One JS object "
                         "containing connection parameters is allowed.")
        .ThrowAsJavaScriptException();
    return;
  }
  Napi::Object params = info[0].As<Napi::Object>();

  mg_session_params *mg_params = mg_session_params_make();
  if (!params) {
    Napi::TypeError::New(env, "Failed to allocate parameters.")
        .ThrowAsJavaScriptException();
    return;
  }
  mg_session_params_set_host(mg_params,
                             params.Get("host").ToString().Utf8Value().c_str());
  mg_session_params_set_port(mg_params,
                             params.Get("port").ToNumber().Uint32Value());
  mg_session_params_set_sslmode(mg_params, MG_SSLMODE_REQUIRE);

  int mg_status = mg_connect(mg_params, &session_);
  mg_session_params_destroy(mg_params);
  if (mg_status < 0) {
    mg_session_destroy(session_);
    Napi::TypeError::New(env, "Failed to connect.")
        .ThrowAsJavaScriptException();
    return;
  }
}

Napi::Value Connection::Execute(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::TypeError::New(env, "Not yet implemented.")
      .ThrowAsJavaScriptException();
  return env.Null();
}

