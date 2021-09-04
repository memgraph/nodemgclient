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

#include "async_connection.hpp"

#include <cassert>

#include "glue.hpp"
#include "message.hpp"
#include "mgclient.hpp"

Napi::FunctionReference AsyncConnection::constructor;

Napi::Object AsyncConnection::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env, "Connection",
                  {
                      InstanceMethod("Connect", &AsyncConnection::Connect),
                      InstanceMethod("Execute", &AsyncConnection::Execute),
                      InstanceMethod("FetchAll", &AsyncConnection::FetchAll),
                      InstanceMethod("FetchOne", &AsyncConnection::FetchOne),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Connection", func);
  return exports;
}

Napi::Object AsyncConnection::NewInstance(const Napi::CallbackInfo &info) {
  Napi::EscapableHandleScope scope(info.Env());
  Napi::Object obj = constructor.New({});
  return scope.Escape(napi_value(obj)).ToObject();
}

std::optional<mg::Client::Params> create_params(
    const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsObject()) {
    Napi::TypeError::New(env, NODEMG_MSG_CONN_WRONG_ARG)
        .ThrowAsJavaScriptException();
    return std::nullopt;
  }

  Napi::Object user_params = info[0].As<Napi::Object>();
  mg::Client::Params mg_params;

  auto napi_host_value = user_params.Get("host");
  if (!napi_host_value.IsUndefined()) {
    mg_params.host = napi_host_value.ToString().Utf8Value();
  }

  auto napi_port_value = user_params.Get("port");
  if (!napi_port_value.IsUndefined()) {
    auto port = napi_port_value.ToNumber().Uint32Value();
    if (port > std::numeric_limits<decltype(mg_params.port)>::max()) {
      Napi::TypeError::New(env, NODEMG_MSG_CONN_PORT_OUT_OF_RANGE)
          .ThrowAsJavaScriptException();
      return std::nullopt;
    }
    mg_params.port = static_cast<decltype(mg_params.port)>(port);
  }

  auto napi_username_value = user_params.Get("username");
  if (!napi_username_value.IsUndefined()) {
    mg_params.username = napi_username_value.ToString().Utf8Value();
  }

  auto napi_password_value = user_params.Get("password");
  if (!napi_password_value.IsUndefined()) {
    mg_params.password = napi_password_value.ToString().Utf8Value();
  }

  auto napi_client_name_value = user_params.Get("client_name");
  if (!napi_client_name_value.IsUndefined()) {
    mg_params.user_agent = napi_client_name_value.ToString().Utf8Value();
  }

  auto napi_use_ssl_value = user_params.Get("use_ssl");
  if (!napi_use_ssl_value.IsUndefined()) {
    if (napi_use_ssl_value.ToBoolean()) {
      mg_params.use_ssl = MG_SSLMODE_REQUIRE;
    } else {
      mg_params.use_ssl = MG_SSLMODE_DISABLE;
    }
  }

  return mg_params;
}

AsyncConnection::AsyncConnection(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<AsyncConnection>(info), client_(nullptr) {}

AsyncConnection::~AsyncConnection() {}

void AsyncConnection::SetClient(std::unique_ptr<mg::Client> client) {
  this->client_ = std::move(client);
}

class AsyncConnectWorker final : public Napi::AsyncWorker {
 public:
  AsyncConnectWorker(const Napi::Promise::Deferred &deferred,
                     mg::Client::Params params)
      : AsyncWorker(Napi::Function::New(deferred.Promise().Env(),
                                        [](const Napi::CallbackInfo &) {})),
        deferred_(deferred),
        params_(std::move(params)) {}
  ~AsyncConnectWorker() = default;

  void Execute() {
    client_ = mg::Client::Connect(params_);
    if (!client_) {
      std::string msg("Failed to async connect.");
      SetError(msg);
      return;
    }
  }

  void OnOK() {
    Napi::Object obj = AsyncConnection::constructor.New({});
    AsyncConnection *async_connection = AsyncConnection::Unwrap(obj);
    async_connection->SetClient(std::move(client_));
    this->deferred_.Resolve(obj);
  }

  void OnError(const Napi::Error &e) {
    this->deferred_.Reject(Napi::Error::New(Env(), e.Message()).Value());
  }

 private:
  Napi::Promise::Deferred deferred_;
  mg::Client::Params params_;
  std::unique_ptr<mg::Client> client_;
};

// TODO(gitbuda): Try to push AsyncConnection::Connect to constructor.
Napi::Value AsyncConnection::Connect(const Napi::CallbackInfo &info) {
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());

  auto params = create_params(info);
  if (!params) {
    return info.Env().Undefined();
  }

  AsyncConnectWorker *wk = new AsyncConnectWorker(deferred, std::move(*params));
  wk->Queue();
  return deferred.Promise();
}

class AsyncExecuteWorker final : public Napi::AsyncWorker {
 public:
  AsyncExecuteWorker(const Napi::Promise::Deferred &deferred,
                     mg::Client *client, std::string query, mg::ConstMap params)
      : AsyncWorker(Napi::Function::New(deferred.Promise().Env(),
                                        [](const Napi::CallbackInfo &) {})),
        deferred_(deferred),
        client_(client),
        query_(std::move(query)),
        params_(std::move(params)) {}
  ~AsyncExecuteWorker() = default;

  void Execute() {
    auto status = client_->Execute(query_, params_);
    if (!status) {
      std::string msg("Failed to async execute query.");
      SetError(msg);
      return;
    }
  }

  void OnOK() {
    auto env = deferred_.Env();
    this->deferred_.Resolve(env.Null());
  }

  void OnError(const Napi::Error &e) {
    this->deferred_.Reject(Napi::Error::New(Env(), e.Message()).Value());
  }

 private:
  Napi::Promise::Deferred deferred_;
  mg::Client *client_;
  std::string query_;
  mg::ConstMap params_;
};

Napi::Value AsyncConnection::Execute(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  std::string query;
  mg_map *mg_params = NULL;
  if (info.Length() == 1 || info.Length() == 2) {
    auto maybe_query = info[0];
    if (!maybe_query.IsString()) {
      Napi::Error::New(env, NODEMG_MSG_EXEC_QUERY_STRING_ERROR)
          .ThrowAsJavaScriptException();
      return env.Null();
    }
    query = maybe_query.As<Napi::String>().Utf8Value();
  }
  if (info.Length() == 2) {
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

  auto wk = new AsyncExecuteWorker(deferred, client_.get(), std::move(query),
                                   mg::ConstMap(mg_params));
  wk->Queue();
  return deferred.Promise();
}

class AsyncFetchAllWorker final : public Napi::AsyncWorker {
 public:
  AsyncFetchAllWorker(const Napi::Promise::Deferred &deferred,
                      mg::Client *client)
      : AsyncWorker(Napi::Function::New(deferred.Promise().Env(),
                                        [](const Napi::CallbackInfo &) {})),
        deferred_(deferred),
        client_(client) {}
  ~AsyncFetchAllWorker() = default;

  void Execute() {
    try {
      data_ = client_->FetchAll();
    } catch (const std::exception &error) {
      std::string msg =
          std::string("Failed to fetch one data. ") + std::string(error.what());
      SetError(msg);
      return;
    }
  }

  void OnOK() {
    auto env = deferred_.Env();

    if (!data_) {
      this->deferred_.Resolve(env.Null());
      return;
    }

    auto output_array_value = Napi::Array::New(env, data_->size());
    for (uint32_t outer_index = 0; outer_index < data_->size(); ++outer_index) {
      auto inner_array = (*data_)[outer_index];
      auto inner_array_size = inner_array.size();
      auto inner_array_value = Napi::Array::New(env, inner_array_size);
      for (uint32_t inner_index = 0; inner_index < inner_array_size;
           ++inner_index) {
        auto value = MgValueToNapiValue(env, inner_array[inner_index].ptr());
        if (!value) {
          std::string msg("Failed to convert fetched data.");
          SetError(msg);
          return;
        }
        inner_array_value[inner_index] = *value;
      }
      output_array_value[outer_index] = inner_array_value;
    }

    this->deferred_.Resolve(output_array_value);
  }

  void OnError(const Napi::Error &e) {
    this->deferred_.Reject(Napi::Error::New(Env(), e.Message()).Value());
  }

 private:
  Napi::Promise::Deferred deferred_;
  mg::Client *client_;
  decltype(client_->FetchAll()) data_;
};

Napi::Value AsyncConnection::FetchAll(const Napi::CallbackInfo &info) {
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
  auto wk = new AsyncFetchAllWorker(deferred, client_.get());
  wk->Queue();
  return deferred.Promise();
}

class AsyncFetchOneWorker final : public Napi::AsyncWorker {
 public:
  AsyncFetchOneWorker(const Napi::Promise::Deferred &deferred,
                      mg::Client *client)
      : AsyncWorker(Napi::Function::New(deferred.Promise().Env(),
                                        [](const Napi::CallbackInfo &) {})),
        deferred_(deferred),
        client_(client) {}
  ~AsyncFetchOneWorker() = default;

  void Execute() {
    try {
      data_ = client_->FetchOne();
    } catch (const std::exception &error) {
      std::string msg =
          std::string("Failed to fetch one data. ") + std::string(error.what());
      SetError(msg);
      return;
    }
  }

  void OnOK() {
    auto env = deferred_.Env();

    if (!data_) {
      this->deferred_.Resolve(env.Null());
      return;
    }

    auto array_value = Napi::Array::New(env, data_->size());
    for (uint32_t index = 0; index < data_->size(); ++index) {
      auto value = MgValueToNapiValue(env, (*data_)[index].ptr());
      if (!value) {
        std::string msg("Failed to convert fetched data.");
        SetError(msg);
        return;
      }
      array_value[index] = *value;
    }

    this->deferred_.Resolve(array_value);
  }

  void OnError(const Napi::Error &e) {
    this->deferred_.Reject(Napi::Error::New(Env(), e.Message()).Value());
  }

 private:
  Napi::Promise::Deferred deferred_;
  mg::Client *client_;
  decltype(client_->FetchOne()) data_;
};

Napi::Value AsyncConnection::FetchOne(const Napi::CallbackInfo &info) {
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
  auto wk = new AsyncFetchOneWorker(deferred, client_.get());
  wk->Queue();
  return deferred.Promise();
}
