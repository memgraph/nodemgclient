// Copyright (c) 2016-2021 Memgraph Ltd. [https://memgraph.com]
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

#include "connection.hpp"

#include <cassert>

#include "cursor.hpp"
#include "glue.hpp"
#include "message.hpp"

struct TrustData {
  TrustData(Napi::Env e, Napi::Function callback)
      : env(e), trust_callback(callback) {}
  Napi::Env env;
  Napi::Function trust_callback;
};

static int execute_trust_callback(const char *hostname, const char *ip_address,
                                  const char *key_type, const char *fingerprint,
                                  TrustData *trust_data) {
  auto env = trust_data->env;
  auto napi_hostname = Napi::String::New(env, hostname);
  if (env.IsExceptionPending()) {
    env.GetAndClearPendingException();
    return -1;
  }
  auto napi_ip_address = Napi::String::New(env, ip_address);
  if (env.IsExceptionPending()) {
    env.GetAndClearPendingException();
    return -1;
  }
  auto napi_key_type = Napi::String::New(env, key_type);
  if (env.IsExceptionPending()) {
    env.GetAndClearPendingException();
    return -1;
  }
  auto napi_fingerprint = Napi::String::New(env, fingerprint);
  if (env.IsExceptionPending()) {
    env.GetAndClearPendingException();
    return -1;
  }
  auto result = trust_data->trust_callback(
      {napi_hostname, napi_ip_address, napi_key_type, napi_fingerprint});
  if (env.IsExceptionPending()) {
    env.GetAndClearPendingException();
    return -1;
  }
  if (!result.IsBoolean()) {
    return -1;
  }
  if (result.ToBoolean().Value()) {
    return 0;
  } else {
    return -1;
  }
}

// TODO(gitbuda): Add promise.
Connection::Connection(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<Connection>(info) {
  // Read connection parameters and establish connection if possible connection.
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  if (info.Length() < 1 || !info[0].IsObject()) {
    Napi::TypeError::New(env, NODEMG_MSG_CONN_WRONG_ARG)
        .ThrowAsJavaScriptException();
    return;
  }

  // Declare all Memgraph connection parameters.
  const char *mg_host = NULL;
  const char *mg_address = NULL;
  int mg_port = -1;  // TODO(gitbuda): Define 7687 as a default port.
  const char *mg_username = NULL;
  const char *mg_password = NULL;
  const char *mg_client_name = NULL;
  enum mg_sslmode mg_ssl_mode = MG_SSLMODE_DISABLE;
  const char *mg_ssl_cert = NULL;
  const char *mg_ssl_key = NULL;
  std::unique_ptr<TrustData> mg_trust_data;

  // Read and validate all Memgraph connection parameters defined by user.
  Napi::Object params = info[0].As<Napi::Object>();
  // Dealing with string from napi is a bit tricky. std::string object has be
  // created upfront in order to preserve the value long enough.
  auto napi_host_value = params.Get("host");
  auto napi_address_value = params.Get("address");
  if ((napi_host_value.IsUndefined() && napi_address_value.IsUndefined()) ||
      (!napi_host_value.IsUndefined() && !napi_address_value.IsUndefined())) {
    Napi::Error::New(env, NODEMG_MSG_CONN_HOST_ADDR_MISSING)
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
    Napi::Error::New(env, NODEMG_MSG_CONN_PORT_OUT_OF_RANGE)
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
    if (napi_use_ssl_value.ToBoolean()) {
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
  auto napi_trust_callback_value = params.Get("trust_callback");
  if (!napi_trust_callback_value.IsUndefined()) {
    if (!napi_trust_callback_value.IsFunction()) {
      Napi::Error::New(env, NODEMG_MSG_CONN_TRUST_CALLBACK_IS_NOT_FUNCTION)
          .ThrowAsJavaScriptException();
      return;
    }
    auto napi_trust_callback = napi_trust_callback_value.As<Napi::Function>();
    if (!napi_trust_callback_value.IsUndefined()) {
      mg_trust_data = std::make_unique<TrustData>(env, napi_trust_callback);
    }
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
    mg_session_params_set_user_agent(mg_params, mg_client_name);
  }
  mg_session_params_set_sslmode(mg_params, mg_ssl_mode);
  if (mg_ssl_cert) {
    mg_session_params_set_sslcert(mg_params, mg_ssl_cert);
  }
  if (mg_ssl_key) {
    mg_session_params_set_sslkey(mg_params, mg_ssl_key);
  }
  if (mg_trust_data) {
    mg_session_params_set_trust_callback(
        mg_params,
        reinterpret_cast<mg_trust_callback_type>(execute_trust_callback));
    mg_session_params_set_trust_data(
        mg_params, reinterpret_cast<void *>(mg_trust_data.get()));
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

  status_ = ConnectionStatus::Ready;
}

Napi::FunctionReference Connection::constructor;

Napi::Object Connection::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(
      env, "Connection", {InstanceMethod("Cursor", &Connection::Cursor)});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Connection", func);
  return exports;
}

Napi::Object Connection::NewInstance(const Napi::CallbackInfo &info) {
  Napi::EscapableHandleScope scope(info.Env());
  Napi::Object obj = constructor.New({info[0]});
  return scope.Escape(napi_value(obj)).ToObject();
}

Connection::~Connection() {
  if (session_) {
    mg_session_destroy(session_);
    session_ = nullptr;
  }
}

Napi::Value Connection::Run(const Napi::CallbackInfo &info) {
  assert(status_ == ConnectionStatus::Ready ||
         status_ == ConnectionStatus::InTransaction);

  Napi::Env env = info.Env();

  // TODO(gitbuda): Consider passing the exact arguments. Remove info.
  if (info.Length() < 1 || info.Length() > 3) {
    Napi::Error::New(env, NODEMG_MSG_EXEC_WRONG_ARGS)
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  std::string query;
  mg_map *mg_params = NULL;
  if (info.Length() == 1 || info.Length() == 2 || info.Length() == 3) {
    auto maybe_query = info[0];
    if (!maybe_query.IsString()) {
      Napi::Error::New(env, NODEMG_MSG_EXEC_QUERY_STRING_ERROR)
          .ThrowAsJavaScriptException();
      return env.Null();
    }
    query = maybe_query.As<Napi::String>().Utf8Value();
  }
  if (info.Length() == 2 || info.Length() == 3) {
    auto maybe_params = info[1];
    if (!maybe_params.IsObject()) {
      Napi::Error::New(env, NODEMG_MSG_EXEC_QUERY_PARAMS_ERROR)
          .ThrowAsJavaScriptException();
      return env.Null();
    }
    auto params = maybe_params.As<Napi::Object>();
    auto maybe_mg_params = NapiObjectToMgMap(env, params);
    if (!maybe_mg_params) {
      Napi::Error::New(env, NODEMG_MSG_EXEC_CONSTRUCT_QUERY_PARAMS_FAIL)
          .ThrowAsJavaScriptException();
      return env.Null();
    }
    mg_params = *maybe_mg_params;
  }
  const mg_list *mg_columns;
  std::optional<Napi::Value> columns;

  int status = mg_session_run(session_, query.c_str(), mg_params, NULL,
                              &mg_columns, NULL);
  mg_map_destroy(mg_params);
  if (status != 0) {
    return HandleError(env, NODEMG_MSG_EXEC_FAIL);
  }

  status_ = ConnectionStatus::Executing;
  if (mg_columns) {
    columns = MgListToNapiArray(env, mg_columns);
    if (columns) {
      return *columns;
    } else {
      status_ = ConnectionStatus::Bad;
      Napi::Error::New(env, "Unable to fetch columns from server.")
          .ThrowAsJavaScriptException();
      return env.Null();
    }
  } else {
    return Napi::Array::New(env);
  }
}

Napi::Value Connection::Cursor(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  return Cursor::constructor.New({Napi::External<Connection>::New(env, this)});
}

Napi::Value Connection::Begin(Napi::Env env) {
  assert(status_ == ConnectionStatus::Ready);

  // Send BEGIN command and expect no results.
  auto run_status = RunWithoutResults(env, "BEGIN");
  if (env.IsExceptionPending()) {
    status_ = ConnectionStatus::Bad;
    return run_status;
  }

  status_ = ConnectionStatus::InTransaction;
  return env.Null();
}

Napi::Value Connection::Commit(Napi::Env env) {
  assert(status_ == ConnectionStatus::InTransaction);

  // Send COMMIT command and expect no results.
  auto run_status = RunWithoutResults(env, "COMMIT");
  if (env.IsExceptionPending()) {
    status_ = ConnectionStatus::Bad;
    return run_status;
  }

  status_ = ConnectionStatus::Ready;
  return env.Null();
}

Napi::Value Connection::Rollback(Napi::Env env) {
  assert(status_ == ConnectionStatus::InTransaction);

  // Send COMMIT command and expect no results.
  auto run_status = RunWithoutResults(env, "ROLLBACK");
  if (env.IsExceptionPending()) {
    status_ = ConnectionStatus::Bad;
    return run_status;
  }

  status_ = ConnectionStatus::Ready;
  return env.Null();
}

std::pair<Napi::Value, int> Connection::Pull(Napi::Env env) {
  assert(status_ == ConnectionStatus::Executing);

  // TODO(gitbuda): Pass n to the pull call.
  int mg_pull_status = mg_session_pull(session_, NULL);
  if (mg_pull_status != 0) {
    status_ = ConnectionStatus::Bad;
    return {HandleError(env, "Unable to pull data from server"), -1};
  }

  status_ = ConnectionStatus::Fetching;
  return {env.Null(), 0};
}

std::pair<Napi::Value, int> Connection::Fetch(Napi::Env env) {
  assert(status_ == ConnectionStatus::Fetching);

  mg_result *mg_result;
  int mg_status = mg_session_fetch(session_, &mg_result);
  if (mg_status != 0 && mg_status != 1) {
    status_ = ConnectionStatus::Bad;
    return {HandleError(env, "Unable to pull data from server"), -1};
  }

  if (mg_status == 0) {
    auto summary = MgMapToNapiObject(env, mg_result_summary(mg_result));
    if (!summary) {
      status_ = ConnectionStatus::Bad;
      return {HandleError(env, "Unable to fetch summary from server"), -1};
    }
    status_ = ConnectionStatus::Ready;
    return {*summary, 0};
  }

  auto data = MgListToNapiArray(env, mg_result_row(mg_result));
  if (!data) {
    DiscardAll(env);
    return {HandleError(env, "Unable to fetch data from server"), -1};
  }
  return {*data, mg_status};
}

Napi::Value Connection::HandleError(Napi::Env env, const char *message) {
  std::string execute_error(message);
  execute_error.append(" ");
  execute_error.append(mg_session_error(session_));
  Napi::Error::New(env, execute_error).ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value Connection::RunWithoutResults(Napi::Env env,
                                          const std::string &query) {
  int run_status =
      mg_session_run(session_, query.c_str(), NULL, NULL, NULL, NULL);
  if (run_status != 0) {
    return HandleError(env, NODEMG_MSG_EXEC_FAIL);
  }

  int pull_status = mg_session_pull(session_, NULL);
  if (pull_status != 0) {
    return HandleError(env, NODEMG_MSG_EXEC_FAIL);
  }

  while (1) {
    mg_result *result;
    int fetch_status = mg_session_fetch(session_, &result);
    if (fetch_status == 1) {
      continue;
    }

    if (fetch_status == 0) {
      break;
    }

    status_ = ConnectionStatus::Bad;
    return HandleError(
        env, "Unexpected data received after executing without results");
  }

  return env.Null();
}

// TODO(gitbuda): Standardize error handling ExecuteWithout and DiscardAll.

void Connection::DiscardAll(Napi::Env env) {
  int pull_status = mg_session_pull(session_, NULL);
  if (pull_status != 0) {
    status_ = ConnectionStatus::Bad;
    Napi::Error::New(env, "There was and error while discarding.")
        .ThrowAsJavaScriptException();
    return;
  }

  while (1) {
    mg_result *result;
    int fetch_status = mg_session_fetch(session_, &result);
    if (fetch_status == 1) {
      continue;
    }

    if (fetch_status == 0) {
      break;
    }

    status_ = ConnectionStatus::Bad;
    Napi::Error::New(env, "There was and error while discarding.")
        .ThrowAsJavaScriptException();
    return;
  }
}
