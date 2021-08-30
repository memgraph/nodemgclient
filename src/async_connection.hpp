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

class AsyncConnection final : public Napi::ObjectWrap<AsyncConnection> {
 public:
  static Napi::FunctionReference constructor;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Object NewInstance(const Napi::CallbackInfo &info);

  AsyncConnection(const Napi::CallbackInfo &info);
  ~AsyncConnection();
  void SetClient(std::unique_ptr<mg::Client> client);

  Napi::Value Connect(const Napi::CallbackInfo &info);
  Napi::Value Execute(const Napi::CallbackInfo &info);

 private:
  std::unique_ptr<mg::Client> client_;
};
