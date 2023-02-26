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

// TODO(gitbuda): Port EventEmitter code to async.

const memgraph = require('..');
const query = require('../test/queries');
const EventEmitter = require('events').EventEmitter;

const emitter = new EventEmitter()
  .on('start', () => {
    console.log('### START ###');
  })
  .on('record', (record) => {
    console.log(record.Values());
  })
  .on('end', (summary) => {
    console.log(summary);
    console.log('### END ###');
  });

(async () => {
  try {
    // TODO(gitbuda): Design the correct interface. Connect should also return
    // a promise.
    const connection = memgraph.Connect({ host: 'localhost', port: 7687 });

    await connection.ExecuteAndFetchAll(query.DELETE_ALL);

    const result = await connection.ExecuteLazy(
      `UNWIND [0, 1] AS item RETURN "value_x2" AS x, "value_y2" AS y;`,
    );
    console.log(result);
    // TODO(gitbuda): Figure out how to hide the bind call from a user.
    result.Stream(emitter.emit.bind(emitter));
  } catch (e) {
    console.log(e);
  }
})();
