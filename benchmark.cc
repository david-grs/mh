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

struct stats_cmp
{
    explicit stats_cmp(const stats& ref) :
      _ref(ref)
    {}

    template <typename SampleT>
    std::enable_if_t<std::is_arithmetic<typename SampleT::value_type>::value>
    operator()(SampleT sample)
    {
        SampleT ref = _ref.get<SampleT>();
        print(SampleT::name(), sample, ref);
    }

    void operator()(const quantiles_t& qs)
    {
        const quantiles_t& ref = _ref.get<quantiles_t>();

        for (int i = 0; i < (int)qs.size(); ++i)
        {
            auto ref_qs = std::find_if(std::cbegin(ref), std::cend(ref), [&](auto& p) { return p.first == qs[i].first; });
            print("q" + std::to_string(qs[i].first), qs[i].second, ref_qs->second);
        }
    }

private:
    template <typename StringT>
    void print(StringT&& name, double sample, double ref)
    {
        double diff = -100.0 + (100.0 * sample) / ref;
        std::cout << name << "=" << sample << " (";

        if (diff <= 0)
            std::cout << rang::fg::green;
        else
            std::cout << rang::fg::red << '+';

        std::cout << std::setw(2) << std::setprecision(2) << std::fixed << diff << rang::fg::reset << ") ";
    }

    const stats& _ref;
};

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
        for (const test& t : tests)
        {
            auto it = std::find_if(std::cbegin(ref_tests), std::cend(ref_tests), [&](const test& x) { return x.name == t.name && x.seed == t.seed; });
            assert_throw(it != ref_tests.end(), "ref test not found");

            const test& ref_test = *it;
            std::cout << ref_test.name << ": ";
            t.results.visit(stats_cmp(ref_test.results));
            std::cout << "\n";
        }
    }

    return 0;
}
