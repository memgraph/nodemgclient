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
const util = require('./util');

test('Connect empty args should work (port is always different)', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    const connection = await memgraph.Client().Connect({ port: port });
    expect(connection).toBeDefined();
  }, port);
});

test('Connect fail because port spelling is wrong', () => {
  expect(() => {
    memgraph.Client().Connect({ prt: 7687 });
  }).toThrow();
});

test('Connect fail because port is out of range', () => {
  expect(() => {
    memgraph.Client().Connect({ port: -100 });
  }).toThrow();
  expect(() => {
    memgraph.Client().Connect({ port: 1000000 });
  }).toThrow();
});

test('Connect to Memgraph host via SSL multiple times', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(
    async () => {
      for (let iter = 0; iter < 100; iter++) {
        const connection = await memgraph.Client().Connect({
          host: '127.0.0.1',
          port: port,
          use_ssl: true,
        });
        expect(connection).toBeDefined();
      }
    },
    port,
    true,
  );
});

test('Connect fail because host is wrong', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(async () => {
    await expect(
      memgraph.Client().Connect({ host: 'wrong_host' }),
    ).rejects.toThrow();
  }, port);
});
