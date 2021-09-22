[![Actions Status](https://github.com/memgraph/nodemgclient/workflows/CI/badge.svg)](https://github.com/memgraph/nodemgclient/actions)

# nodemgclient - Node.js Memgraph Client

`nodemgclient` a Node.js binding for
[mgclient](https://github.com/memgraph/mgclient) used to interact with
[Memgraph](https://memgraph.com).

## Installation

### Build from Source

To install `nodemgclient` from sources you will need (only Linux is tested at
the moment):

* OpenSSL >= 1.0.2
* A CMake >= 3.10
* A Clang compiler supporting C11 and C++17 standard
* Node.js >= 12

First install the prerequisites:

* On Debian/Ubuntu:

```bash
sudo apt install -y npm nodejs cmake make gcc g++ clang libssl-dev
```

* On RedHat/CentOS:

```bash
sudo yum install -y npm nodejs cmake3 make gcc gcc-c++ clang openssl-devel
```

Once prerequisites are in place, you can build `nodemgclient` by running:

```bash
npm ci
npm run build:release
```

To test ([Docker](https://docs.docker.com/engine/install) is required) run:

```bash
npm run test
```

## Implementation and Interface Notes

### Temporal Types

Suitable JS type to store Memgrpah temporal types don't exist. In particular,
it's impossible to convert `mg_duration` and `mg_local_time` to the `Date`
type. Since [the temporal
specification](https://github.com/tc39/proposal-temporal) is not yet widely
supported, the decision was to expose plain JS objects (dict) with the exact
fields `mgclient` is providing (for more details, please take a look under
`mgclient`
[header](https://github.com/memgraph/mgclient/blob/master/include/mgclient.h)
and [source](https://github.com/memgraph/mgclient/blob/master/src/mgclient.c)
files). In addition, when possible (`mg_date` and `mg_local_date_time`), are
converted into objects which have `date` property,
which in fact, is the JS `Date` representation of these types. Keep in mind the
loss of precision because JS `Date` time fields can only store up to
milliseconds precision. However, Memgraph supports microsecond precision for
the local time and therefore any use of the `date` property (JS `Date` object)
can potentially cause loss of information.

Module exposes `create` functions, e.g. `createMgDate`, which simplify creation
of temporal object interpretable by Memgraph. For more details take a look
under the API docs under [index.js](./index.js) file.
