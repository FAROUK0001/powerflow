cmake_minimum_required(VERSION 3.20)
project(PowerFlowSolver)

# 1. Set to C++23!
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Tell CMake where to look when you type #include "core/models/bus.hpp"
# The "." means "start looking from the root directory"
include_directories(.)

# 3. List ALL your files here.
# We include the .hpp files here just so CLion shows them nicely in the project tree!
set(SOURCES
        main.cpp

        # Parsers (The New Clean Architecture!)
        core/parsers/line_parser.cpp
        core/parsers/line_parser.hpp
        core/parsers/phase_config_parser.cpp
        core/parsers/phase_config_parser.hpp

        # Core Models (Implementations)
        core/models/network.cpp
        core/models/ybus_builder.cpp

        # Core Models Headers (For CLion visibility)
        core/models/branch.hpp
        core/models/bus.hpp
        core/models/network.hpp
        core/models/regulator.hpp
        core/models/system_config.hpp
        core/models/transformer.hpp
        core/models/ybus_builder.hpp
        core/models/phase_config.hpp

        # Core Math & Memory Headers (For CLion visibility)
        core/linalg/lu.hpp
        core/linalg/matrix_dense.hpp
        core/linalg/matrix_sparse_csr.hpp
        core/linalg/vector.hpp
        core/memory/linear_allocator.hpp
        core/memory/pool_allocator.hpp
        core/solver/convergence.hpp
        core/solver/newton_raphson.hpp
        core/threading/task_queue.hpp
        core/threading/thread_pool.hpp
        core/parsers/phase_config_parser.cpp
        core/parsers/phase_config_parser.hpp
        core/parsers/transformer_parser.hpp
        core/parsers/transformer_parser.cpp
        core/models/load.hpp
        core/parsers/load_parser.hpp
        core/parsers/load_parser.cpp
        core/models/distributed_load.hpp
        core/parsers/distributed_load_parser.cpp
        core/parsers/distributed_load_parser.hpp
        core/parsers/regulator_parser.hpp
        core/parsers/regulator_parser.cpp
        core/models/capacitor.hpp
        core/parsers/capacitor_parser.hpp
        core/parsers/capacitor_parser.cpp
)

# 4. Create the actual executable program
add_executable(PowerFlowSolver ${SOURCES})
add_executable(TestSparse tests/test_sparse_matrix.cpp)
add_executable(TestComplexSparse tests/test_sparse_complex.cpp)
add_executable(TestRealGrid tests/test_real_grid.cpp)