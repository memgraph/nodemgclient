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

const memgraph = require('../lib');
const query = require('../test/queries');

const connection = memgraph.connect({
  host: 'localhost',
  port: 7687,
  username: 'admin',
  password: 'admin',
  trust_callback: (hostname, ip_address, key_type, fingerprint) => {
    console.log(hostname);
    console.log(ip_address);
    console.log(key_type);
    console.log(fingerprint);
    // TODO(gitbuda): Move this part to the documentation.
    // console.log(xyz); -> Works fine.
    // throw Error("error"); -> Works fine.
    // throw 10 -> FATAL ERROR; NOTE: Napi can't handle everything.
    return true;
  },
});

const nodesNo = connection.execute(query.COUNT_NODES);
const edgesNo = connection.execute(query.COUNT_EDGES);

console.log('Number of Nodes: ' + nodesNo[0][0]);
console.log('Number of Edges: ' + edgesNo[0][0]);
