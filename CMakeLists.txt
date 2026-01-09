cmake_minimum_required(VERSION 3.25)
project(limit_order_book LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX /permissive-")
endif()

option(BUILD_TESTS "Build tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

add_library(lob_spec INTERFACE)
target_include_directories(lob_spec INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(lob_runtime INTERFACE)
target_include_directories(lob_runtime INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lob_runtime INTERFACE lob_spec)

add_library(lob_compile_time INTERFACE)
target_include_directories(lob_compile_time INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lob_compile_time INTERFACE lob_spec)

if(BUILD_TESTS)
    enable_testing()
    
    add_executable(reference_tests tests/reference/reference_book.hpp)
    target_link_libraries(reference_tests PRIVATE lob_spec)
    
    add_executable(conformance_tests tests/conformance/main.cpp)
    target_link_libraries(conformance_tests PRIVATE lob_spec)
    add_test(NAME conformance_tests COMMAND conformance_tests)
endif()

if(BUILD_EXAMPLES)
    add_executable(runtime_example examples/runtime_basic.cpp)
    target_link_libraries(runtime_example PRIVATE lob_runtime)
    
    add_executable(compile_time_example examples/compile_time_basic.cpp)
    target_link_libraries(compile_time_example PRIVATE lob_compile_time)
endif()

if(BUILD_BENCHMARKS)
    # Benchmarks will be added later
endif()

install(DIRECTORY include/ DESTINATION include)