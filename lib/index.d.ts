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

// Keep in mind structural subtyping
// https://www.typescriptlang.org/docs/handbook/type-compatibility.html
// Keep in mind node-addon-api context
// https://stackoverflow.com/questions/44275172/typescript-declarations-file-for-node-c-addon

declare interface Record {
}

declare interface Cursor {
}

declare interface Connection {
  constructor(params?: any): Connection;
  Cursor(): Cursor;
  ExecuteLazy(query: string, params?: any): Cursor;
  Execute(query: string, params?: any): Promise<any>;
  ExecuteAndFetchRecords(query: string, params?: any): any;
}

declare function Connect(params: any): Connection;

export {Record, Cursor, Connection, Connect};
