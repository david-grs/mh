include_directories(..)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(mic_tests mic_tests.cc)
target_link_libraries(mic_tests ${GTEST_BOTH_LIBRARIES} pthread m)

add_executable(ht_tests ht_tests.cc)
target_link_libraries(ht_tests ${GTEST_BOTH_LIBRARIES} pthread m)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_link_libraries(mic_tests stdc++)
    target_link_libraries(ht_tests stdc++)
endif()

add_test(unit_tests mic_tests)
add_test(unit_tests ht_tests)

