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

const getPort = require('get-port');

const memgraph = require('../lib');
const query = require('./queries');
const util = require('./util');

// TODO(gitbuda): Figure out why sometimes test SEGFAULT.

test('Basic data types', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    const connection = memgraph.connect({ host: 'localhost', port: port });
    expect(connection).toBeDefined();
    const nullValue = connection.execute('RETURN Null;')[0][0];
    expect(nullValue).toEqual(null);
    const listValue = connection.execute('RETURN [1, 2];')[0][0];
    expect(listValue).toEqual([1n, 2n]);
    const mapValue = connection.execute('RETURN {k1: 1, k2: "v"} as d;')[0][0];
    expect(mapValue).toEqual({ k1: 1n, k2: 'v' });
  }, port);
}, 10000);

test('Basic queries', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    const connection = memgraph.connect({ host: 'localhost', port: port });
    expect(connection).toBeDefined();
    connection.execute(query.DELETE_ALL);
    connection.execute(query.CREATE_TRIANGLE);
    const nodesNo = connection.execute(query.COUNT_NODES);
    expect(nodesNo[0][0]).toEqual(3n);
    const edgesNo = connection.execute(query.COUNT_EDGES);
    expect(edgesNo[0][0]).toEqual(3n);
    expect(() => {
      connection.execute('QUERY');
    }).toThrow();
  }, port);
}, 10000);

test('Create and fetch a node', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    const connection = memgraph.connect({ host: 'localhost', port: port });
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
    expect(node.properties.prop3).toEqual(10n);
    expect(node.properties.prop4).toEqual(100.0);
    expect(node.properties.prop5).toEqual('test');
  }, port);
}, 10000);

test('Create and fetch a relationship', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    const connection = memgraph.connect({ host: 'localhost', port: port });
    expect(connection).toBeDefined();
    connection.execute(query.DELETE_ALL);
    connection.execute(query.CREATE_RICH_EDGE);
    const node = connection.execute(query.EDGES)[0][0];
    expect(node.id).toBeGreaterThanOrEqual(0);
    expect(node.type).toContain('Type');
    expect(node.properties.prop1).toEqual(true);
    expect(node.properties.prop2).toEqual(false);
    expect(node.properties.prop3).toEqual(1n);
    expect(node.properties.prop4).toEqual(2.0);
    expect(node.properties.prop5).toEqual('test');
  }, port);
}, 10000);

test('Create and fetch a path', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    const connection = memgraph.connect({ host: 'localhost', port: port });
    expect(connection).toBeDefined();
    connection.execute(query.DELETE_ALL);
    connection.execute(query.CREATE_PATH);
    const nodesNo = connection.execute(query.COUNT_NODES);
    expect(nodesNo[0][0]).toEqual(4n);
    const edgesNo = connection.execute(query.COUNT_EDGES);
    expect(edgesNo[0][0]).toEqual(3n);
    const data = connection.execute(query.MATCH_PATHS);
    expect(data.length).toEqual(3);
    const longestPath = data[2][0];
    expect(longestPath.nodes.length).toEqual(4);
    expect(longestPath.nodes[0]).toEqual(
      expect.objectContaining({
        labels: ['Label'],
        properties: { id: 1n },
      }),
    );
    expect(longestPath.nodes[1]).toEqual(
      expect.objectContaining({
        labels: ['Label'],
        properties: { id: 2n },
      }),
    );
    expect(longestPath.nodes[2]).toEqual(
      expect.objectContaining({
        labels: ['Label'],
        properties: { id: 3n },
      }),
    );
    expect(longestPath.nodes[3]).toEqual(
      expect.objectContaining({
        labels: ['Label'],
        properties: { id: 4n },
      }),
    );
    expect(longestPath.relationships.length).toEqual(3);
    expect(longestPath.relationships[0]).toEqual(
      expect.objectContaining({
        startNodeId: 0n,
        endNodeId: 1n,
        type: 'Type',
        properties: { id: 1n },
      }),
    );
    expect(longestPath.relationships[1]).toEqual(
      expect.objectContaining({
        startNodeId: 1n,
        endNodeId: 2n,
        type: 'Type',
        properties: { id: 2n },
      }),
    );
    expect(longestPath.relationships[2]).toEqual(
      expect.objectContaining({
        startNodeId: 2n,
        endNodeId: 3n,
        type: 'Type',
        properties: { id: 3n },
      }),
    );
  }, port);
}, 10000);

test('Create path and fetch unbound relationship', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    const connection = memgraph.connect({ host: 'localhost', port: port });
    expect(connection).toBeDefined();
    connection.execute(query.DELETE_ALL);
    connection.execute(query.CREATE_PATH);
    const nodesNo = connection.execute(query.COUNT_NODES);
    expect(nodesNo[0][0]).toEqual(4n);
    const edgesNo = connection.execute(query.COUNT_EDGES);
    expect(edgesNo[0][0]).toEqual(3n);
    const data = connection.execute(query.MATCH_RELATIONSHIPS)[0][0];
    expect(data.length).toEqual(1);
    const relationship = data[0];
    expect(relationship).toEqual(
      expect.objectContaining({
        startNodeId: 0n,
        endNodeId: 1n,
        type: 'Type',
        properties: { id: 1n },
      }),
    );
  }, port);
}, 10000);

test('Use query parameters', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    const connection = memgraph.connect({ host: 'localhost', port: port });
    expect(connection).toBeDefined();
    connection.execute(query.DELETE_ALL);
    connection.execute(query.CREATE_NODE_USING_PARAMS, {
      nullProperty: null,
      trueProperty: true,
      falseProperty: false,
      bigIntProperty: 10n,
      numberProperty: 10.5,
      stringProperty: 'string test',
      arrayProperty: ['one', 'two'],
      objectProperty: { one: 'one', two: 'two' },
    });
    const nodesNo = connection.execute(query.COUNT_NODES);
    expect(nodesNo[0][0]).toEqual(1n);
    const node = connection.execute(query.NODES)[0][0];
    expect(node).toEqual(
      expect.objectContaining({
        id: 0n,
        labels: ['Node'],
        properties: {
          trueProperty: true,
          falseProperty: false,
          bigIntProperty: 10n,
          numberProperty: 10.5,
          stringProperty: 'string test',
          arrayProperty: ['one', 'two'],
          objectProperty: { one: 'one', two: 'two' },
        },
      }),
    );
  }, port);
}, 10000);

test('Query parameters not provided', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    const connection = memgraph.connect({ host: 'localhost', port: port });
    expect(connection).toBeDefined();
    expect(() => {
      connection.execute(query.CREATE_NODE_USING_PARAMS);
    }).toThrow();
  }, port);
}, 10000);
