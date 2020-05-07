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
  const char *mg_host = NULL;
  const char *mg_address = NULL;
  int mg_port = -1;
  const char *mg_username = NULL;
  const char *mg_password = NULL;
  const char *mg_client_name = NULL;
  enum mg_sslmode mg_ssl_mode = MG_SSLMODE_REQUIRE;
  const char *mg_ssl_cert = NULL;
  const char *mg_ssl_key = NULL;
  // TODO(gitbuda): Add trust callback.
  // TODO(gitbuda): Write tests.

  // Read and validate all user parameters.
  Napi::Object params = info[0].As<Napi::Object>();
  // Dealing with string from napi is a bit tricky. std::string object has be
  // created upfront in order to preserve the value long enough.
  auto napi_host_value = params.Get("host");
  auto napi_address_value = params.Get("address");
  if ((napi_host_value.IsUndefined() && napi_address_value.IsUndefined()) ||
      (!napi_host_value.IsUndefined() && !napi_address_value.IsUndefined())) {
    Napi::Error::New(env, NODEMG_MSG_HOST_ADDR_MISSING)
        .ThrowAsJavaScriptException();
    return;
  }
  auto napi_host = napi_host_value.ToString().Utf8Value();
  if (!napi_host_value.IsUndefined()) {
    mg_host = napi_host.c_str();
  }
  auto napi_address = napi_address_value.ToString().Utf8Value();
  if (!napi_address_value.IsUndefined()) {
    mg_address = napi_address.c_str();
  }
  auto napi_port_value = params.Get("port");
  if (!napi_port_value.IsUndefined()) {
    mg_port = napi_port_value.ToNumber().Int32Value();
  }
  if (mg_port < 0 || mg_port > 65535) {
    Napi::Error::New(env, NODEMG_MSG_PORT_OUT_OF_RANGE)
        .ThrowAsJavaScriptException();
    return;
  }
  auto napi_username_value = params.Get("username");
  auto napi_username = napi_username_value.ToString().Utf8Value();
  if (!napi_username_value.IsUndefined()) {
    mg_username = napi_username.c_str();
  }
  auto napi_password_value = params.Get("password");
  auto napi_password = napi_password_value.ToString().Utf8Value();
  if (!napi_password_value.IsUndefined()) {
    mg_password = napi_password.c_str();
  }
  auto napi_client_name_value = params.Get("client_name");
  auto napi_client_name = napi_client_name_value.ToString().Utf8Value();
  if (!napi_client_name_value.IsUndefined()) {
    mg_client_name = napi_client_name.c_str();
  }
  auto napi_use_ssl_value = params.Get("use_ssl");
  if (!napi_use_ssl_value.IsUndefined()) {
    bool use_ssl = napi_use_ssl_value.ToBoolean();
    std::cout << use_ssl << std::endl;
    if (use_ssl) {
      mg_ssl_mode = MG_SSLMODE_REQUIRE;
    } else {
      mg_ssl_mode = MG_SSLMODE_DISABLE;
    }
  }
  auto napi_ssl_cert_value = params.Get("ssl_cert");
  auto napi_ssl_cert = napi_ssl_cert_value.ToString().Utf8Value();
  if (!napi_ssl_cert_value.IsUndefined()) {
    mg_ssl_cert = napi_ssl_cert.c_str();
  }
  auto napi_ssl_key_value = params.Get("ssl_key");
  auto napi_ssl_key = napi_ssl_key_value.ToString().Utf8Value();
  if (!napi_ssl_key_value.IsUndefined()) {
    mg_ssl_key = napi_ssl_key.c_str();
  }

  // Pass user parameters to the mgclient.
  mg_session_params *mg_params = mg_session_params_make();
  if (!mg_params) {
    Napi::Error::New(env, NODEMG_MSG_CONN_PARAMS_ALLOC_FAIL)
        .ThrowAsJavaScriptException();
    return;
  }
  if (mg_host) {
    mg_session_params_set_host(mg_params, mg_host);
  }
  mg_session_params_set_port(mg_params, static_cast<uint16_t>(mg_port));
  if (mg_address) {
    mg_session_params_set_address(mg_params, mg_address);
  }
  if (mg_username) {
    mg_session_params_set_username(mg_params, mg_username);
  }
  if (mg_password) {
    mg_session_params_set_password(mg_params, mg_password);
  }
  if (mg_client_name) {
    mg_session_params_set_client_name(mg_params, mg_client_name);
  }
  mg_session_params_set_sslmode(mg_params, mg_ssl_mode);
  if (mg_ssl_cert) {
    mg_session_params_set_sslcert(mg_params, mg_ssl_cert);
  }
  if (mg_ssl_key) {
    mg_session_params_set_sslkey(mg_params, mg_ssl_key);
  }

  // Try to connect to the server.
  int mg_status = mg_connect(mg_params, &session_);
  mg_session_params_destroy(mg_params);
  if (mg_status < 0) {
    std::string connect_error(NODEMG_MSG_CONN_FAIL);
    connect_error.append(" ");
    connect_error.append(mg_session_error(session_));
    mg_session_destroy(session_);
    session_ = nullptr;
    Napi::TypeError::New(env, connect_error).ThrowAsJavaScriptException();
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
  Napi::EscapableHandleScope scope(env);

  if (info.Length() != 1) {
    Napi::Error::New(env, NODEMG_MSG_WRONG_EXECUTE_ARG)
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
      return scope.Escape(napi_value(env.Null()));
    }
    data[index++] = *row;
  }
  return scope.Escape(napi_value(data));
}

