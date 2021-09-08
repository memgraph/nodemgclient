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

const Bindings = require('bindings')('nodemgclient');
const pjson = require('./package.json');

// This class exists becuase of additional logic that is easier to implement in
// JavaScript + to extend the implementation with easy to use primitives.
class Connection {
  constructor(client) {
    this.client = client;
  }

  async Execute(query, params={}) {
    return await this.client.Execute(query, params);
  }

  async FetchAll() {
    return await this.client.FetchAll();
  }

  async DiscardAll() {
    return await this.client.DiscardAll();
  }

  async Begin() {
    return await this.client.Begin();
  }

  async Commit() {
    return await this.client.Commit();
  }

  async Rollback() {
    return await this.client.Rollback();
  }

  async ExecuteAndFetchAll(query, params={}) {
    await this.client.Execute(query, params);
    return await this.client.FetchAll();
  }
}

const Memgraph = {
  Client: () => {
    return new Bindings.Client("nodemgclient/" + pjson.version);
  },
  Connect: async (params) => {
    let client = new Bindings.Client("nodemgclient/" + pjson.version);
    // TODO(gitbuda): If the second client is not passed, execution blocks, check why.
    client = await client.Connect(params);
    return new Connection(client);
  }
}

module.exports = Memgraph;
