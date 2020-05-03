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
#include "glue.hpp"
#include "message.hpp"

Napi::FunctionReference Connection::constructor;

Connection::~Connection() {
  if (session_) {
    mg_session_destroy(session_);
    session_ = nullptr;
  }
}

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
  // Read connection parameters and establish connection if possible connection.
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  if (info.Length() < 1 || !info[0].IsObject()) {
    Napi::TypeError::New(env, NODEMG_MSG_WRONG_CONN_ARG)
        .ThrowAsJavaScriptException();
    return;
  }
  // TODO(gitbuda): Deal with all possible connection parameters.
  Napi::Object params = info[0].As<Napi::Object>();
  mg_session_params *mg_params = mg_session_params_make();
  if (!params) {
    Napi::Error::New(env, NODEMG_MSG_CONN_PARAMS_ALLOC_FAIL)
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
    Napi::TypeError::New(env, NODEMG_MSG_CONN_FAIL)
        .ThrowAsJavaScriptException();
    mg_session_destroy(session_);
    session_ = nullptr;
    return;
  }
}

Napi::Object Connection::NewInstance(Napi::Env env, Napi::Value params) {
  Napi::EscapableHandleScope scope(env);
  Napi::Object obj = constructor.New({params});
  return scope.Escape(napi_value(obj)).ToObject();
}

Napi::Value Connection::Execute(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 1) {
    Napi::TypeError::New(env, NODEMG_MSG_WRONG_EXECUTE_ARG)
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  auto query = info[0].As<Napi::String>().Utf8Value();

  // TODO(gitbuda): Pass query parameters.
  // TODO(gitbuda): Deal with columns.

  int status = mg_session_run(session_, query.c_str(), NULL, NULL);
  if (status != 0) {
    Napi::Error::New(env, NODEMG_MSG_RUN_FAIL).ThrowAsJavaScriptException();
    return env.Null();
  }
  mg_result *result;
  uint32_t index = 0;
  auto data = Napi::Array::New(env);
  while ((status = mg_session_pull(session_, &result)) == 1) {
    auto row = MgListToNapiArray(env, mg_result_row(result));
    if (!row) {
      return env.Null();
    }
    data[index++] = *row;
  }
  return data;
}

