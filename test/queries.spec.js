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

test('Basic data types', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = memgraph.Connect({ address: '127.0.0.1', port: port });
    expect(connection).toBeDefined();
    const nullValue = util
      .firstRecord(await connection.ExecuteAndFetchRecords('RETURN Null;'))
      .Values()[0];
    expect(nullValue).toEqual(null);
    const listValue = util
      .firstRecord(await connection.ExecuteAndFetchRecords('RETURN [1, 2];'))
      .Values()[0];
    expect(listValue).toEqual([1n, 2n]);
    const mapValue = (
      await connection.ExecuteAndFetchRecords('RETURN {k1: 1, k2: "v"} as d;')
    )['data'][0].Values()[0];
    expect(mapValue).toEqual({ k1: 1n, k2: 'v' });
  }, port);
}, 10000);

test('Basic queries', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = memgraph.Connect({ address: '127.0.0.1', port: port });
    expect(connection).toBeDefined();
    await connection.ExecuteAndFetchRecords(query.DELETE_ALL);
    await connection.ExecuteAndFetchRecords(query.CREATE_TRIANGLE);
    const nodesNo = util.firstRecord(
      await connection.ExecuteAndFetchRecords(query.COUNT_NODES),
    );
    expect(nodesNo.Values()[0]).toEqual(3n);
    const edgesNo = util.firstRecord(
      await connection.ExecuteAndFetchRecords(query.COUNT_EDGES),
    );
    expect(edgesNo.Values()[0]).toEqual(3n);
    await expect(connection.Execute('QUERY')).rejects.toThrow();
  }, port);
}, 10000);

test('Create and fetch a node', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = memgraph.Connect({ address: '127.0.0.1', port: port });
    expect(connection).toBeDefined();
    await connection.ExecuteAndFetchRecords(query.DELETE_ALL);
    await connection.ExecuteAndFetchRecords(query.CREATE_RICH_NODE);
    const node = util
      .firstRecord(await connection.ExecuteAndFetchRecords(query.NODES))
      .Values()[0];
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
  await util.checkAgainstMemgraph(async () => {
    const connection = memgraph.Connect({ address: '127.0.0.1', port: port });
    expect(connection).toBeDefined();
    await connection.ExecuteAndFetchRecords(query.DELETE_ALL);
    await connection.ExecuteAndFetchRecords(query.CREATE_RICH_EDGE);
    const node = util
      .firstRecord(await connection.ExecuteAndFetchRecords(query.EDGES))
      .Values()[0];
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
  await util.checkAgainstMemgraph(async () => {
    const connection = memgraph.Connect({ address: '127.0.0.1', port: port });
    expect(connection).toBeDefined();
    await connection.ExecuteAndFetchRecords(query.DELETE_ALL);
    await connection.ExecuteAndFetchRecords(query.CREATE_PATH);
    const nodesNo = util.firstRecord(
      await connection.ExecuteAndFetchRecords(query.COUNT_NODES),
    );
    expect(nodesNo.Values()[0]).toEqual(4n);
    const edgesNo = util.firstRecord(
      await connection.ExecuteAndFetchRecords(query.COUNT_EDGES),
    );
    expect(edgesNo.Values()[0]).toEqual(3n);
    const data = await connection.ExecuteAndFetchRecords(query.MATCH_PATHS);
    expect(data['data'].length).toEqual(3);
    const longestPath = data['data'][2].Values()[0];
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
  await util.checkAgainstMemgraph(async () => {
    const connection = memgraph.Connect({ address: '127.0.0.1', port: port });
    expect(connection).toBeDefined();
    await connection.ExecuteAndFetchRecords(query.DELETE_ALL);
    await connection.ExecuteAndFetchRecords(query.CREATE_PATH);
    const nodesNo = util.firstRecord(
      await connection.ExecuteAndFetchRecords(query.COUNT_NODES),
    );
    expect(nodesNo.Values()[0]).toEqual(4n);
    const edgesNo = util.firstRecord(
      await connection.ExecuteAndFetchRecords(query.COUNT_EDGES),
    );
    expect(edgesNo.Values()[0]).toEqual(3n);
    const result = await connection.ExecuteAndFetchRecords(
      query.MATCH_RELATIONSHIPS,
    );
    expect(result['data'].length).toEqual(1);
    const relationship = util.firstRecord(result).Values()[0][0];
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
  await util.checkAgainstMemgraph(async () => {
    const connection = memgraph.Connect({ address: '127.0.0.1', port: port });
    expect(connection).toBeDefined();
    await connection.ExecuteAndFetchRecords(query.DELETE_ALL);
    await connection.ExecuteAndFetchRecords(query.CREATE_NODE_USING_PARAMS, {
      nullProperty: null,
      trueProperty: true,
      falseProperty: false,
      bigIntProperty: 10n,
      numberProperty: 10.5,
      stringProperty: 'string test',
      arrayProperty: ['one', 'two'],
      objectProperty: { one: 'one', two: 'two' },
    });
    const nodesNo = util.firstRecord(
      await connection.ExecuteAndFetchRecords(query.COUNT_NODES),
    );
    expect(nodesNo.Values()[0]).toEqual(1n);
    const node = util
      .firstRecord(await connection.ExecuteAndFetchRecords(query.NODES))
      .Values()[0];
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
  await util.checkAgainstMemgraph(async () => {
    const connection = memgraph.Connect({ address: '127.0.0.1', port: port });
    expect(connection).toBeDefined();
    await expect(
      connection.Execute(query.CREATE_NODE_USING_PARAMS),
    ).rejects.toThrow();
  }, port);
}, 10000);

test('Result columns', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = memgraph.Connect({ address: '127.0.0.1', port: port });
    expect(connection).toBeDefined();
    await connection.ExecuteAndFetchRecords(query.DELETE_ALL);
    const cursor = connection.Cursor();
    const result = await cursor.Execute(
      `RETURN "value_x" AS x, "value_y" AS y;`,
    );
    expect(cursor.Columns()).toEqual(['x', 'y']);
    const record = util.firstRecord(result);
    expect(record.Values()).toEqual(['value_x', 'value_y']);
    expect(record.Get('x')).toEqual('value_x');
    expect(record.Get('y')).toEqual('value_y');
    expect(() => {
      record.Get();
    }).toThrow();
    expect(() => {
      record.Get(null);
    }).toThrow();
  }, port);
}, 10000);
