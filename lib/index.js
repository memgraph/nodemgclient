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

const _addon = require('bindings')('nodemgclient');

// TODO(gitbuda): Design the correct library interface.
//     * _addon.Connection should also return a promise.
//     * Add EventEmitter.
//     * Add async and ensure that the interface is right.

class Connection {
  constructor(params) {
    this.connection_ = new _addon.Sync.Connection(params);
  }

  Cursor() {
    return this.connection_.Cursor();
  }

  async ExecuteLazy(query, params = {}) {
    const cursor = this.connection_.Cursor();
    await cursor.Execute(query, params, true);
    return cursor;
  }

  Execute(query, params = {}) {
    const cursor = this.connection_.Cursor();
    return new Promise(function (resolve, reject) {
      cursor
        .Execute(query, params)
        .then((result) => {
          resolve(result);
        })
        .catch((error) => {
          reject(error);
        });
    });
  }

  async ExecuteAndFetchRecords(query, params = {}) {
    const cursor = this.connection_.Cursor();
    const data = await cursor.Execute(query, params);
    return data;
  }
}

class AsyncConnection {
  constructor() {
    this.connection_ = new _addon.Async.Connection();
  }

  Connect(params) {
    return this.connection_.Connect(params);
  }

  Execute(query) {
    this.connection_.Execute(query);
  }
}

module.exports = {
  Connect: (params) => {
    return new Connection(params);
  },
  AsyncConnection: () => {
    return new AsyncConnection();
  },
};
