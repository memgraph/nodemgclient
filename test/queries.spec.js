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

const memgraph = require('..');
const query = require('./queries');
const util = require('./util');

test('Queries data types', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = await memgraph.Connect({
      host: '127.0.0.1',
      port: port,
    });
    expect(connection).toBeDefined();

    const nullValue = util.firstRecord(
      await connection.ExecuteAndFetchAll('RETURN Null;'),
    );
    expect(nullValue).toEqual(null);

    const listValue = util.firstRecord(
      await connection.ExecuteAndFetchAll('RETURN [1, 2];'),
    );
    expect(listValue).toEqual([1n, 2n]);

    const mapValue = util.firstRecord(
      await connection.ExecuteAndFetchAll('RETURN {k1: 1, k2: "v"} as d;'),
    );
    expect(mapValue).toEqual({ k1: 1n, k2: 'v' });

    const temporalValues = await connection.ExecuteAndFetchAll(
      query.TEMPORAL_VALUES,
    );
    expect(temporalValues[0][0]).toEqual({
      objectType: 'date',
      days: -3642n,
      date: new Date('1960-01-12T00:00:00.000Z'),
    });
    expect(temporalValues[0][1]).toEqual({
      objectType: 'local_time',
      nanoseconds: 36548123456000n,
    });
    expect(temporalValues[0][2]).toEqual({
      objectType: 'local_date_time',
      seconds: 1632988862n,
      nanoseconds: 0n,
      date: new Date('2021-09-30T08:01:02.000Z'),
    });
    expect(temporalValues[0][3]).toEqual({
      objectType: 'duration',
      days: 1n,
      seconds: 7384n,
      nanoseconds: 560000000n,
    });
  }, port);
}, 10000);

test('Queries basic graph', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = await memgraph.Connect({
      host: '127.0.0.1',
      port: port,
    });
    expect(connection).toBeDefined();

    await connection.ExecuteAndFetchAll(query.DELETE_ALL);
    await connection.ExecuteAndFetchAll(query.CREATE_TRIANGLE);

    const nodesNo = util.firstRecord(
      await connection.ExecuteAndFetchAll(query.COUNT_NODES),
    );
    expect(nodesNo).toEqual(3n);

    const edgesNo = util.firstRecord(
      await connection.ExecuteAndFetchAll(query.COUNT_EDGES),
    );
    expect(edgesNo).toEqual(3n);
    await expect(connection.Execute('QUERY')).rejects.toThrow();
  }, port);
}, 10000);

test('Queries create and fetch a node', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = await memgraph.Connect({
      host: '127.0.0.1',
      port: port,
    });
    expect(connection).toBeDefined();

    await connection.ExecuteAndFetchAll(query.DELETE_ALL);
    await connection.ExecuteAndFetchAll(query.CREATE_RICH_NODE);

    const node = util.firstRecord(
      await connection.ExecuteAndFetchAll(query.NODES),
    );
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

test('Queries create and fetch a relationship', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = await memgraph.Connect({
      host: '127.0.0.1',
      port: port,
    });
    expect(connection).toBeDefined();

    await connection.ExecuteAndFetchAll(query.DELETE_ALL);
    await connection.ExecuteAndFetchAll(query.CREATE_RICH_EDGE);

    const node = util.firstRecord(
      await connection.ExecuteAndFetchAll(query.EDGES),
    );
    expect(node.id).toBeGreaterThanOrEqual(0);
    expect(node.edgeType).toContain('Type');
    expect(node.properties.prop1).toEqual(true);
    expect(node.properties.prop2).toEqual(false);
    expect(node.properties.prop3).toEqual(1n);
    expect(node.properties.prop4).toEqual(2.0);
    expect(node.properties.prop5).toEqual('test');
  }, port);
}, 10000);

test('Queries create and fetch a path', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = await memgraph.Connect({
      host: '127.0.0.1',
      port: port,
    });
    expect(connection).toBeDefined();

    await connection.ExecuteAndFetchAll(query.DELETE_ALL);
    await connection.ExecuteAndFetchAll(query.CREATE_PATH);

    const nodesNo = util.firstRecord(
      await connection.ExecuteAndFetchAll(query.COUNT_NODES),
    );
    expect(nodesNo).toEqual(4n);
    const edgesNo = util.firstRecord(
      await connection.ExecuteAndFetchAll(query.COUNT_EDGES),
    );
    expect(edgesNo).toEqual(3n);
    const data = await connection.ExecuteAndFetchAll(query.MATCH_PATHS);
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
        edgeType: 'Type',
        properties: { id: 1n },
      }),
    );
    expect(longestPath.relationships[1]).toEqual(
      expect.objectContaining({
        startNodeId: 1n,
        endNodeId: 2n,
        edgeType: 'Type',
        properties: { id: 2n },
      }),
    );
    expect(longestPath.relationships[2]).toEqual(
      expect.objectContaining({
        startNodeId: 2n,
        endNodeId: 3n,
        edgeType: 'Type',
        properties: { id: 3n },
      }),
    );
  }, port);
}, 10000);

test('Queries create path and fetch unbound relationship', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = await memgraph.Connect({
      host: '127.0.0.1',
      port: port,
    });
    expect(connection).toBeDefined();

    await connection.ExecuteAndFetchAll(query.DELETE_ALL);
    await connection.ExecuteAndFetchAll(query.CREATE_PATH);

    const nodesNo = util.firstRecord(
      await connection.ExecuteAndFetchAll(query.COUNT_NODES),
    );
    expect(nodesNo).toEqual(4n);
    const edgesNo = util.firstRecord(
      await connection.ExecuteAndFetchAll(query.COUNT_EDGES),
    );
    expect(edgesNo).toEqual(3n);
    const result = await connection.ExecuteAndFetchAll(
      query.MATCH_RELATIONSHIPS,
    );
    expect(result.length).toEqual(1);
    const relationship = util.firstRecord(result)[0];
    expect(relationship).toEqual(
      expect.objectContaining({
        startNodeId: 0n,
        endNodeId: 1n,
        edgeType: 'Type',
        properties: { id: 1n },
      }),
    );
  }, port);
}, 10000);

test('Queries use query parameters', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = await memgraph.Connect({
      host: '127.0.0.1',
      port: port,
    });
    expect(connection).toBeDefined();
    const dateProperty = memgraph.createMgDate(-3642n);
    const localTimeProperty = memgraph.createMgLocalTime(36548123456000n);
    const localDateTimeProperty = memgraph.createMgLocalDateTime(
      1632988862n,
      0n,
    );
    const durationProperty = memgraph.createMgDuration(1n, 7384n, 560000000n);
    await connection.ExecuteAndFetchAll(query.DELETE_ALL);
    await connection.ExecuteAndFetchAll(query.CREATE_NODE_USING_PARAMS, {
      nullProperty: null,
      trueProperty: true,
      falseProperty: false,
      bigIntProperty: 10n,
      numberProperty: 10.5,
      stringProperty: 'string test',
      arrayProperty: ['one', 'two'],
      objectProperty: { one: 'one', two: 'two' },
      dateProperty: dateProperty,
      localTimeProperty: localTimeProperty,
      localDateTimeProperty: localDateTimeProperty,
      durationProperty: durationProperty,
    });
    const nodesNo = util.firstRecord(
      await connection.ExecuteAndFetchAll(query.COUNT_NODES),
    );
    expect(nodesNo).toEqual(1n);
    const node = util.firstRecord(
      await connection.ExecuteAndFetchAll(query.NODES),
    );
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
          dateProperty: {
            objectType: 'date',
            days: -3642n,
            date: new Date('1960-01-12T00:00:00.000Z'),
          },
          localTimeProperty: localTimeProperty,
          localDateTimeProperty: {
            objectType: 'local_date_time',
            seconds: 1632988862n,
            nanoseconds: 0n,
            date: new Date('2021-09-30T08:01:02.000Z'),
          },
          durationProperty: durationProperty,
        },
      }),
    );
  }, port);
}, 10000);

test('Queries query parameters not provided', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = await memgraph.Connect({
      host: '127.0.0.1',
      port: port,
    });
    expect(connection).toBeDefined();

    await expect(
      connection.Execute(query.CREATE_NODE_USING_PARAMS),
    ).rejects.toThrow();
  }, port);
}, 10000);
