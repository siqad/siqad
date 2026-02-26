# AGENTS.md

## Cursor Cloud specific instructions

### Overview

SiQAD (Silicon Quantum Atomic Designer) is a C++17/Qt6 native desktop CAD application for designing and simulating silicon dangling bond (Si-DB) circuits. It is **not** a web app — it is a single compiled binary plus simulation plugins.

### Build and run

See `README.md` for the canonical build dependency list. The project builds via:

```
CMAKE_FLAGS="-DCMAKE_CXX_COMPILER=g++-13 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=release" ./make_everything_dev release
```

The compiled binary is at `./build/release/siqad`. Git submodules **must** be initialized before building (`git submodule update --init --recursive`).

### Non-obvious caveats

- **Compiler selection**: On Ubuntu 24.04 the default `c++` symlink points to Clang 18, which selects GCC 14's libstdc++ installation but `libstdc++-14-dev` is not available. You must explicitly pass `-DCMAKE_CXX_COMPILER=g++-13` via `CMAKE_FLAGS` to avoid linker errors (`cannot find -lstdc++`).
- **Qt offscreen mode**: For headless testing set `QT_QPA_PLATFORM=offscreen`. The unit test target already sets this automatically via `set_tests_properties`.
- **Tests**: Unit tests are in `build/src/` (not the top-level `build/`). Run with `cd build/src && QT_QPA_PLATFORM=offscreen ctest --output-on-failure`.
- **Plugin pip warnings**: On startup, SiQAD may log warnings about HoppingDynamics and PoisSolver plugins failing to install pip dependencies. These are non-critical Python-based plugins; the core GUI and C++-based simulators (SimAnneal, ExhaustiveGS, QuickExact, QuickSim, ClusterComplete) work without them.
- **cpu_cores detection**: The `make_everything_dev` script uses `grep "^core id" /proc/cpuinfo | sort -u | wc -l` which may return 1 on some VMs. Builds still work but may be slow.
