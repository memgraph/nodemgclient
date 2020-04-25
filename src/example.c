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

#include <assert.h>

#include <node_api.h>

#include <mgclient.h>

#define NAPI_CHECK_STATUS                        \
  if (status != napi_ok) {                       \
    napi_throw_error(env, "N-API call failed."); \
    return;                                      \
  }

static napi_value CountNodes(napi_env env, napi_callback_info info) {
  napi_status node_status;
  napi_value count;

  mg_session_params *params = mg_session_params_make();
  if (!params) {
    napi_throw_error(env, NULL, "Failed to allocate parameters.");
    return NULL;
  }
  mg_session_params_set_host(params, "127.0.0.1");
  mg_session_params_set_port(params, 7687);
  mg_session_params_set_sslmode(params, MG_SSLMODE_REQUIRE);

  mg_session *session = NULL;
  int mg_status = mg_connect(params, &session);
  mg_session_params_destroy(params);
  if (mg_status < 0) {
    mg_session_destroy(session);
    napi_throw_error(env, NULL, "Failed to connect.");
    return NULL;
  }

  if (mg_session_run(session, "MATCH (n) RETURN count(n) AS cnt", NULL, NULL) <
      0) {
    mg_session_destroy(session);
    napi_throw_error(env, NULL, "Failed to execute query.");
    return NULL;
  }
  mg_result *result;
  mg_status = mg_session_pull(session, &result);
  if (mg_status < 0) {
    mg_session_destroy(session);
    napi_throw_error(env, NULL, "Failed to get the count");
    return NULL;
  }
  node_status = napi_create_int64(
      env, mg_value_integer(mg_list_at(mg_result_row(result), 0)), &count);
  mg_session_destroy(session);
  assert(status == napi_ok);
  return count;
}

#define DECLARE_NAPI_METHOD(name, func) \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

static napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor desc = DECLARE_NAPI_METHOD("count", CountNodes);
  status = napi_define_properties(env, exports, 1, &desc);
  assert(status == napi_ok);
  return exports;
}

NAPI_MODULE(nodemgclient, Init)
