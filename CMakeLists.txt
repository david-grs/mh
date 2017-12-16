cmake_minimum_required(VERSION 2.8)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-std=c++17 HAVE_FLAG_STD_CXX17)
check_cxx_compiler_flag(-std=c++1z HAVE_FLAG_STD_CXX1Z)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else()
    if(HAVE_FLAG_STD_CXX17)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    elseif (HAVE_FLAG_STD_CXX1Z)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
    else()
        message(FATAL_ERROR "C++17 support required")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wpedantic -funroll-loops -Wconversion -Wswitch-default -Wswitch-enum -Wunreachable-code -Wwrite-strings -Wcast-align -Wshadow -Wundef -Wno-switch-default -Wold-style-cast -Wshadow -Wdouble-promotion")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat -Wno-c++98-compat-pedantic")
endif()

include_directories(.)

option(USE_TCMALLOC "USE_TCMALLOC" OFF)

add_executable(benchmark benchmark.cc stats.cc benchmark_io.cc benchmark_ht.cc benchmark_ha.cc)
target_link_libraries(benchmark m)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_link_libraries(benchmark stdc++)
endif()

if (USE_TCMALLOC)
    find_library(TCMALLOC_LIB NAMES tcmalloc_minimal)
    target_link_libraries(benchmark ${TCMALLOC_LIB})
endif ()

add_subdirectory(tests)

