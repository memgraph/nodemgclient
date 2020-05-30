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

#include <map>
#include <string>

#include <mgclient.h>
#include <napi.h>

// TODO(gitbuda): Implement a stringify method.

class Result : public Napi::ObjectWrap<Result> {
 public:
  Result(const Napi::CallbackInfo &info);
  static Napi::FunctionReference constructor;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);

  Napi::Value Columns(const Napi::CallbackInfo &info);
  Napi::Value Records(const Napi::CallbackInfo &info);

 private:
  mg_session *mg_session_{nullptr};
  std::map<std::string, uint32_t> columns_;
};
