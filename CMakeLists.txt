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
    # Benchmarks will be added later
endif()

# Installation
install(
    DIRECTORY include/
    DESTINATION include
)