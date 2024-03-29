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

const memgraph = require('..');
const query = require('../test/queries');

(async () => {
  try {
    const connection = await memgraph.Connect({
      host: 'localhost',
      port: 7687,
    });

    await connection.ExecuteAndFetchAll(query.DELETE_ALL);

    const result = await connection.ExecuteAndFetchAll(
      `RETURN "value_x2" AS x, "value_y2" AS y;`,
    );
    console.log(result);

    await connection.ExecuteAndFetchAll('FAIL');
  } catch (e) {
    console.log(e);
  }
})();
