cmake_minimum_required(VERSION 2.8)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-std=c++17 HAVE_FLAG_STD_CXX17)
if(HAVE_FLAG_STD_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
    check_cxx_compiler_flag(-std=c++1z HAVE_FLAG_STD_CXX1Z)

    if(HAVE_FLAG_STD_CXX1Z)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -lstdc++")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -funroll-loops")

include_directories(.)

option(USE_TCMALLOC "USE_TCMALLOC" OFF)

add_executable(benchmark benchmark.cc stats.cc benchmark_io.cc benchmark_ht.cc benchmark_ha.cc)
target_link_libraries(benchmark m)

if (USE_TCMALLOC)
  find_library(TCMALLOC_LIB NAMES tcmalloc_minimal)
  target_link_libraries(benchmark ${TCMALLOC_LIB})
endif ()

add_subdirectory(tests)

