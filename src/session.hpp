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

// TODO(gitbuda): Encapsulates mg_session* and returns pure data (no Napi
// dependency) so that it could be used in the async context. Something
// equivalent to mg::Client has to used because value has to be stored
// somewhere because mg_session_pull reuses the record memory.

class Session {
 public:
  Session();
};
