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

module.exports = Object.freeze({
  COUNT_NODES: `MATCH (n) RETURN count(n) AS nodes_no;`,
  COUNT_EDGES: `MATCH ()-[e]->() RETURN count(e) AS edges_no;`,
  DELETE_ALL: `MATCH (n) DETACH DELETE n;`,
  CREATE_TRIANGLE: `
   CREATE (n1:Node {id: 1}),
          (n2:Node {id: 2}),
          (n3:Node {id: 3}),
          (n1)-[e1:Edge {id: 1}]->(n2),
          (n2)-[e2:Edge {id: 2}]->(n3),
          (n3)-[e3:Edge {id: 3}]->(n1);`,
});
