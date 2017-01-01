#include "stats.h"
#include "benchmark.h"
#include "probes.h"
#include "tsc_chrono.h"

#include <boost/tokenizer.hpp>

#include <sstream>
#include <fstream>
#include <random>
#include <iostream>

template <typename StringT, typename Callable>
void load_file(StringT&& filename, Callable f)
{
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

        f(std::move(desc), seed, std::move(s));
    }
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

    std::uniform_int_distribution<> rng(1, std::numeric_limits<int>::max());
    std::vector<test> tests = benchmark(seed);

    if (cmp)
    {
        std::vector<test> ref_tests;
        load_file("foo.txt", [&](std::string&& desc, unsigned long seed, stats&& s)
        {
            std::cout << "loading " << desc << " seed " << seed << " stats " << s.get<sum_t>() << std::endl;
            ref_tests.push_back({std::move(desc), seed, std::move(s)});
        });

        for (const test& t : tests)
        {
            auto it = std::find_if(std::cbegin(ref_tests), std::cend(ref_tests), [&](const test& x) { return x.name == t.name && x.seed == t.seed; });
            assert_throw(it != ref_tests.end(), "ref test not found");
        }
    }

    return 0;
}
