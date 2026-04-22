# PowerFlow Solver

[![CMake CI](https://github.com/FAROUK0001/powerflow/actions/workflows/ci.yml/badge.svg)](https://github.com/FAROUK0001/powerflow/actions/workflows/ci.yml)

PowerFlow Solver is a C++ power-system analysis project focused on sparse-matrix based power-flow
building blocks, Y-bus construction, and Newton-Raphson style solver components.

## Prerequisites

- CMake 3.20+
- A C++23-capable compiler:
  - GCC 13+
  - Clang 16+
  - MSVC (recent Visual Studio 2022 toolset)

## Build (CMake)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Run

Run the main solver executable:

```bash
./build/PowerFlowSolver
```

Run the unit test executable directly:

```bash
./build/PowerFlowUnitTests
```

Or via CTest:

```bash
ctest --test-dir build --output-on-failure
```

Disable tests when needed:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPOWERFLOW_BUILD_TESTS=OFF
```

## Development notes

### Build types

Use standard CMake build types such as `Debug` and `Release`.

### Project CMake options

- `POWERFLOW_BUILD_TESTS` (default: `ON`) — build test executables and register CTest tests.
- `POWERFLOW_ENABLE_ASAN` (default: `OFF`) — enable AddressSanitizer.
- `POWERFLOW_ENABLE_UBSAN` (default: `OFF`) — enable UndefinedBehaviorSanitizer.
- `POWERFLOW_ENABLE_TSAN` (default: `OFF`) — enable ThreadSanitizer.
- `POWERFLOW_ENABLE_LTO` (default: `OFF`) — enable IPO/LTO when supported.

Example development configure command:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DPOWERFLOW_BUILD_TESTS=ON -DPOWERFLOW_ENABLE_ASAN=ON
```

Sanitizer examples:

```bash
# AddressSanitizer + UndefinedBehaviorSanitizer (Linux, GCC/Clang)
cmake -S . -B build-asan-ubsan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPOWERFLOW_BUILD_TESTS=ON \
  -DPOWERFLOW_ENABLE_ASAN=ON \
  -DPOWERFLOW_ENABLE_UBSAN=ON
cmake --build build-asan-ubsan --parallel
ctest --test-dir build-asan-ubsan --output-on-failure

# ThreadSanitizer (mutually exclusive with ASAN/UBSAN)
cmake -S . -B build-tsan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPOWERFLOW_BUILD_TESTS=ON \
  -DPOWERFLOW_ENABLE_TSAN=ON
cmake --build build-tsan --parallel
ctest --test-dir build-tsan --output-on-failure
```

CMake compile command export is enabled by default in this project for improved tooling integration.
