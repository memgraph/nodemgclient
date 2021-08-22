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

// CONN - Errors related to the connect.
constexpr char NODEMG_MSG_CONN_WRONG_ARG[] =
    "Wrong connection argument. One JavaScript object containing connection "
    "parameters is allowed.";
constexpr char NODEMG_MSG_CONN_PARAMS_ALLOC_FAIL[] =
    "Failed to allocate CONNECTION parameters.";
constexpr char NODEMG_MSG_CONN_FAIL[] = "Connection failed.";
constexpr char NODEMG_MSG_CONN_HOST_ADDR_MISSING[] =
    "Exactly one of host and address parameters must be specified.";
constexpr char NODEMG_MSG_CONN_PORT_OUT_OF_RANGE[] = "Port out of range.";
constexpr char NODEMG_MSG_CONN_TRUST_CALLBACK_IS_NOT_FUNCTION[] =
    "Trust callback isn't callable.";

// EXEC - Errors related to the execute.
constexpr char NODEMG_MSG_EXEC_WRONG_ARGS[] =
    "Wrong execute arguments. The first argument has to be query. Second "
    "argument is optional and should contain the query parameters "
    "object/dictionary.";
constexpr char NODEMG_MSG_EXEC_QUERY_STRING_ERROR[] =
    "The first execute argument has to be string.";
constexpr char NODEMG_MSG_EXEC_QUERY_PARAMS_ERROR[] =
    "The second execute argument has to be dictionary/object.";
constexpr char NODEMG_MSG_EXEC_CONSTRUCT_QUERY_PARAMS_FAIL[] =
    "Failed to construct QUERY parameters.";
constexpr char NODEMG_MSG_EXEC_FAIL[] = "Fail to execute query (run command).";
// - Errors from server to client.
constexpr char NODEMG_MSG_EXEC_UNKNOWN_TYPE_ERROR[] =
    "A value of unknown type encountered during query execution.";
constexpr char NODEMG_MSG_EXEC_UNRECOGNIZED_TYPE_ERROR[] =
    "Unrecognized type encountered during query execution.";
// - Errors from converting client arguments before sending them to server.
constexpr char NODEMG_MSG_EXEC_UNABLE_TO_LOSSLESSLY_CONVERT_INT[] =
    "Unable to losslessly convert value to Memgraph int64.";
constexpr char NODEMG_MSG_EXEC_UNABLE_TO_CONSTRUCT_MG_VALUE[] =
    "Unable to construct Memgraph value.";
constexpr char NODEMG_MSG_EXEC_UNRECOGNIZED_JS_VALUE[] =
    "Unrecognized JavaScript value.";
constexpr char NODEMG_MSG_EXEC_UNABLE_TO_CONSTRUCT_MG_STRING[] =
    "Unable to construct Memgraph string.";
constexpr char NODEMG_MSG_EXEC_UNABLE_TO_ADD_TO_MG_MAP[] =
    "Unable to add value to Memgraph map";
constexpr char NODEMG_MSG_EXEC_UNABLE_TO_CONSTRUCT_MG_MAP[] =
    "Unable to construct Memgraph map";
constexpr char NODEMG_MSG_EXEC_UNABLE_TO_CONSTRUCT_MG_LIST[] =
    "Unable to construct Memgraph list";
constexpr char NODEMG_MSG_EXEC_UNABLE_TO_APPEND_VALUE_TO_MG_LIST[] =
    "Unable to append Memgraph list value";
