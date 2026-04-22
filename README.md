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

Run test executables directly:

```bash
./build/TestSparse
./build/TestComplexSparse
./build/TestRealGrid
```

Or via CTest:

```bash
ctest --test-dir build --output-on-failure
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

CMake compile command export is enabled by default in this project for improved tooling integration.
