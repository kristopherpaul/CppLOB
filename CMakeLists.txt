cmake_minimum_required(VERSION 3.25)
project(limit_order_book LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /WX /permissive-)
endif()

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

# Libraries
add_library(lob_spec INTERFACE)

target_include_directories(lob_spec
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(lob_runtime INTERFACE)

target_link_libraries(lob_runtime
    INTERFACE
        lob_spec
)

target_include_directories(lob_runtime
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(lob_compile_time INTERFACE)

target_link_libraries(lob_compile_time
    INTERFACE
        lob_spec
)

target_include_directories(lob_compile_time
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Tests
if(BUILD_TESTS)
    enable_testing()

    add_executable(conformance_tests
        tests/conformance/main.cpp
    )

    target_link_libraries(conformance_tests
        PRIVATE
            lob_spec
            lob_runtime
            lob_compile_time
    )

    target_include_directories(conformance_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    add_test(
        NAME conformance_tests
        COMMAND conformance_tests
    )

    add_executable(runtime_invariants
        tests/runtime/invariants_test.cpp
    )

    target_link_libraries(runtime_invariants
        PRIVATE
            lob_runtime
    )

    add_test(
        NAME runtime_invariants
        COMMAND runtime_invariants
    )

    set(COMPILE_TIME_TEST_SOURCES
        core_test
        book_invariants_test
        match_result_test
    )

    foreach(TEST_NAME IN LISTS COMPILE_TIME_TEST_SOURCES)
        add_executable(compile_time_${TEST_NAME}
            tests/compile_time/${TEST_NAME}.cpp
        )

        target_link_libraries(compile_time_${TEST_NAME}
            PRIVATE
                lob_compile_time
        )

        target_include_directories(compile_time_${TEST_NAME}
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/tests
        )

        add_test(
            NAME compile_time_${TEST_NAME}
            COMMAND compile_time_${TEST_NAME}
        )
    endforeach()
endif()

# Examples
if(BUILD_EXAMPLES)
    add_executable(runtime_example
        examples/runtime_basic.cpp
    )

    target_link_libraries(runtime_example
        PRIVATE
            lob_runtime
    )

    add_executable(compile_time_example
        examples/compile_time_basic.cpp
    )

    target_link_libraries(compile_time_example
        PRIVATE
            lob_compile_time
    )
endif()

# Benchmarks
if(BUILD_BENCHMARKS)
    include(FetchContent)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark
        GIT_TAG v1.8.3
    )
    FetchContent_MakeAvailable(benchmark)

    # Benchmark configuration options
    option(BENCHMARK_CONFIG_SMALL "Run small config benchmarks (1-1000 prices, 10K orders)" ON)
    option(BENCHMARK_CONFIG_MEDIUM "Run medium config benchmarks (1-5000 prices, 50K orders)" ON)
    option(BENCHMARK_CONFIG_LARGE "Run large config benchmarks (1-10000 prices, 100K orders)" OFF)

    # Compile-time benchmark launcher script
    set(CT_COMPILE_TIMER "${CMAKE_CURRENT_SOURCE_DIR}/scripts/compile_timer.py")
    set(CT_GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_ct_benchmarks.py")

    # Runtime benchmarks
    add_executable(runtime_benchmarks
        benchmarks/runtime/runtime_submit.cpp
        benchmarks/runtime/runtime_cancel.cpp
        benchmarks/runtime/runtime_best_price.cpp
        benchmarks/runtime/runtime_evolution.cpp
        benchmarks/runtime/runtime_allocation.cpp
        benchmarks/runtime/workloads.cpp
    )
    target_link_libraries(runtime_benchmarks PRIVATE lob_runtime benchmark::benchmark benchmark::benchmark_main)
    target_include_directories(runtime_benchmarks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/runtime)
    target_compile_options(runtime_benchmarks PRIVATE -DBENCHMARK_CONFIG_SMALL=${BENCHMARK_CONFIG_SMALL} -DBENCHMARK_CONFIG_MEDIUM=${BENCHMARK_CONFIG_MEDIUM} -DBENCHMARK_CONFIG_LARGE=${BENCHMARK_CONFIG_LARGE})

    # Compile-time benchmarks - generate configurations
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/benchmarks/compile_time/generated/.generated
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/benchmarks/compile_time/generated
        COMMAND python3 ${CT_GENERATOR} 
            ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/compile_time/compile_time_scaling.cpp.in
            ${CMAKE_CURRENT_BINARY_DIR}/benchmarks/compile_time/generated
        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/benchmarks/compile_time/generated/.generated
        COMMENT "Generating compile-time benchmark configurations"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Create compile-time benchmark targets from generated sources
    add_custom_target(compile_time_benchmarks_generate DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/benchmarks/compile_time/generated/.generated)

    # Function to create CT benchmark targets from generated files
    function(create_ct_benchmark_targets)
        file(GLOB CT_GENERATED_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/benchmarks/compile_time/generated/*.cpp")
        foreach(src ${CT_GENERATED_SOURCES})
            get_filename_component(name ${src} NAME_WE)
            add_executable(ct_${name} ${src})
            target_link_libraries(ct_${name} PRIVATE lob_compile_time lob_spec)
            set_target_properties(ct_${name} PROPERTIES EXCLUDE_FROM_ALL TRUE)
            # Use compile timer for measurement
            set_target_properties(ct_${name} PROPERTIES
                CXX_COMPILER_LAUNCHER "${CT_COMPILE_TIMER};${CMAKE_CXX_COMPILER}"
            )
        endforeach()
    endfunction()

    # Generate targets at configure time (will be empty initially, but works after first build)
    # For actual use, run: cmake --build build --target compile_time_benchmarks_generate && cmake build
    create_ct_benchmark_targets()

    # Binary size measurement target
    add_custom_target(measure_binary_size
        COMMAND ${CMAKE_COMMAND} -E echo "=== Binary Size Analysis ==="
        COMMAND ${CMAKE_COMMAND} -E echo "Runtime benchmarks executable:"
        COMMAND ${CMAKE_COMMAND} -E echo "  size: \$<TARGET_FILE:runtime_benchmarks>"
        COMMAND ${CMAKE_COMMAND} -E env size \$<TARGET_FILE:runtime_benchmarks> 2>/dev/null || ${CMAKE_COMMAND} -E echo "  (size command not available)"
        COMMAND ${CMAKE_COMMAND} -E echo "Compile-time example:"
        COMMAND ${CMAKE_COMMAND} -E env size \$<TARGET_FILE:compile_time_example> 2>/dev/null || ${CMAKE_COMMAND} -E echo "  (build with BUILD_EXAMPLES=ON)"
        COMMENT "Measuring binary sizes..."
    )
endif()

# Installation
install(
    DIRECTORY include/
    DESTINATION include
)