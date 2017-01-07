#include "stats.h"
#include "benchmark.h"
#include "probes.h"
#include "tsc_chrono.h"

#include "ext/rang.hpp"

#include <boost/tokenizer.hpp>

#include <sstream>
#include <fstream>
#include <random>
#include <iomanip>
#include <iostream>

auto load_ref_file(const std::string& filename)
{
    std::vector<test> ref_tests;
    std::ifstream ifs(filename);
    boost::char_separator<char> sep(",");

    for (std::string line; std::getline(ifs, line); )
    {
        boost::tokenizer<boost::char_separator<char>> tok(line, sep);
        assert(std::distance(tok.begin(), tok.end()) == 3);

        auto it = tok.begin();
        std::string desc = *it;
        ++it;

        unsigned long seed = std::stoll(*it);
        ++it;

        stats s;
        std::istringstream iss(*it);
        iss >> s;

        ref_tests.push_back({std::move(desc), seed, std::move(s)});
    }

    return ref_tests;
}

int main(int argc, char** argv)
{
    if (argc != 4 && argc != 5)
    {
        std::cerr << argv[0] << ": -ht|-ha [-gen|-cmp] N [seed]" << std::endl;
        return 1;
    }

    tsc_chrono::init();

    auto benchmark = argv[1] == std::string("-ht") ? benchmark_ht : benchmark_ha;
    const bool cmp = argv[2] == std::string("-cmp");
    const int n = std::atoi(argv[3]);
    long unsigned seed = argc == 5 ? std::atoll(argv[4]) : std::random_device()();

    //std::cout << "options: n=" << n << " seed=" << seed << " gen=" << std::boolalpha << gen << std::endl;

    std::vector<test> ref_tests = cmp ? load_ref_file("foo.txt") : std::vector<test>();

    std::uniform_int_distribution<> rng(1, std::numeric_limits<int>::max());
    std::vector<test> tests = benchmark(seed);

    if (cmp)
    {
        std::cout << std::setw(20) << std::left << "test";

        for (const test& t : tests)
            if (t.seed == seed)
                std::cout << std::setw(20) << std::left << t.name;
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
            std::cout << std::string(20 - oss.str().size(), ' ');
        };

        typemap<median_t, mean_t, stddev_t>().visit([&](auto field)
        {
            std::cout << std::setw(20) << std::left << field.name();
            for (const test& t : tests)
            {
                if (t.seed == seed)
                {
                    auto it = std::find_if(std::cbegin(ref_tests), std::cend(ref_tests), [&](const test& x) { return x.name == t.name && x.seed == t.seed; });
                    format(t.results.template get<decltype(field)>(), it->results.template get<decltype(field)>());
                }
            }
            std::cout << std::endl;
        });
    }

    return 0;
}
