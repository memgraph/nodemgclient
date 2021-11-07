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

const Docker = require('dockerode');
const assert = require('assert');

const docker = new Docker({ socketPath: '/var/run/docker.sock' });

async function checkAgainstMemgraph(check, port = 7687, sslEnabled = false) {
  const cmd = [
    '--telemetry-enabled=False',
    '--log-level=TRACE',
    '--also-log-to-stderr',
  ];
  if (sslEnabled) {
    cmd.push(
      '--bolt-key-file=/etc/memgraph/ssl/key.pem',
      '--bolt-cert-file=/etc/memgraph/ssl/cert.pem',
    );
  }
  const container = await docker.createContainer({
    Image: 'memgraph:2.0.1',
    Tty: false,
    AttachStdin: false,
    AttachStdout: false,
    AttachStderr: false,
    OpenStdin: false,
    StdinOnce: false,
    Cmd: cmd,
    HostConfig: {
      AutoRemove: true,
      PortBindings: {
        '7687/tcp': [{ HostPort: port.toString() }],
      },
    },
  });
  await container.start();
  // Waiting is not completly trivial because TCP connections is live while
  // Memgraph is still not up and running.
  // TODO(gitbuda): Replace wait with the client connection attempts.
  await new Promise((resolve) => setTimeout(resolve, 2000));
  try {
    await check();
    // eslint-disable-next-line no-useless-catch
  } catch (err) {
    throw err;
  } finally {
    await container.remove({ force: true });
  }
}

function firstRecord(result) {
  assert(!!result && typeof result === 'object', 'Result has to be Object');
  const data = result[0];
  assert(!!data && Array.isArray(data), 'Data has to be Array');
  assert(data.length > 0);
  return data[0];
}

module.exports = {
  checkAgainstMemgraph,
  firstRecord,
};
