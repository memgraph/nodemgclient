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

#include <mgclient.h>
#include <napi.h>

#include <optional>

namespace nodemg {

[[nodiscard]] std::optional<Napi::Value> MgValueToNapiValue(
    Napi::Env env, const mg_value *input_value);

[[nodiscard]] std::optional<Napi::Value> MgListToNapiArray(
    Napi::Env env, const mg_list *input_list);

[[nodiscard]] std::optional<Napi::Value> MgMapToNapiObject(
    Napi::Env env, const mg_map *input_map);

[[nodiscard]] std::optional<mg_value *> NapiValueToMgValue(
    Napi::Env env, Napi::Value input_value);

[[nodiscard]] std::optional<mg_map *> NapiObjectToMgMap(
    Napi::Env env, Napi::Object input_value);

}  // namespace nodemg
