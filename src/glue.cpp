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

#include "util.hpp"

namespace nodemg {

Napi::Value MgStringToNapiString(Napi::Env env, const mg_string *input_string) {
  Napi::EscapableHandleScope scope(env);
  Napi::Value output_string = Napi::String::New(
      env, mg_string_data(input_string), mg_string_size(input_string));
  return scope.Escape(napi_value(output_string));
}

Napi::Value MgDateToNapiDate(Napi::Env env, const mg_date *input) {
  Napi::EscapableHandleScope scope(env);
  Napi::Object output = Napi::Object::New(env);
  auto days = mg_date_days(input);
  output.Set("objectType", "date");
  output.Set("days", Napi::BigInt::New(env, days));
  output.Set("date", Napi::Date::New(env, days * 24 * 60 * 60 * 1000));
  return scope.Escape(napi_value(output));
}

Napi::Value MgLocalTimeToNapiLocalTime(Napi::Env env,
                                       const mg_local_time *input) {
  Napi::EscapableHandleScope scope(env);
  auto nanoseconds = mg_local_time_nanoseconds(input);
  Napi::Object output = Napi::Object::New(env);
  output.Set("objectType", "local_time");
  output.Set("nanoseconds", Napi::BigInt::New(env, nanoseconds));
  return scope.Escape(napi_value(output));
}

Napi::Value MgLocalDateTimeToNapiDate(Napi::Env env,
                                      const mg_local_date_time *input) {
  Napi::EscapableHandleScope scope(env);
  auto seconds = mg_local_date_time_seconds(input);
  auto nanoseconds = mg_local_date_time_nanoseconds(input);
  // NOTE: An obvious loss of precision (nanoseconds to milliseconds).
  auto milliseconds = 1.0 * (seconds * 1000 + nanoseconds / 10000000);
  Napi::Object output = Napi::Object::New(env);
  output.Set("objectType", "local_date_time");
  output.Set("seconds", Napi::BigInt::New(env, seconds));
  output.Set("nanoseconds", Napi::BigInt::New(env, nanoseconds));
  output.Set("date", Napi::Date::New(env, milliseconds));
  return scope.Escape(napi_value(output));
}

Napi::Value MgDurationToNapiDuration(Napi::Env env, const mg_duration *input) {
  Napi::EscapableHandleScope scope(env);
  auto days = mg_duration_days(input);
  auto seconds = mg_duration_seconds(input);
  auto nanoseconds = mg_duration_nanoseconds(input);
  Napi::Object output = Napi::Object::New(env);
  output.Set("objectType", "duration");
  output.Set("days", Napi::BigInt::New(env, days));
  output.Set("seconds", Napi::BigInt::New(env, seconds));
  output.Set("nanoseconds", Napi::BigInt::New(env, nanoseconds));
  return scope.Escape(napi_value(output));
}

std::optional<Napi::Value> MgListToNapiArray(Napi::Env env,
                                             const mg_list *input_list) {
  Napi::EscapableHandleScope scope(env);
  auto input_list_size = mg_list_size(input_list);
  auto output_array = Napi::Array::New(env, input_list_size);
  for (uint32_t index = 0; index < input_list_size; ++index) {
    auto value = MgValueToNapiValue(env, mg_list_at(input_list, index));
    if (!value) {
      return std::nullopt;
    }
    output_array[index] = *value;
  }
  return scope.Escape(output_array);
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
  output_relationship.Set("edgeType", relationship_type);
  output_relationship.Set("properties", *relationship_properties);
  return scope.Escape(napi_value(output_relationship));
}

std::optional<Napi::Value> MgUnboundRelationshipToNapiRelationship(
    Napi::Env env, const mg_unbound_relationship *input_unbound_relationship) {
  Napi::EscapableHandleScope scope(env);

  auto relationship_id = Napi::BigInt::New(
      env, mg_unbound_relationship_id(input_unbound_relationship));
  int64_t relationship_start_node_id = -1;
  int64_t relationship_end_node_id = -1;

  auto relationship_type = MgStringToNapiString(
      env, mg_unbound_relationship_type(input_unbound_relationship));

  auto relationship_properties = MgMapToNapiObject(
      env, mg_unbound_relationship_properties(input_unbound_relationship));
  if (!relationship_properties) {
    return std::nullopt;
  }

  Napi::Object output_relationship = Napi::Object::New(env);
  output_relationship.Set("id", relationship_id);
  output_relationship.Set("startNodeId", relationship_start_node_id);
  output_relationship.Set("endNodeId", relationship_end_node_id);
  output_relationship.Set("edgeType", relationship_type);
  output_relationship.Set("properties", *relationship_properties);
  return scope.Escape(napi_value(output_relationship));
}

std::optional<Napi::Value> MgPathToNapiPath(Napi::Env env,
                                            const mg_path *input_path) {
  Napi::EscapableHandleScope scope(env);

  auto nodes = Napi::Array::New(env);
  auto relationships = Napi::Array::New(env);
  int64_t prev_node_id = -1;
  for (uint32_t index = 0; index <= mg_path_length(input_path); ++index) {
    int64_t curr_node_id = mg_node_id(mg_path_node_at(input_path, index));
    auto node = MgNodeToNapiNode(env, mg_path_node_at(input_path, index));
    if (!node) {
      return std::nullopt;
    }
    nodes[index] = *node;
    if (index > 0) {
      auto relationship = MgUnboundRelationshipToNapiRelationship(
          env, mg_path_relationship_at(input_path, index - 1));
      if (!relationship) {
        return std::nullopt;
      }
      if (mg_path_relationship_reversed_at(input_path, index - 1)) {
        relationship->As<Napi::Object>().Set(
            "startNodeId", Napi::BigInt::New(env, curr_node_id));
        relationship->As<Napi::Object>().Set(
            "endNodeId", Napi::BigInt::New(env, prev_node_id));
      } else {
        relationship->As<Napi::Object>().Set(
            "startNodeId", Napi::BigInt::New(env, prev_node_id));
        relationship->As<Napi::Object>().Set(
            "endNodeId", Napi::BigInt::New(env, curr_node_id));
      }
      relationships[index - 1] = *relationship;
    }
    prev_node_id = curr_node_id;
  }

  Napi::Object output_path = Napi::Object::New(env);
  output_path.Set("nodes", nodes);
  output_path.Set("relationships", relationships);
  return scope.Escape(napi_value(output_path));
}

// TODO(gitbuda): Consider the error handling because this method is used to
// during e.g. list construction which doesn't have any error handling inside.
// Policy has to be created. It probably makes sense to have both because
// more granular error messages could be presented to the user.
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
    case MG_VALUE_TYPE_DATE:
      return scope.Escape(
          napi_value(MgDateToNapiDate(env, mg_value_date(input_value))));
    case MG_VALUE_TYPE_LOCAL_TIME:
      return scope.Escape(napi_value(
          MgLocalTimeToNapiLocalTime(env, mg_value_local_time(input_value))));
    case MG_VALUE_TYPE_LOCAL_DATE_TIME:
      return scope.Escape(napi_value(MgLocalDateTimeToNapiDate(
          env, mg_value_local_date_time(input_value))));
    case MG_VALUE_TYPE_DURATION:
      return scope.Escape(napi_value(
          MgDurationToNapiDuration(env, mg_value_duration(input_value))));
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
    case MG_VALUE_TYPE_UNBOUND_RELATIONSHIP: {
      auto unbound_relationship_value = MgUnboundRelationshipToNapiRelationship(
          env, mg_value_unbound_relationship(input_value));
      if (!unbound_relationship_value) {
        return std::nullopt;
      }
      return scope.Escape(napi_value(*unbound_relationship_value));
    }
    case MG_VALUE_TYPE_PATH: {
      auto path_value = MgPathToNapiPath(env, mg_value_path(input_value));
      if (!path_value) {
        return std::nullopt;
      }
      return scope.Escape(napi_value(*path_value));
    }
    case MG_VALUE_TYPE_UNKNOWN:
      NODEMG_THROW(
          "A value of unknown type encountered during query execution.");
      return std::nullopt;
    default:
      NODEMG_THROW("Unrecognized type encountered during query execution.");
      return std::nullopt;
  }
}

std::optional<int64_t> GetInt64Value(Napi::Object input,
                                     const std::string &key) {
  if (!input.Has(key)) return std::nullopt;
  auto key_value = input.Get(key);
  if (!key_value.IsBigInt()) return std::nullopt;
  bool lossless;
  auto value = key_value.As<Napi::BigInt>().Int64Value(&lossless);
  if (!lossless) {
    return std::nullopt;
  }
  return value;
}

std::optional<mg_date *> NapiObjectToMgDate(Napi::Object input) {
  auto maybe_days = GetInt64Value(input, "days");
  if (!maybe_days) return std::nullopt;
  return mg_date_make(*maybe_days);
}

std::optional<mg_local_time *> NapiObjectToMgLocalTime(Napi::Object input) {
  auto maybe_nanoseconds = GetInt64Value(input, "nanoseconds");
  if (!maybe_nanoseconds) return std::nullopt;
  return mg_local_time_make(*maybe_nanoseconds);
}

std::optional<mg_local_date_time *> NapiObjectToMgLocalDateTime(
    Napi::Object input) {
  auto maybe_seconds = GetInt64Value(input, "seconds");
  if (!maybe_seconds) return std::nullopt;
  auto maybe_nanoseconds = GetInt64Value(input, "nanoseconds");
  if (!maybe_nanoseconds) return std::nullopt;
  return mg_local_date_time_make(*maybe_seconds, *maybe_nanoseconds);
}

std::optional<mg_duration *> NapiObjectToMgDuration(Napi::Object input) {
  auto maybe_days = GetInt64Value(input, "days");
  if (!maybe_days) return std::nullopt;
  auto maybe_seconds = GetInt64Value(input, "seconds");
  if (!maybe_seconds) return std::nullopt;
  auto maybe_nanoseconds = GetInt64Value(input, "nanoseconds");
  if (!maybe_nanoseconds) return std::nullopt;
  return mg_duration_make(0, *maybe_days, *maybe_seconds, *maybe_nanoseconds);
}

std::optional<mg_list *> NapiArrayToMgList(Napi::Env env,
                                           Napi::Array input_array) {
  mg_list *output_list = nullptr;
  output_list = mg_list_make_empty(input_array.Length());
  if (!output_list) {
    mg_list_destroy(output_list);
    NODEMG_THROW("Fail to construct Memgraph list.");
    return std::nullopt;
  }
  for (uint32_t index = 0; index < input_array.Length(); ++index) {
    auto maybe_value = NapiValueToMgValue(env, input_array[index]);
    if (!maybe_value) {
      mg_list_destroy(output_list);
      NODEMG_THROW("Fail to construct Memgraph value while constructing list.");
      return std::nullopt;
    }
    if (mg_list_append(output_list, *maybe_value) != 0) {
      mg_list_destroy(output_list);
      NODEMG_THROW("Fail to append Memgraph list value.");
      return std::nullopt;
    }
  }
  return output_list;
}

std::optional<mg_value *> NapiValueToMgValue(Napi::Env env,
                                             Napi::Value input_value) {
  mg_value *output_value = nullptr;
  if (input_value.IsEmpty() || input_value.IsUndefined() ||
      input_value.IsNull()) {
    output_value = mg_value_make_null();
  } else if (input_value.IsBoolean()) {
    output_value = mg_value_make_bool(input_value.As<Napi::Boolean>().Value());
  } else if (input_value.IsBigInt()) {
    bool lossless;
    int64_t as_int64 = input_value.As<Napi::BigInt>().Int64Value(&lossless);
    if (!lossless) {
      NODEMG_THROW("Fail to losslessly convert value to Memgraph int64.");
      return std::nullopt;
    }
    output_value = mg_value_make_integer(as_int64);
  } else if (input_value.IsNumber()) {
    auto as_double = input_value.As<Napi::Number>().DoubleValue();
    output_value = mg_value_make_float(as_double);
  } else if (input_value.IsString()) {
    mg_string *input_mg_string =
        mg_string_make(input_value.As<Napi::String>().Utf8Value().c_str());
    if (!input_mg_string) {
      NODEMG_THROW("Fail to construct Memgraph string.");
      return std::nullopt;
    }
    output_value = mg_value_make_string2(input_mg_string);
  } else if (input_value.IsArray()) {
    auto maybe_mg_list = NapiArrayToMgList(env, input_value.As<Napi::Array>());
    if (!maybe_mg_list) {
      return std::nullopt;
    }
    output_value = mg_value_make_list(*maybe_mg_list);
    // NOTE: The "dispatch" below could be implemented in a much better way
    // (probably the whole function as well), but that's a nice exercise for the
    // future.
  } else if (input_value.IsObject()) {
    auto input_object = input_value.As<Napi::Object>();
    if (input_object.Has("objectType")) {
      auto input_object_type =
          input_object.Get("objectType").As<Napi::String>().Utf8Value();
      if (input_object_type == "date") {
        auto maybe_mg_date = NapiObjectToMgDate(input_object);
        if (!maybe_mg_date) {
          NODEMG_THROW("Converting JS date to Memgraph date failed!");
          return std::nullopt;
        }
        output_value = mg_value_make_date(*maybe_mg_date);
      } else if (input_object_type == "local_time") {
        auto maybe_mg_local_time = NapiObjectToMgLocalTime(input_object);
        if (!maybe_mg_local_time) {
          NODEMG_THROW(
              "Converting JS local time to Memgraph local time failed!");
          return std::nullopt;
        }
        output_value = mg_value_make_local_time(*maybe_mg_local_time);
      } else if (input_object_type == "local_date_time") {
        auto maybe_mg_local_date_time =
            NapiObjectToMgLocalDateTime(input_object);
        if (!maybe_mg_local_date_time) {
          NODEMG_THROW(
              "Converting JS local date time to Memgraph local date time "
              "failed!");
          return std::nullopt;
        }
        output_value = mg_value_make_local_date_time(*maybe_mg_local_date_time);
      } else if (input_object_type == "duration") {
        auto maybe_mg_duration = NapiObjectToMgDuration(input_object);
        if (!maybe_mg_duration) {
          NODEMG_THROW("Converting JS duration to Memgraph duration failed!");
          return std::nullopt;
        }
        output_value = mg_value_make_duration(*maybe_mg_duration);
      } else {
        NODEMG_THROW("Unknown type of JS Object!");
        return std::nullopt;
      }
    }
    auto maybe_mg_map = NapiObjectToMgMap(env, input_object);
    if (!maybe_mg_map) {
      return std::nullopt;
    }
    output_value = mg_value_make_map(*maybe_mg_map);
  } else {
    NODEMG_THROW("Unrecognized JavaScript value.");
    return std::nullopt;
  }
  if (!output_value) {
    NODEMG_THROW("Fail to construct Memgraph value.");
    return std::nullopt;
  }
  return output_value;
}

std::optional<mg_map *> NapiObjectToMgMap(Napi::Env env,
                                          Napi::Object input_object) {
  mg_map *output_map = nullptr;
  auto keys = input_object.GetPropertyNames();
  output_map = mg_map_make_empty(keys.Length());
  if (!output_map) {
    mg_map_destroy(output_map);
    NODEMG_THROW("Fail to construct Memgraph map.");
    return std::nullopt;
  }
  for (uint32_t index = 0; index < keys.Length(); index++) {
    Napi::Value napi_key = keys[index];
    mg_string *mg_key =
        mg_string_make(napi_key.As<Napi::String>().Utf8Value().c_str());
    if (!mg_key) {
      mg_map_destroy(output_map);
      NODEMG_THROW("Fail to constract Memgraph string while creating map.");
      return std::nullopt;
    }
    auto maybe_mg_value = NapiValueToMgValue(env, input_object.Get(napi_key));
    if (!maybe_mg_value) {
      mg_map_destroy(output_map);
      return std::nullopt;
    }
    if (mg_map_insert_unsafe2(output_map, mg_key, *maybe_mg_value) != 0) {
      mg_map_destroy(output_map);
      NODEMG_THROW("Fail to add value to Memgraph map.");
      return std::nullopt;
    }
  }
  return output_map;
}

}  // namespace nodemg
