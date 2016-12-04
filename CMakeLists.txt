cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include_directories(.)

option(USE_TCMALLOC "USE_TCMALLOC" OFF)

add_executable(mic mic.cc)
add_executable(benchmark benchmark.cc stats.cc benchmark_ht.cc benchmark_ha.cc)

if (USE_TCMALLOC)
  find_library(TCMALLOC_LIB NAMES tcmalloc_minimal)
  target_link_libraries(benchmark ${TCMALLOC_LIB})
endif ()

