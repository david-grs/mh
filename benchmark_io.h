#include "benchmark.h"

#include <vector>

namespace io
{
    void cmp_tests_full(const std::vector<test>& tests, const std::vector<test>& ref_tests);
    void cmp_tests_short(const std::vector<test>& tests, const std::vector<test>& ref_tests);
}
