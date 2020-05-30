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
  NODES: `MATCH (n) RETURN n;`,
  EDGES: `MATCH ()-[e]->() RETURN e;`,
  NODE_EDGE_IDS: `MATCH (n)-[e]->(m) RETURN n.id, e.id;`,
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
  CREATE_RICH_NODE: `
    CREATE (n:Label1:Label2 {prop0: Null,
                             prop1: True,
                             prop2: False,
                             prop3: 10,
                             prop4: 100.0,
                             prop5: "test"})`,
  CREATE_RICH_EDGE: `
    CREATE ()-[e:Type {prop1: True,
	                   prop2: False,
					   prop3: 1,
					   prop4: 2.0,
					   prop5: "test"}]->();`,
  CREATE_PATH: `
    CREATE
      (:Label {id:1})
      -[:Type {id: 1}]->
      (:Label {id:2})
      -[:Type {id: 2}]->
      (:Label {id:3})
      -[:Type {id: 3}]->
      (:Label {id:4});`,
  MATCH_PATHS: `MATCH path=(startNode {id: 1})-[*]->(endNode) RETURN path;`,
  MATCH_RELATIONSHIPS: `
    MATCH path=(endNode {id: 2})<-[]-(startNode {id: 1})
    RETURN relationships(path);`,
  CREATE_NODE_USING_PARAMS: `
    CREATE (n:Node {nullProperty: $nullProperty,
                    trueProperty: $trueProperty,
                    falseProperty: $falseProperty,
                    bigIntProperty: $bigIntProperty,
                    numberProperty: $numberProperty,
                    stringProperty: $stringProperty,
                    arrayProperty: $arrayProperty,
                    objectProperty: $objectProperty});`,
  NAMED_COLUMNS: `RETURN "value_x" AS x, "value_y" AS y;`,
});
