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

constexpr char NODEMG_MSG_WRONG_CONN_ARG[] =
    "Wrong connection argument. One JavaScript object containing connection "
    "parameters is allowed.";
constexpr char NODEMG_MSG_CONN_PARAMS_ALLOC_FAIL[] =
    "Failed to allocate CONNECTION Parameters.";
constexpr char NODEMG_MSG_CONN_FAIL[] = "Connection failed.";
constexpr char NODEMG_MSG_UNKNOWN_TYPE_ERROR[] =
    "A value of unknown type encountered.";
constexpr char NODEMG_MSG_UNRECOGNIZED_TYPE_ERROR[] =
    "Unrecognized type encountered.";
constexpr char NODEMG_MSG_WRONG_EXECUTE_ARGS[] =
    "Wrong execute arguments. The first argument has to be query. Second "
    "argument is optional and should contain the query parameters "
    "object/dictionary type of object.";
constexpr char NODEMG_MSG_QUERY_STRING_ERROR[] =
    "The first execute argument has to be string.";
constexpr char NODEMG_MSG_QUERY_PARAMS_ERROR[] =
    "The second execute argument has to be dictionary/object.";
constexpr char NODEMG_MSG_RUN_FAIL[] = "Fail to execute query (run command).";
constexpr char NODEMG_MSG_HOST_ADDR_MISSING[] =
    "Exactly one of host and address parameters must be specified.";
constexpr char NODEMG_MSG_PORT_OUT_OF_RANGE[] = "Port out of range.";
constexpr char NODEMG_MSG_TRUST_CALLBACK_IS_NOT_FUNCTION[] =
    "Trust callback isn't callable.";
constexpr char NODEMG_MSG_QUERY_PARAMS_ALLOC_FAIL[] =
    "Failed to allocate QUERY parameters.";
constexpr char NODEMG_MSG_UNABLE_TO_LOSSLESSLY_CONVERT_INT[] =
    "Unable to losslessly convert value to int64.";
constexpr char NODEMG_MSG_UNABLE_TO_CONSTRUCT_MG_VALUE[] =
    "Unable to construct Memgraph value.";
constexpr char NODEMG_MSG_UNABLE_TO_CONSTRUCT_STRING[] =
    "Unable to construct Memgraph string.";
constexpr char NODEMG_MSG_UNABLE_TO_CONSTRUCT_VALUE[] =
    "Unable to construct Memgraph value.";
constexpr char NODEMG_MSG_UNABLE_TO_ADD_TO_MAP[] =
    "Unable to add value to Memgraph map";
constexpr char NODEMG_MSG_UNABLE_TO_CONSTRUCT_MG_MAP[] =
    "Unable to construct Memgraph map";
constexpr char NODEMG_MSG_UNABLE_TO_CONSTRUCT_MG_LIST[] =
    "Unable to construct Memgraph list";
constexpr char NODEMG_MSG_UNABLE_TO_APPEND_MG_LIST_VALUE[] =
    "Unable to append Memgraph list value";
