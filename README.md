# hyperspirit

> A header-only, RFC compliant, HTTP parser in C++

## Overview

Hyperspirit is built with Boost.Spirit ("hypertext transfer protocol" + "boost spirit" = hyperspirit), a library for
creating parser grammars.

Features:
- Dependent only on Boost (1.46+).
- Runtime performance of a recursive descent parser.

## How to Build

There is no build step required as the library is header-only, but the following steps can be followed to build the unit
test suite.

### Requirements

- CMake
- Boost 1.46+

#### OSX

```bash
brew install cmake boost
```

#### Debian

```bash
# apt-get install cmake libboost
```

#### RHEL (5 or above)

```bash
# yum install cmake libboost
```

### Instructions

```bash
$ git clone git@github.com:cjgdev/hyperspirit.git
$ mkdir build && cd build
$ cmake ../ && make test
```

## Usage

```cpp
#include <hyperspirit/hyperspirit.hpp>

namespace hyperspirit::http = http;

// ...

// parse the `input` string
http::request req;
http::request_parser<string::const_iterator> grammar;
if (parse(input.begin(), input.end(), grammar, data)) {
    // the parsed data is in `req`
}
```

## Documentation

Please refer to the [wiki](wiki) for the latest documentation.

## Authors

- [@cjgdev](https://github.com/cjgdev)

## License

The library is licensed under the BSD license. A copy can be found [here](LICENSE).