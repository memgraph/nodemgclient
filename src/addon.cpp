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

#include "connection.hpp"

Napi::Object CreateConnection(const Napi::CallbackInfo &info) {
  return Connection::NewInstance(info.Env(), info[0]);
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  Napi::Object new_exports =
      Napi::Function::New(env, CreateConnection, "Connection");
  return Connection::Init(env, new_exports);
}

NODE_API_MODULE(addon, InitAll)
