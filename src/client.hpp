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

#include <napi.h>

#include <mgclient.hpp>

// TODO(gitbuda): Ensure AsyncConnection can't be missused in the concurrent
// environmnt (multiple threads calling the same object).

namespace nodemg {

class Client final : public Napi::ObjectWrap<Client> {
 public:
  static Napi::FunctionReference constructor;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Object NewInstance(const Napi::CallbackInfo &info);

  Client(const Napi::CallbackInfo &info);
  ~Client();
  // Public because it's called from AsyncWorker.
  void SetMgClient(std::unique_ptr<mg::Client> client);

  enum class TxOp { Begin, Commit, Rollback };

  Napi::Value Connect(const Napi::CallbackInfo &info);
  Napi::Value Execute(const Napi::CallbackInfo &info);
  Napi::Value FetchAll(const Napi::CallbackInfo &info);
  Napi::Value DiscardAll(const Napi::CallbackInfo &info);
  Napi::Value FetchOne(const Napi::CallbackInfo &info);
  Napi::Value Begin(const Napi::CallbackInfo &info);
  Napi::Value Commit(const Napi::CallbackInfo &info);
  Napi::Value Rollback(const Napi::CallbackInfo &info);

 private:
  std::unique_ptr<mg::Client> client_;
  std::string name_;

  std::optional<mg::Client::Params> PrepareConnect(
      const Napi::CallbackInfo &info);
  std::optional<std::pair<std::string, mg::ConstMap>> PrepareQuery(
      const Napi::CallbackInfo &info);
};

}  // namespace nodemg
