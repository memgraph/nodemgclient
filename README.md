[![Actions Status](https://github.com/memgraph/nodemgclient/workflows/CI/badge.svg)](https://github.com/memgraph/nodemgclient/actions)

# nodemgclien - Memgraph client for Node.js

`nodemgclient` a Node.js binding for
[mgclient](https://github.com/memgraph/mgclient) used to interact with
Memgraph DBMS.

## Prerequisites

### Build prerequisites

`nodemgclient` is a wrapper around the
[mgclient](https://github.com/memgraph/mgclient) library. To install it from
sources you will need:
    * A Cmake >= 3.10.
    * A C compiler supporting C11 standard.
    * A C++ compiler supporting C++17 standard.
    * Node.js >= 10.4.
    * [mgclient](https://github.com/memgraph/mgclient) (headers and the lib).

Once prerequisites are met, you can install `nodemgclient` by running:

```bash
npm install
```
