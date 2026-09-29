cmake_minimum_required(VERSION 3.14)
project(hunters-solver VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CUDA "Enable CUDA GPU Acceleration" OFF)
option(BUILD_TESTS "Build Test Suite" ON)

if(MSVC)
    add_compile_options(/W4 /O2 /EHsc)
else()
    add_compile_options(-Wall -Wextra -O3 -pedantic)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(HUNTERS_CORE_SOURCES
    src/model/Model.cpp
    src/model/LPParser.cpp
    src/sparse/SparseMatrix.cpp
    src/sparse/LinearAlgebra.cpp
    src/lp/BasisMatrix.cpp
    src/lp/Pricing.cpp
    src/lp/RatioTest.cpp
    src/lp/SimplexSolver.cpp
    src/presolve/Scaling.cpp
    src/presolve/Presolver.cpp
    src/milp/BranchingRules.cpp
    src/milp/Heuristics.cpp
    src/milp/BranchAndBound.cpp
    src/numerical/SolutionValidator.cpp
    src/gpu/GpuContext.cpp
    src/gpu/GpuSpMV.cpp
    src/api/HuntersSolver.cpp
)

if(ENABLE_CUDA)
    enable_language(CUDA)
    add_definitions(-DHUNTERS_ENABLE_CUDA)
    list(APPEND HUNTERS_CORE_SOURCES
        cuda/spmv_kernel.cu
        cuda/vector_kernel.cu
    )
endif()

# Core Static Library
add_library(hunters_core STATIC ${HUNTERS_CORE_SOURCES})

# CLI Executable
add_executable(hunters-solver src/main.cpp)
target_link_libraries(hunters-solver PRIVATE hunters_core)

# Test Suite
if(BUILD_TESTS)
    enable_testing()
    file(GLOB TEST_SOURCES
        "tests/unit/*.cpp"
        "tests/numerical/*.cpp"
        "tests/integration/*.cpp"
        "tests/test_runner.cpp"
    )
    if(TEST_SOURCES)
        add_executable(hunters_tests ${TEST_SOURCES})
        target_link_libraries(hunters_tests PRIVATE hunters_core)
        add_test(NAME HuntersTestSuite COMMAND hunters_tests)
    endif()
endif()
