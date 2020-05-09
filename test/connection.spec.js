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

// TODO(gitbuda): Add utils to mamage Memgraph server and add all tests.

test('Connect to Memgraph host via SSL', () => {
  for (let iter = 0; iter < 100; iter++) {
    const connection = memgraph.connect({
      host: 'localhost',
      port: 7687,
      use_ssl: true,
    });
    expect(connection).toBeDefined();
  }
});

test('Connect to Memgraph address via SSL', () => {
  const connection = memgraph.connect({
    address: '127.0.0.1',
    port: 7687,
    use_ssl: true,
  });
  expect(connection).toBeDefined();
});

test('Fail because trust_callback is not callable', () => {
  expect(() => {
    memgraph.connect({
      host: 'localhost',
      port: 7687,
      use_ssl: true,
      trust_callback: 'Not callable.',
    });
  }).toThrow();
});

test('Fail because trust_callback returns false', () => {
  expect(() => {
    memgraph.connect({
      host: 'localhost',
      port: 7687,
      use_ssl: true,
      trust_callback: () => {
        return false;
      },
    });
  }).toThrow();
});

test('Connect trust_callback when returns true', () => {
  const connection = memgraph.connect({
    address: '127.0.0.1',
    port: 7687,
    use_ssl: true,
    trust_callback: () => {
      return true;
    },
  });
  expect(connection).toBeDefined();
});

test('Connect and check that trust callback received all arguments', () => {
  const connection = memgraph.connect({
    address: '127.0.0.1',
    port: 7687,
    use_ssl: true,
    trust_callback: () => {
      return true;
    },
  });
  expect(connection).toBeDefined();
});
