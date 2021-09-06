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

#include "client.hpp"

#include <cassert>

#include "glue.hpp"
#include "mgclient.hpp"
#include "util.hpp"

namespace nodemg {

static const std::string CFG_HOST = "host";
static const std::string CFG_PORT = "port";
static const std::string CFG_USERNAME = "username";
static const std::string CFG_PASSWORD = "password";
static const std::string CFG_CLIENT_NAME = "client_name";
static const std::string CFG_USE_SSL = "use_ssl";

Napi::FunctionReference Client::constructor;

Napi::Object Client::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env, "Client",
                  {
                      InstanceMethod("Connect", &Client::Connect),
                      InstanceMethod("Execute", &Client::Execute),
                      InstanceMethod("FetchAll", &Client::FetchAll),
                      InstanceMethod("DiscardAll", &Client::DiscardAll),
                      InstanceMethod("FetchOne", &Client::FetchOne),
                      InstanceMethod("Begin", &Client::Begin),
                      InstanceMethod("Commit", &Client::Commit),
                      InstanceMethod("Rollback", &Client::Rollback),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Client", func);
  return exports;
}

Napi::Object Client::NewInstance(const Napi::CallbackInfo &info) {
  Napi::EscapableHandleScope scope(info.Env());
  Napi::Object obj = constructor.New({info[0]});
  return scope.Escape(napi_value(obj)).ToObject();
}

std::optional<mg::Client::Params> Client::PrepareConnect(
    const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  mg::Client::Params mg_params;
  mg_params.user_agent = name_;

  if (info.Length() < 1) {
    return mg_params;
  }

  static const std::string NODEMG_MSG_WRONG_CONNECT_ARG =
      "Wrong connect argument. An object containing { host, port, username, "
      "password, client_name, use_ssl } is required. All arguments are "
      "optional.";
  if (!info[0].IsObject()) {
    NODEMG_THROW(NODEMG_MSG_WRONG_CONNECT_ARG);
    return std::nullopt;
  }

  Napi::Object user_params = info[0].As<Napi::Object>();
  // Used to report an error if user misspelled any argument.
  uint32_t counter = 0;

  if (user_params.Has(CFG_HOST)) {
    counter++;
    auto napi_host = user_params.Get(CFG_HOST);
    if (!napi_host.IsString()) {
      NODEMG_THROW("`host` connect argument has to be string.");
      return std::nullopt;
    }
    mg_params.host = napi_host.ToString().Utf8Value();
  }

  if (user_params.Has(CFG_PORT)) {
    counter++;
    auto napi_port = user_params.Get(CFG_PORT);
    if (!napi_port.IsNumber()) {
      NODEMG_THROW("`port` connect argument has to be number.");
      return std::nullopt;
    }
    auto port = napi_port.ToNumber().Uint32Value();
    if (port > std::numeric_limits<decltype(mg_params.port)>::max()) {
      NODEMG_THROW(
          "`port` connect argument out of range. Port has to be a number "
          "between 0 and 65535.");
      return std::nullopt;
    }
    mg_params.port = static_cast<decltype(mg_params.port)>(port);
  }

  if (user_params.Has(CFG_USERNAME)) {
    counter++;
    auto napi_username = user_params.Get(CFG_USERNAME);
    if (!napi_username.IsString()) {
      NODEMG_THROW("`username` connect argument has to be string.");
      return std::nullopt;
    }
    mg_params.username = napi_username.ToString().Utf8Value();
  }

  if (user_params.Has(CFG_PASSWORD)) {
    counter++;
    auto napi_password = user_params.Get(CFG_PASSWORD);
    if (!napi_password.IsString()) {
      NODEMG_THROW("`password` connect argument has to be string.");
      return std::nullopt;
    }
    mg_params.username = napi_password.ToString().Utf8Value();
  }

  if (user_params.Has(CFG_CLIENT_NAME)) {
    counter++;
    auto napi_client_name = user_params.Get(CFG_CLIENT_NAME);
    if (!napi_client_name.IsString()) {
      NODEMG_THROW("`client_name` connect argument has to be string.");
    }
    mg_params.user_agent = napi_client_name.ToString().Utf8Value();
  }

  if (user_params.Has(CFG_USE_SSL)) {
    counter++;
    auto napi_use_ssl = user_params.Get("use_ssl");
    if (!napi_use_ssl.IsBoolean()) {
      NODEMG_THROW("`use_ssl` connect argument has to be boolean.");
      return std::nullopt;
    }
    if (napi_use_ssl.ToBoolean()) {
      mg_params.use_ssl = MG_SSLMODE_REQUIRE;
    } else {
      mg_params.use_ssl = MG_SSLMODE_DISABLE;
    }
  }

  if (user_params.GetPropertyNames().Length() != counter) {
    NODEMG_THROW(NODEMG_MSG_WRONG_CONNECT_ARG);
    return std::nullopt;
  }

  return mg_params;
}

std::optional<std::pair<std::string, mg::ConstMap>> Client::PrepareQuery(
    const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  std::string query;
  mg_map *query_params = NULL;

  if (info.Length() == 1 || info.Length() == 2) {
    auto maybe_query = info[0];
    if (!maybe_query.IsString()) {
      NODEMG_THROW("The first execute argument has to be string.");
      return std::nullopt;
    }
    query = maybe_query.As<Napi::String>().Utf8Value();
  }

  if (info.Length() == 2) {
    auto maybe_params = info[1];
    if (!maybe_params.IsObject()) {
      NODEMG_THROW(
          "The second execute argument has to be an object containing query "
          "parameters.");
      return std::nullopt;
    }
    auto params = maybe_params.As<Napi::Object>();
    auto maybe_mg_params = NapiObjectToMgMap(env, params);
    if (!maybe_mg_params) {
      NODEMG_THROW("Unable to create query parameters object.");
      return std::nullopt;
    }
    query_params = *maybe_mg_params;
  }

  return std::make_pair(query, mg::ConstMap(query_params));
}

Client::Client(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<Client>(info), client_(nullptr), name_("nodemgclient") {
  if (info.Length() == 1) {
    name_ = info[0].As<Napi::String>().Utf8Value();
  }
}

Client::~Client() {}

void Client::SetMgClient(std::unique_ptr<mg::Client> client) {
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
    static const std::string NODEMG_MSG_CONNECT_FAILED =
        "Connect failed. Ensure Memgraph is running and Client is properly "
        "configured.";
    try {
      client_ = mg::Client::Connect(params_);
      if (!client_) {
        SetError(NODEMG_MSG_CONNECT_FAILED);
        return;
      }
    } catch (const std::exception &error) {
      SetError(NODEMG_MSG_CONNECT_FAILED + " " + error.what());
      return;
    }
  }

  void OnOK() {
    Napi::Object obj = Client::constructor.New({});
    Client *async_connection = Client::Unwrap(obj);
    async_connection->SetMgClient(std::move(client_));
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

Napi::Value Client::Connect(const Napi::CallbackInfo &info) {
  auto env = info.Env();

  if (client_) {
    NODEMG_THROW("Already connected.");
    return env.Undefined();
  }

  auto params = PrepareConnect(info);
  if (!params) {
    return env.Undefined();
  }

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
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
    static const std::string NODEMG_MSG_EXECUTE_FAIL =
        "Failed to execute a query.";
    try {
      auto status = client_->Execute(query_, params_);
      if (!status) {
        SetError(NODEMG_MSG_EXECUTE_FAIL);
        return;
      }
    } catch (const std::exception &error) {
      SetError(NODEMG_MSG_EXECUTE_FAIL + " " + error.what());
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

Napi::Value Client::Execute(const Napi::CallbackInfo &info) {
  auto env = info.Env();

  auto query_params = PrepareQuery(info);
  if (!query_params) {
    return info.Env().Undefined();
  }

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  auto wk = new AsyncExecuteWorker(deferred, client_.get(),
                                   std::move(query_params->first),
                                   std::move(query_params->second));
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
    static const std::string NODEMG_MSG_FETCH_ONE_FAIL =
        "Failed to fetch one record.";
    try {
      data_ = client_->FetchAll();
    } catch (const std::exception &error) {
      SetError(NODEMG_MSG_FETCH_ONE_FAIL + error.what());
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
          SetError("Failed to convert fetched data.");
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

Napi::Value Client::FetchAll(const Napi::CallbackInfo &info) {
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
  auto wk = new AsyncFetchAllWorker(deferred, client_.get());
  wk->Queue();
  return deferred.Promise();
}

class AsyncDiscardAllWorker final : public Napi::AsyncWorker {
 public:
  AsyncDiscardAllWorker(const Napi::Promise::Deferred &deferred,
                        mg::Client *client)
      : AsyncWorker(Napi::Function::New(deferred.Promise().Env(),
                                        [](const Napi::CallbackInfo &) {})),
        deferred_(deferred),
        client_(client) {}
  ~AsyncDiscardAllWorker() = default;

  void Execute() {
    static const std::string NODEMG_MSG_DISCARD_ALL_FAIL =
        "Failed to discard all data.";
    try {
      client_->DiscardAll();
    } catch (const std::exception &error) {
      SetError(NODEMG_MSG_DISCARD_ALL_FAIL + error.what());
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
};

Napi::Value Client::DiscardAll(const Napi::CallbackInfo &info) {
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
  auto wk = new AsyncDiscardAllWorker(deferred, client_.get());
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
    static const std::string NODEMG_MSG_FETCH_ONE_FAIL =
        "Failed to fetch one record.";
    try {
      data_ = client_->FetchOne();
    } catch (const std::exception &error) {
      SetError(NODEMG_MSG_FETCH_ONE_FAIL + error.what());
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
        SetError("Failed to convert fetched data.");
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

Napi::Value Client::FetchOne(const Napi::CallbackInfo &info) {
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
  auto wk = new AsyncFetchOneWorker(deferred, client_.get());
  wk->Queue();
  return deferred.Promise();
}

class AsyncTxOpWoker final : public Napi::AsyncWorker {
 public:
  AsyncTxOpWoker(const Napi::Promise::Deferred &deferred, mg::Client *client,
                 Client::TxOp tx_op)
      : AsyncWorker(Napi::Function::New(deferred.Promise().Env(),
                                        [](const Napi::CallbackInfo &) {})),
        deferred_(deferred),
        client_(client),
        tx_op_(tx_op) {}
  ~AsyncTxOpWoker() = default;

  void Execute() {
    try {
      switch (tx_op_) {
        case Client::TxOp::Begin: {
          auto status = client_->BeginTransaction();
          if (!status) {
            SetError("Fail to BEGIN transaction.");
            return;
          }
          break;
        }
        case Client::TxOp::Commit: {
          auto status = client_->CommitTransaction();
          if (!status) {
            SetError("Fail to COMMIT transaction.");
            return;
          }
          break;
        }
        case Client::TxOp::Rollback: {
          auto status = client_->RollbackTransaction();
          if (!status) {
            SetError("Fail to ROLLBACK transaction.");
            return;
          }
          break;
        }
        default:
          throw std::runtime_error("Wrong transaction operation.");
      }
    } catch (const std::exception &error) {
      static const std::string NODEMG_MSG_TXOP_FAIL =
          "Fail to execute transaction operation.";
      SetError(NODEMG_MSG_TXOP_FAIL + " " + error.what());
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
  Client::TxOp tx_op_;
};

Napi::Value Client::Begin(const Napi::CallbackInfo &info) {
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
  auto wk = new AsyncTxOpWoker(deferred, client_.get(), Client::TxOp::Begin);
  wk->Queue();
  return deferred.Promise();
}

Napi::Value Client::Commit(const Napi::CallbackInfo &info) {
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
  auto wk = new AsyncTxOpWoker(deferred, client_.get(), Client::TxOp::Commit);
  wk->Queue();
  return deferred.Promise();
}

Napi::Value Client::Rollback(const Napi::CallbackInfo &info) {
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
  auto wk = new AsyncTxOpWoker(deferred, client_.get(), Client::TxOp::Rollback);
  wk->Queue();
  return deferred.Promise();
}

}  // namespace nodemg
