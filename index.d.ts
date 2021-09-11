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

export class Connection {
    constructor(client: any);
    client: any;
    Execute(query: any, params?: {}): Promise<any>;
    FetchAll(): Promise<any>;
    DiscardAll(): Promise<any>;
    Begin(): Promise<any>;
    Commit(): Promise<any>;
    Rollback(): Promise<any>;
    ExecuteAndFetchAll(query: any, params?: {}): Promise<any>;
}
export namespace Memgraph {
    export function Client_1(): any;
    export { Client_1 as Client };
    export function Connect_1(params: any): Promise<Connection>;
    export { Connect_1 as Connect };
}
import Client = Memgraph.Client;
import Connect = Memgraph.Connect;
export { Memgraph as default, Client, Connect };
