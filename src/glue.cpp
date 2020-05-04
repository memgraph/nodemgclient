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

Napi::Value MgStringToNapiString(Napi::Env env, const mg_string *input_string) {
  Napi::EscapableHandleScope scope(env);

  Napi::Value output_string = Napi::String::New(
      env, mg_string_data(input_string), mg_string_size(input_string));
  return scope.Escape(napi_value(output_string));
}

std::optional<Napi::Value> MgListToNapiArray(Napi::Env env,
                                             const mg_list *input_list) {
  Napi::EscapableHandleScope scope(env);

  auto input_list_size = mg_list_size(input_list);
  auto output_list = Napi::Array::New(env, input_list_size);
  for (uint32_t index = 0; index < input_list_size; ++index) {
    auto value = MgValueToNapiValue(env, mg_list_at(input_list, index));
    if (!value) {
      return std::nullopt;
    }
    output_list[index] = *value;
  }
  return scope.Escape(napi_value(output_list));
}

std::optional<Napi::Value> MgMapToNapiObject(Napi::Env env,
                                             const mg_map *input_map) {
  Napi::EscapableHandleScope scope(env);

  Napi::Object output_object = Napi::Object::New(env);
  for (uint32_t i = 0; i < mg_map_size(input_map); ++i) {
    auto key = MgStringToNapiString(env, mg_map_key_at(input_map, i));
    auto value = MgValueToNapiValue(env, mg_map_value_at(input_map, i));
    if (!value) {
      return std::nullopt;
    }
    output_object.Set(key, *value);
  }
  return scope.Escape(napi_value(output_object));
}

std::optional<Napi::Value> MgNodeToNapiNode(Napi::Env env,
                                            const mg_node *input_node) {
  Napi::EscapableHandleScope scope(env);

  auto node_id = Napi::BigInt::New(env, mg_node_id(input_node));

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
  return scope.Escape(napi_value(output_node));
}

std::optional<Napi::Value> MgRelationshipToNapiRelationship(
    Napi::Env env, const mg_relationship *input_relationship) {
  Napi::EscapableHandleScope scope(env);

  auto relationship_id =
      Napi::BigInt::New(env, mg_relationship_id(input_relationship));
  auto relationship_start_node_id =
      Napi::BigInt::New(env, mg_relationship_start_id(input_relationship));
  auto relationship_end_node_id =
      Napi::BigInt::New(env, mg_relationship_end_id(input_relationship));

  auto relationship_type =
      MgStringToNapiString(env, mg_relationship_type(input_relationship));

  auto relationship_properties =
      MgMapToNapiObject(env, mg_relationship_properties(input_relationship));
  if (!relationship_properties) {
    return std::nullopt;
  }

  Napi::Object output_relationship = Napi::Object::New(env);
  output_relationship.Set("id", relationship_id);
  output_relationship.Set("startNodeId", relationship_start_node_id);
  output_relationship.Set("endNodeId", relationship_end_node_id);
  output_relationship.Set("type", relationship_type);
  output_relationship.Set("properties", *relationship_properties);
  return scope.Escape(napi_value(output_relationship));
}

std::optional<Napi::Value> MgValueToNapiValue(Napi::Env env,
                                              const mg_value *input_value) {
  Napi::EscapableHandleScope scope(env);
  switch (mg_value_get_type(input_value)) {
    case MG_VALUE_TYPE_NULL:
      return scope.Escape(napi_value(env.Null()));
    case MG_VALUE_TYPE_BOOL:
      return scope.Escape(
          napi_value(Napi::Boolean::New(env, mg_value_bool(input_value))));
    case MG_VALUE_TYPE_INTEGER:
      return scope.Escape(
          napi_value(Napi::BigInt::New(env, mg_value_integer(input_value))));
    case MG_VALUE_TYPE_FLOAT:
      return scope.Escape(
          napi_value(Napi::Number::New(env, mg_value_float(input_value))));
    case MG_VALUE_TYPE_STRING:
      return scope.Escape(
          napi_value(MgStringToNapiString(env, mg_value_string(input_value))));
    case MG_VALUE_TYPE_LIST: {
      auto list_value = MgListToNapiArray(env, mg_value_list(input_value));
      if (!list_value) {
        return std::nullopt;
      }
      return scope.Escape(napi_value(*list_value));
    }
    case MG_VALUE_TYPE_MAP: {
      auto map_value = MgMapToNapiObject(env, mg_value_map(input_value));
      if (!map_value) {
        return std::nullopt;
      }
      return scope.Escape(napi_value(*map_value));
    }
    case MG_VALUE_TYPE_NODE: {
      auto node_value = MgNodeToNapiNode(env, mg_value_node(input_value));
      if (!node_value) {
        return std::nullopt;
      }
      return scope.Escape(napi_value(*node_value));
    }
    case MG_VALUE_TYPE_RELATIONSHIP: {
      auto relationship_value = MgRelationshipToNapiRelationship(
          env, mg_value_relationship(input_value));
      if (!relationship_value) {
        return std::nullopt;
      }
      return scope.Escape(napi_value(*relationship_value));
    }
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
