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

import * as Memgraph from '../..';

(async () => {
  try {
    const connection:Memgraph.Connection = await Memgraph.Connect({ host: 'localhost', port: 7687 });
    console.log(await connection.ExecuteAndFetchAll("MATCH (n) RETURN n LIMIT 1;"));
  } catch (e) {
    console.log(e);
  }
})();
