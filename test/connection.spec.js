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
const util = require('./util');

// TODO(gitbuda): Add all connection params test.

test('Fail because connection params are not there', () => {
  expect(() => {
    memgraph.Connect();
  }).toThrow();
});

test('Fail because both host and address are missing', () => {
  expect(() => {
    memgraph.Connect({ port: 7687, use_ssl: true });
  }).toThrow();
});

test('Fail because port number is out of range', () => {
  expect(() => {
    memgraph.Connect({ host: 'localhost', port: -100, use_ssl: true });
  }).toThrow();
  expect(() => {
    memgraph.Connect({ host: 'localhost', port: 10000, use_ssl: true });
  }).toThrow();
});

test('Connect to Memgraph host via SSL multiple times', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    for (let iter = 0; iter < 100; iter++) {
      const connection = memgraph.Connect({
        host: 'localhost',
        port: port,
        use_ssl: true,
      });
      expect(connection).toBeDefined();
    }
  }, port);
});

test('Connect to Memgraph address via SSL', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    const connection = memgraph.Connect({
      address: '127.0.0.1',
      port: port,
      use_ssl: true,
    });
    expect(connection).toBeDefined();
  }, port);
});

test('Fail because trust_callback is not callable', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    expect(() => {
      memgraph.Connect({
        host: 'localhost',
        port: port,
        use_ssl: true,
        trust_callback: 'Not callable.',
      });
    }).toThrow();
  }, port);
});

test('Fail because trust_callback returns false', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    expect(() => {
      memgraph.Connect({
        host: 'localhost',
        port: port,
        use_ssl: true,
        trust_callback: () => {
          return false;
        },
      });
    }).toThrow();
  }, port);
});

test('trust_callback when returns true', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    const connection = memgraph.Connect({
      address: '127.0.0.1',
      port: port,
      use_ssl: true,
      trust_callback: () => {
        return true;
      },
    });
    expect(connection).toBeDefined();
  }, port);
});

test('trust callback received all arguments', async () => {
  const port = await getPort();
  await util.checkAgainstMemgraph(() => {
    const connection = memgraph.Connect({
      address: '127.0.0.1',
      port: port,
      use_ssl: true,
      trust_callback: () => {
        return true;
      },
    });
    expect(connection).toBeDefined();
  }, port);
});
