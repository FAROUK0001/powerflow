cmake_minimum_required(VERSION 3.20)

project(PowerFlowSolver)

# --------------------------------------------------
# C++ Standard
# --------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------
# Include directories
# --------------------------------------------------
include_directories(.)

# --------------------------------------------------
# Source files
# --------------------------------------------------

set(SOURCES
        main.cpp

        # ---------------- Core Models ----------------
        core/models/ybus_builder.cpp
)

# --------------------------------------------------
# Parsers
# --------------------------------------------------

set(PARSERS
        core/parsers/line_parser.cpp
        core/parsers/line_parser.hpp

        core/parsers/phase_config_parser.cpp
        core/parsers/phase_config_parser.hpp

        core/parsers/transformer_parser.cpp
        core/parsers/transformer_parser.hpp

        core/parsers/load_parser.cpp
        core/parsers/load_parser.hpp

        core/parsers/distributed_load_parser.cpp
        core/parsers/distributed_load_parser.hpp

        core/parsers/regulator_parser.cpp
        core/parsers/regulator_parser.hpp

        core/parsers/capacitor_parser.cpp
        core/parsers/capacitor_parser.hpp
)

# --------------------------------------------------
# Models (headers only)
# --------------------------------------------------

set(MODELS
        core/models/branch.hpp
        core/models/regulator.hpp
        core/models/transformer.hpp
        core/models/ybus_builder.hpp
        core/models/phase_config.hpp
        core/models/load.hpp
        core/models/distributed_load.hpp
        core/models/capacitor.hpp
)

# --------------------------------------------------
# Linear Algebra
# --------------------------------------------------

set(LINALG
        core/linalg/lu.hpp
        core/linalg/matrix_dense.hpp
        core/linalg/matrix_sparse_csr.hpp
        core/linalg/vector.hpp
)

# --------------------------------------------------
# Memory
# --------------------------------------------------

set(MEMORY
        core/memory/linear_allocator.hpp
        core/memory/pool_allocator.hpp
)

# --------------------------------------------------
# Solver
# --------------------------------------------------

set(SOLVER
        core/solver/convergence.hpp
        core/solver/newton_raphson.hpp
        core/solver/jacobian_builder.hpp
        core/solver/newton_raphson.cpp
        core/solver/jacobian_builder.cpp
)

# --------------------------------------------------
# Threading
# --------------------------------------------------

set(THREADING
        core/threading/task_queue.hpp
        core/threading/thread_pool.hpp
)

# --------------------------------------------------
# Main executable
# --------------------------------------------------

add_executable(
        PowerFlowSolver
        ${SOURCES}
        ${PARSERS}
        ${MODELS}
        ${LINALG}
        ${MEMORY}
        ${SOLVER}
        ${THREADING}
)

# --------------------------------------------------
# Tests
# --------------------------------------------------

add_executable(TestSparse tests/test_sparse_matrix.cpp)
add_executable(TestComplexSparse tests/test_sparse_complex.cpp)
add_executable(TestRealGrid tests/test_real_grid.cpp)
add_executable(YBusDumper tests/ybus_dumper.cpp core/models/ybus_builder.cpp core/parsers/line_parser.cpp core/parsers/phase_config_parser.cpp core/parsers/capacitor_parser.cpp core/parsers/transformer_parser.cpp)