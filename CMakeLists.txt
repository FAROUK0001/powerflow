cmake_minimum_required(VERSION 3.20)

project(PowerFlowSolver LANGUAGES CXX)

# --------------------------------------------------
# Build options
# --------------------------------------------------

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(POWERFLOW_BUILD_TESTS "Build test executables" ON)
option(POWERFLOW_BUILD_BENCHMARKS "Build performance benchmark executable" OFF)
option(POWERFLOW_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(POWERFLOW_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(POWERFLOW_ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(POWERFLOW_ENABLE_LTO "Enable link-time optimization" OFF)

if(POWERFLOW_ENABLE_TSAN AND (POWERFLOW_ENABLE_ASAN OR POWERFLOW_ENABLE_UBSAN))
    message(FATAL_ERROR "ThreadSanitizer cannot be combined with AddressSanitizer or UndefinedBehaviorSanitizer due to runtime conflicts")
endif()

if(POWERFLOW_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT POWERFLOW_IPO_SUPPORTED OUTPUT POWERFLOW_IPO_OUTPUT)
    if(NOT POWERFLOW_IPO_SUPPORTED)
        message(WARNING "POWERFLOW_ENABLE_LTO requested but IPO/LTO is not supported: ${POWERFLOW_IPO_OUTPUT}")
    endif()
endif()

function(powerflow_apply_common_settings target_name)
    target_compile_features(${target_name} PRIVATE cxx_std_23)
    target_include_directories(${target_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    set(powerflow_sanitize_compile_options "")
    set(powerflow_sanitize_link_options "")

    if(POWERFLOW_ENABLE_ASAN)
        if(MSVC)
            list(APPEND powerflow_sanitize_compile_options /fsanitize=address)
            list(APPEND powerflow_sanitize_link_options /fsanitize=address)
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            list(APPEND powerflow_sanitize_compile_options -fsanitize=address -fno-omit-frame-pointer)
            list(APPEND powerflow_sanitize_link_options -fsanitize=address)
        else()
            message(WARNING "POWERFLOW_ENABLE_ASAN is not supported for compiler '${CMAKE_CXX_COMPILER_ID}' and will be ignored")
        endif()
    endif()

    if(POWERFLOW_ENABLE_UBSAN)
        if(MSVC)
            message(WARNING "POWERFLOW_ENABLE_UBSAN is not supported with MSVC and will be ignored")
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            list(APPEND powerflow_sanitize_compile_options -fsanitize=undefined -fno-omit-frame-pointer)
            list(APPEND powerflow_sanitize_link_options -fsanitize=undefined)
        else()
            message(WARNING "POWERFLOW_ENABLE_UBSAN is not supported for compiler '${CMAKE_CXX_COMPILER_ID}' and will be ignored")
        endif()
    endif()

    if(POWERFLOW_ENABLE_TSAN)
        if(MSVC)
            message(WARNING "POWERFLOW_ENABLE_TSAN is not supported with MSVC and will be ignored")
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            list(APPEND powerflow_sanitize_compile_options -fsanitize=thread)
            list(APPEND powerflow_sanitize_link_options -fsanitize=thread)
        else()
            message(WARNING "POWERFLOW_ENABLE_TSAN is not supported for compiler '${CMAKE_CXX_COMPILER_ID}' and will be ignored")
        endif()
    endif()

    if(powerflow_sanitize_compile_options)
        target_compile_options(${target_name} PRIVATE ${powerflow_sanitize_compile_options})
    endif()

    if(powerflow_sanitize_link_options)
        target_link_options(${target_name} PRIVATE ${powerflow_sanitize_link_options})
    endif()

    if(POWERFLOW_ENABLE_LTO AND POWERFLOW_IPO_SUPPORTED)
        set_target_properties(${target_name} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endfunction()

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

        core/parsers/parser_utils.hpp
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
        core/linalg/matrix_dense.hpp
        core/linalg/matrix_sparse_csr.hpp
        core/linalg/vector.hpp
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
# Main executable
# --------------------------------------------------

add_executable(
        PowerFlowSolver
        ${SOURCES}
        ${PARSERS}
        ${MODELS}
        ${LINALG}
        ${SOLVER}
)
powerflow_apply_common_settings(PowerFlowSolver)
target_compile_definitions(PowerFlowSolver PRIVATE
        POWERFLOW_DATA_DIR="${CMAKE_SOURCE_DIR}/feeder34/data")

# --------------------------------------------------
# Benchmarks
# --------------------------------------------------

if(POWERFLOW_BUILD_BENCHMARKS)
    add_executable(
            PowerFlowPerfBench
            tests/perf_benchmark.cpp
            core/models/ybus_builder.cpp
            core/parsers/line_parser.cpp
            core/parsers/phase_config_parser.cpp
            core/parsers/load_parser.cpp
            core/parsers/distributed_load_parser.cpp
            core/parsers/capacitor_parser.cpp
            core/parsers/transformer_parser.cpp
            core/solver/newton_raphson.cpp
            core/solver/jacobian_builder.cpp
    )
    powerflow_apply_common_settings(PowerFlowPerfBench)
    target_compile_definitions(PowerFlowPerfBench PRIVATE
            POWERFLOW_DATA_DIR="${CMAKE_SOURCE_DIR}/feeder34/data")
endif()

# --------------------------------------------------
# Tests
# --------------------------------------------------

if(POWERFLOW_BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(
            PowerFlowUnitTests
            tests/test_sparse_matrix.cpp
            tests/test_sparse_complex.cpp
            tests/test_real_grid.cpp
    )
    target_link_libraries(PowerFlowUnitTests PRIVATE Catch2::Catch2WithMain)
    powerflow_apply_common_settings(PowerFlowUnitTests)

    include(Catch)
    catch_discover_tests(PowerFlowUnitTests)

    add_executable(
            YBusDumper
            tests/ybus_dumper.cpp
            core/models/ybus_builder.cpp
            core/parsers/line_parser.cpp
            core/parsers/phase_config_parser.cpp
            core/parsers/capacitor_parser.cpp
            core/parsers/transformer_parser.cpp
    )
    powerflow_apply_common_settings(YBusDumper)
    target_compile_definitions(YBusDumper PRIVATE
            POWERFLOW_DATA_DIR="${CMAKE_SOURCE_DIR}/feeder34/data")
endif()
