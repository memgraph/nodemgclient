[![Actions Status](https://github.com/memgraph/nodemgclient/workflows/CI/badge.svg)](https://github.com/memgraph/nodemgclient/actions)

# nodemgclient - Node.js Memgraph Client

`nodemgclient` a Node.js binding for
[mgclient](https://github.com/memgraph/mgclient) used to interact with
[Memgraph](https://memgraph.com).

## Prerequisites

### Build

`nodemgclient` is a wrapper around the
[mgclient](https://github.com/memgraph/mgclient). To install it from
sources you will need:

* A CMake >= 3.10.
* A Clang compiler supporting C11 standard.
* A Clang compiler supporting C++17 standard.
* Node.js >= 10.4.

Once prerequisites are in place, you can build `nodemgclient` by running:

```bash
npm install
npm run build:release
```
