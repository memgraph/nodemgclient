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

declare class Result {
}

declare class Cursor {
}

declare class Connection {
  constructor(params?: any);
  Cursor(): Cursor;
  ExecuteLazy(query: string, params?: any): Cursor;
  Execute(query: string, params?: any): Promise<any>;
}

declare function Connect(params: any): Connection;

export {Result, Cursor, Connection, Connect};
