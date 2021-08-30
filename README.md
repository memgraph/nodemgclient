[![Actions Status](https://github.com/memgraph/nodemgclient/workflows/CI/badge.svg)](https://github.com/memgraph/nodemgclient/actions)

# nodemgclient - Node.js Memgraph Client

`nodemgclient` a Node.js binding for
[mgclient](https://github.com/memgraph/mgclient) used to interact with
[Memgraph](https://memgraph.com).

## Installation

`nodemgclient` is a wrapper around the
[mgclient](https://github.com/memgraph/mgclient).

### Build from Source

 To install `nodemgclient` from sources you will need (only Linux is tested at
 the moment):

* OpenSSL >= 1.0.2
* A CMake >= 3.10
* A Clang compiler supporting C11 standard
* A Clang compiler supporting C++17 standard
* Node.js >= 12

First install the prerequisites:

* On Debian/Ubuntu:

```bash
sudo apt install -y npm nodejs cmake make gcc g++ clang libssl-dev
```

* On CentOS:

```bash
sudo yum install -y npm nodejs cmake3 make gcc gcc-c++ clang openssl-devel
```

Once prerequisites are in place, you can build `nodemgclient` by running:

```bash
npm ci
npm run build:release
```

To run tests, [Docker](https://docs.docker.com/engine/install) is required.
