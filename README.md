CharIoT arithmetic domain propagation
=====================================

The domsec library provides arithmetic manipulation for abstract domains:
intervals, singleton, disjunctions of elements, symbolic domains.

A demonstrator is available in the [tests](tests) directory.

The API in available [domsec.h](domsec.h) that includes
[AbstractDomain.h](applications/Analyzer/Scalar/AbstractDomain.h).

# build domsec

You first need to go in the `chariot-semantic-domains` directory.

Then the standard `cmake` commands should build the executable `domsec`.

```sh
mkdir build
cd build
cmake ..
make
# make -j 8
```

# demo of domsec

In the build directory, the command

```sh
make test ARGS="-V"
```

runs the unit-test `domsec` on the file `tests/test_general.txt`.
It parses every line and calls the adequate forward/backward transfer
function on the input domains.
The resulting domain is then printed on the standard output.

