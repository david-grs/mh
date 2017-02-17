#include "benchmark_io.h"
#include "typemap.h"

#include "ext/rang.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

#include <algorithm>
#include <map>
#include <tuple>

namespace io
{

static void cmp_tests_full_group(const std::vector<test>& tests, const std::vector<test>& ref_tests)
{
    static const int FieldWidth = 22;

    std::cout << std::setw(FieldWidth) << std::left << "test";

    for (const test& t : tests)
        std::cout << std::setw(FieldWidth) << std::left << t.name();
    std::cout << std::endl;

    auto format = [&](double sample, double ref)
    {
        double diff = -100.0 + (100.0 * sample) / ref;

        auto out = [&](auto& ss)
        {
            ss << std::fixed << std::setprecision(2) << sample << " (";

            if (diff <= .0)
                ss << rang::fg::green;
            else
                ss << rang::fg::red << '+';

            ss << std::setprecision(2) << std::fixed << diff << rang::fg::reset << ")";
        };

        // rang doesn't support ostringstream yet, so we have to format the string twice (to get its size)... :(
        std::ostringstream oss;
        out(oss);
        out(std::cout);

        if (oss.str().size() < FieldWidth)
            std::cout << std::string(FieldWidth - oss.str().size(), ' ');
    };

    typemap<median_t, mean_t, stddev_t, sum_t>().visit([&](auto field)
    {
        using SampleT = decltype(field);

        std::cout << std::setw(FieldWidth) << std::left << SampleT::name();
        for (const test& t : tests)
        {
            auto it = std::find_if(std::cbegin(ref_tests), std::cend(ref_tests), [&](const test& x) { return x.name() == t.name() && x.seed == t.seed; });
            double ref = it != std::cend(ref_tests) ? it->results.template get<SampleT>() : .0;

            format(t.results.template get<SampleT>(), ref);
        }
        std::cout << std::endl;
    });
}

void cmp_tests_full(const std::vector<test>& tests, const std::vector<test>& ref_tests)
{
    std::vector<test> group;
    group.reserve(tests.size());

    std::tuple<std::string /*container*/, unsigned long> last;

    for (int i = 0; i < (int)tests.size(); ++i)
    {
        auto curr = std::make_tuple(tests[i].container, tests[i].seed);
        if (i > 0 && curr != last)
        {
            cmp_tests_full_group(group, ref_tests);
            std::cout << std::endl;
            group.clear();
        }

        group.push_back(tests[i]);
        last = std::move(curr);
    }
}

void cmp_tests_short(const std::vector<test>& curr_tests, const std::vector<test>& ref_tests)
{
    auto get_means = [&](const std::vector<test>& tests)
    {
        using group = std::pair<std::string, std::string>;
        std::map<group, std::vector<double>> groups;

        for (const auto& t : tests)
        {
            auto id = std::make_pair(t.container, t.operation);
            groups[id].emplace_back(t.results.get<median_t>());
        }

        std::map<group, double> means;
        for (const auto& g : groups)
            means[g.first] = std::accumulate(std::cbegin(g.second), std::cend(g.second), 0) / (double)g.second.size();

        return means;
    };

    auto means = get_means(curr_tests);
    auto ref_means = get_means(ref_tests);

    for (const auto& g : means)
    {
        const std::string desc = g.first.first + " " + g.first.second;
        const double mean = g.second;

        auto it = ref_means.find(g.first);
        if (it == ref_means.end())
        {
            std::cout << "cannot find reference for test " << desc << std::endl;
            continue;
        }

        const double ref_mean = it->second;
        const double diff = -100.0 + (100.0 * mean) / ref_mean;

        std::cout << desc << ": " << std::fixed << std::setprecision(2) << mean << " ns/op (";

        if (diff <= .0)
            std::cout << rang::fg::green;
        else
            std::cout << rang::fg::red << '+';

        std::cout << std::setprecision(2) << std::fixed << diff << '%' << rang::fg::reset <<  ")" << std::endl;
    }
}

}
