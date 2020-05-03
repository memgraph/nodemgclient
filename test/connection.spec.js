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
const query = require('./queries');

test('Connect to Memgraph and execute basic queries', () => {
  const connection = memgraph.connect({ host: 'localhost', port: 7687 });
  expect(connection).toBeDefined();
  connection.execute(query.DELETE_ALL);
  connection.execute(query.CREATE_TRIANGLE);
  const nodesNo = connection.execute(query.COUNT_NODES);
  expect(nodesNo[0][0]).toEqual(3);
  const edgesNo = connection.execute(query.COUNT_EDGES);
  expect(edgesNo[0][0]).toEqual(3);
  expect(() => {
    connection.execute('QUERY');
  }).toThrow();
});

test('Create and fetch a node', () => {
  const connection = memgraph.connect({ host: 'localhost', port: 7687 });
  expect(connection).toBeDefined();
  connection.execute(query.DELETE_ALL);
  connection.execute(query.CREATE_RICH_NODE);
  const node = connection.execute(query.NODES)[0][0];
  expect(node.id).toBeGreaterThanOrEqual(0);
  expect(node.labels).toContain('Label1');
  expect(node.labels).toContain('Label2');
  expect(node.properties.prop0).toEqual(undefined);
  expect(node.properties.prop1).toEqual(true);
  expect(node.properties.prop2).toEqual(false);
  expect(node.properties.prop3).toEqual(10);
  expect(node.properties.prop4).toEqual(100.0);
  expect(node.properties.prop5).toEqual('test');
});

test('Create and fetch a relationship', () => {
  const connection = memgraph.connect({ host: 'localhost', port: 7687 });
  expect(connection).toBeDefined();
  connection.execute(query.DELETE_ALL);
  connection.execute(query.CREATE_RICH_EDGE);
  const node = connection.execute(query.EDGES)[0][0];
  expect(node.id).toBeGreaterThanOrEqual(0);
  expect(node.type).toContain('Type');
  expect(node.properties.prop1).toEqual(true);
  expect(node.properties.prop2).toEqual(false);
  expect(node.properties.prop3).toEqual(1);
  expect(node.properties.prop4).toEqual(2.0);
  expect(node.properties.prop5).toEqual('test');
});
