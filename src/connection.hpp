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

#include <napi.h>

#include <mgclient.h>

// TODO(gitbuda): Handle state based on ConnectionStatus. Testing.
enum class ConnectionStatus {
  Ready = 0,
  InTransaction = 1,
  Executing = 2,
  Closed = 3,
  Bad = -1
};

class Connection : public Napi::ObjectWrap<Connection> {
 public:
  Connection(const Napi::CallbackInfo &info);
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Object NewInstance(Napi::Env env, Napi::Value params);

  Napi::Value Cursor(const Napi::CallbackInfo &info);

  // TODO(gitbuda): Following functions have to be Napi indepentend.
  // These functions will be called outside of the Node main thread.
  Napi::Value Run(const Napi::CallbackInfo &info);
  std::pair<Napi::Value, int> Pull(Napi::Env env);
  Napi::Value Begin(Napi::Env env);
  Napi::Value Commit(Napi::Env env);
  Napi::Value Rollback(Napi::Env env);
  void DiscardAll(Napi::Env env);

  ~Connection();

 private:
  static Napi::FunctionReference constructor;
  Napi::Value HandleError(Napi::Env env, const char *message);
  Napi::Value RunWithoutResults(Napi::Env env, const std::string &query);

  mg_session *session_{nullptr};
  ConnectionStatus status_;
};
