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
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Connection", func);
  return exports;
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

Napi::Value AsyncConnection::Execute(const Napi::CallbackInfo &info) {
  return info.Env().Undefined();
}

class AsyncConnectWorker : public Napi::AsyncWorker {
 public:
  AsyncConnectWorker(const Napi::Promise::Deferred &deferred,
                     mg::Client::Params params)
      : AsyncWorker(Napi::Function::New(deferred.Promise().Env(),
                                        [](const Napi::CallbackInfo &) {})),
        deferred_(deferred),
        params_(std::move(params)) {}
  ~AsyncConnectWorker() {}

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

Napi::Object AsyncConnection::NewInstance(const Napi::CallbackInfo &info) {
  Napi::EscapableHandleScope scope(info.Env());
  Napi::Object obj = constructor.New({});
  return scope.Escape(napi_value(obj)).ToObject();
}
