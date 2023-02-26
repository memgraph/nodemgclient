[![Actions Status](https://github.com/memgraph/nodemgclient/workflows/CI/badge.svg)](https://github.com/memgraph/nodemgclient/actions)
[![status: experimental](https://github.com/GIScience/badges/raw/master/status/experimental.svg)](https://github.com/GIScience/badges#experimental)

# nodemgclient - Node.js Memgraph Client

`nodemgclient` a Node.js binding for
[mgclient](https://github.com/memgraph/mgclient) used to interact with
[Memgraph](https://memgraph.com).

## Installation

### Build from Source on Linux

To install `nodemgclient` from source you will need:

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

### Build from Source on Windows

#### Build on Windows using Visual Studio

Since `cmake-js` is used, compiling for Windows is very similar to compiling
for Linux:
```bash
npm ci
npm run build:release
```

If installing OpenSSL package from
https://slproweb.com/products/Win32OpenSSL.html, make sure to use the full one
because of the header files.

NOTE: Compilation does NOT work yet under MinGW.

## Build from Source on MacOS

To build on MacOS it's required to install the `openssl` package, e.g.:
```
brew install openssl
```
Once the package is in place, please set the `OPENSSL_ROOT_DIR` environment variable:
```
export OPENSSL_ROOT_DIR="$(brew --prefix openssl)"
```
Once OpenSSL is in place, please run:
```
npm ci
npm run build:release
```

NOTE: For more adventurous folks, since `cmake-js` is used, it's also possible to set
the OpenSSL path via the following commend:
```
npx cmake-js compile --CDOPENSSL_ROOT_DIR="$(brew --prefix openssl)"
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
