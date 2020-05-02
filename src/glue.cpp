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

#include "glue.hpp"
#include "message.hpp"

Napi::Value MgStringToNapiString(const Napi::Env &env,
                                 const mg_string *input_string) {
  return Napi::String::New(env, mg_string_data(input_string),
                           mg_string_size(input_string));
}

std::optional<Napi::Value> MgListToNapiArray(const Napi::Env &env,
                                             const mg_list *input_list) {
  auto input_list_size = mg_list_size(input_list);
  auto output_list = Napi::Array::New(env, input_list_size);
  for (uint32_t index = 0; index < input_list_size; ++index) {
    auto value = MgValueToNapiValue(env, mg_list_at(input_list, index));
    if (!value) {
      return std::nullopt;
    }
    output_list[index] = *value;
  }
  return output_list;
}

std::optional<Napi::Value> MgMapToNapiObject(const Napi::Env &env,
                                             const mg_map *input_map) {
  Napi::Object output_object = Napi::Object::New(env);
  for (uint32_t i = 0; i < mg_map_size(input_map); ++i) {
    auto key = MgStringToNapiString(env, mg_map_key_at(input_map, i));
    auto value = MgValueToNapiValue(env, mg_map_value_at(input_map, i));
    if (!value) {
      return std::nullopt;
    }
    output_object.Set(key, *value);
  }
  return output_object;
}

std::optional<Napi::Value> MgNodeToNapiNode(const Napi::Env &env,
                                            const mg_node *input_node) {
  // TODO(gitbuda): BigInt should be here but it's still experimental.
  auto node_id = Napi::Number::New(env, mg_node_id(input_node));

  auto label_count = mg_node_label_count(input_node);
  auto node_labels = Napi::Array::New(env, label_count);
  for (uint32_t label_index = 0; label_index < label_count; ++label_index) {
    auto label =
        MgStringToNapiString(env, mg_node_label_at(input_node, label_index));
    node_labels[label_index] = label;
  }

  auto node_properties = MgMapToNapiObject(env, mg_node_properties(input_node));
  if (!node_properties) {
    return std::nullopt;
  }

  Napi::Object output_node = Napi::Object::New(env);
  output_node.Set("id", node_id);
  output_node.Set("labels", node_labels);
  output_node.Set("properties", *node_properties);
  return output_node;
}

std::optional<Napi::Value> MgValueToNapiValue(const Napi::Env &env,
                                              const mg_value *input_value) {
  switch (mg_value_get_type(input_value)) {
    case MG_VALUE_TYPE_NULL:
      return env.Null();
    case MG_VALUE_TYPE_BOOL:
      return Napi::Boolean::New(env, mg_value_bool(input_value));
    case MG_VALUE_TYPE_INTEGER:
      // TODO(gitbuda): BigInt should be here but it's still experimental.
      return Napi::Number::New(env, mg_value_integer(input_value));
    case MG_VALUE_TYPE_FLOAT:
      return Napi::Number::New(env, mg_value_float(input_value));
    case MG_VALUE_TYPE_STRING:
      return MgStringToNapiString(env, mg_value_string(input_value));
    case MG_VALUE_TYPE_LIST:
      return MgListToNapiArray(env, mg_value_list(input_value));
    case MG_VALUE_TYPE_MAP:
      return MgMapToNapiObject(env, mg_value_map(input_value));
    case MG_VALUE_TYPE_NODE:
      return MgNodeToNapiNode(env, mg_value_node(input_value));
    case MG_VALUE_TYPE_RELATIONSHIP:
      Napi::Error::New(env, "Fetching of edges not yet implemented.")
          .ThrowAsJavaScriptException();
      return std::nullopt;
    case MG_VALUE_TYPE_PATH:
      Napi::Error::New(env, "Fetching of paths not yet implemented.")
          .ThrowAsJavaScriptException();
      return std::nullopt;
    case MG_VALUE_TYPE_UNKNOWN:
      Napi::Error::New(env, "Fetching of unknowns not yet implemented.")
          .ThrowAsJavaScriptException();
      return std::nullopt;
    case MG_VALUE_TYPE_UNBOUND_RELATIONSHIP:
      Napi::Error::New(env, "Fetching of unbound edges not yet implemented.")
          .ThrowAsJavaScriptException();
      return std::nullopt;
    default:
      Napi::TypeError::New(env, NODEMG_MSG_TYPE_ERROR)
          .ThrowAsJavaScriptException();
      return std::nullopt;
  }
}
