// Copyright (c) 2016-2021 Memgraph Ltd. [https://memgraph.com]
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

// TODO(gitbuda): Make tests out of this async example.

const memgraph = require('..');

(async () => {
  try {
    const client = memgraph.Client();
    client
      .Connect({ host: 'localhost' })
      .then(async (connection) => {
        console.log('Connected!');
        connection
          .Execute('MATCH (n) WHERE n.name = $name RETURN n, n.name;', {
            name: 'TEST',
          })
          .then(async (_) => {
            try {
              try {
                await connection.Execute('MATCH (n) RETURN n;');
              } catch (e) {
                console.log('The second MATCH should fail: ' + e);
              }
              let data = await connection.FetchAll();
              console.log(data);
              data = await connection.FetchOne();
              console.log(data);
              data = await connection.FetchOne();
              console.log(data);
            } catch (e) {
              console.log(e);
            }
          })
          .catch((e) => {
            console.log(e);
          });
      })
      .catch((e) => {
        console.log(e);
      });
    console.log('Connecting...');
    await new Promise((resolve) => setTimeout(resolve, 1000));

    const txConn = await memgraph
      .Client()
      .Connect({ host: 'localhost', port: 7687 });
    await txConn.Begin();
    await txConn.Execute('CREATE (), ();');
    await txConn.FetchAll();
    await txConn.Execute('CREATE (), ();');
    const txConnData = await txConn.FetchAll();
    console.log(txConnData);
    await txConn.Commit();
  } catch (e) {
    console.log(e);
  }
})();
