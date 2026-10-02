# SiQAD tests

See [the testing guide](../../docs/development/testing.rst) for prerequisites,
commands, fixtures, labels, native clipboard requirements, reports and remaining
coverage.

```sh
cmake -S . -B build-tests -DCMAKE_BUILD_TYPE=Debug -DSIQAD_BUILD_PLUGINS=OFF
cmake --build build-tests --parallel 4
ctest --test-dir build-tests --output-on-failure --no-tests=error
```

Run these from the repository root. Building and testing are separate operations.
`BUILD_TESTING=OFF` omits tests and their QtTest/Python dependencies.
