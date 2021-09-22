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
/**
  * Create Memgraph compatible date object.
  * @param {number} days - The number of days since 1970-01-01 (Unix epoch).
  */
export function createMgDate(days: number): {
    objectType: string;
    days: number;
};
/**
  * Create Memgraph compatible local time object.
  * @param {number} nanoseconds - The number of nanoseconds since midnight.
  */
export function createMgLocalTime(nanoseconds: number): {
    objectType: string;
    nanoseconds: number;
};
/**
  * Create Memgraph compatible local date time object.
  * @param {number} seconds - The number of seconds since 1970-01-01T00:00:00
  * (Unix epoch).
  * @param {number} nanoseconds - The number of nanoseconds since the last
  * second.
  */
export function createMgLocalDateTime(seconds: number, nanoseconds: number): {
    objectType: string;
    seconds: number;
    nanoseconds: number;
};
/**
  * Create Memgraph compatible duration object.
  * NOTE: Semantically Memgraph duration is a sum of all
  * components (days, seconds, nanoseconds).
  * @param {number} days - The number of days.
  * @param {number} seconds - The number of seconds.
  * @param {number} nanoseconds - The number of nanoseconds.
  */
export function createMgDuration(days: number, seconds: number, nanoseconds: number): {
    objectType: string;
    days: number;
    seconds: number;
    nanoseconds: number;
};
import Client = Memgraph.Client;
import Connect = Memgraph.Connect;
export { Memgraph as default, Client, Connect };
