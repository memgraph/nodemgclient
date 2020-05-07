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
    "Failed to allocate connection parameters.";
constexpr char NODEMG_MSG_CONN_FAIL[] = "Connection failed.";
constexpr char NODEMG_MSG_TYPE_ERROR[] = "A value of unknown type encountered.";
constexpr char NODEMG_MSG_WRONG_EXECUTE_ARG[] = "Wrong execute arguments.";
constexpr char NODEMG_MSG_RUN_FAIL[] = "Fail to execute query (run command).";
constexpr char NODEMG_MSG_HOST_ADDR_MISSING[] =
    "Exactly one of host and address parameters must be specified.";
constexpr char NODEMG_MSG_PORT_OUT_OF_RANGE[] = "Port out of range.";
