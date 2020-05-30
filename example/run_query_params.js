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

const connection = memgraph.Connect({ host: 'localhost', port: 7687 });

connection
  .Execute('CREATE (n:Node {name: $name});', { name: 'John Swan' })
  .Records();
console.log(
  connection.Execute('MATCH (n) RETURN n;').Records()[0].Values()[0].properties,
);