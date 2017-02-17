include_directories(..)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(unit_tests ht_tests.cc)
target_link_libraries(unit_tests ${GTEST_BOTH_LIBRARIES} pthread)
add_test(unit_tests unit_tests)

