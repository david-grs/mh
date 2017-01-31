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
#include <unordered_set>

extern "C"
{
    #include <sys/types.h>
    #include <sys/wait.h>
}

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

void cmp_tests(const std::vector<test>& tests, const std::vector<test>& ref_tests)
{
    static const int FieldWidth = 22;

    std::cout << std::setw(FieldWidth) << std::left << "test";

    for (const test& t : tests)
        std::cout << std::setw(FieldWidth) << std::left << t.name;
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
            auto it = std::find_if(std::cbegin(ref_tests), std::cend(ref_tests), [&](const test& x) { return x.name == t.name && x.seed == t.seed; });
            double ref = it != std::cend(ref_tests) ? it->results.template get<SampleT>() : .0;

            format(t.results.template get<SampleT>(), ref);
        }
        std::cout << std::endl;
    });
}

inline std::ostream& operator<<(std::ostream& oss, const std::vector<test>& tests)
{
    for (auto& t : tests)
        oss << t.name << "," << t.seed << "," << t.results << std::endl;
    return oss;
}

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        std::cerr << argv[0] << ": -ht|-ha [-gen] N" << std::endl;
        return 1;
    }

    tsc_chrono::init();

    //auto benchmark = argv[1] == std::string("-ht") ? benchmark_ht : benchmark_ha;
    const bool write = argv[2] == std::string("-gen");
    int runs = std::atoi(argv[3]);

    const std::string ref_filename("samples.ref");
    std::vector<test> ref_tests = load_ref_file(ref_filename);

    std::unordered_set<uint64_t> seeds;
    for (const auto& rt : ref_tests)
        seeds.insert(rt.seed);

    if (write)
    {
        unlink(ref_filename.c_str());

        for (int i = 0; i < runs; ++i)
            seeds.insert(std::random_device()());
    }

    runs = std::min(runs, (int)seeds.size());

    const auto benchmark_fcts = {benchmark_umap, benchmark_google, benchmark_ht};

    using benchmark_fct = std::function<std::vector<test>()>;
    std::vector<benchmark_fct> benchmarks;
    benchmarks.reserve(runs * benchmark_fcts.size());

    for (auto f : benchmark_fcts)
    {
        auto seed_it = std::cbegin(seeds);

        for (int i = 0; i < runs; ++i, ++seed_it)
        {
            const uint64_t seed = *seed_it;
            benchmarks.emplace_back([=]() { return f(seed); });
        }
    }

    std::function<void(decltype(benchmarks)::const_iterator)> benchmark_and_fork = [&](auto bench_it)
    {
        if (bench_it == std::cend(benchmarks))
            return;

        std::vector<test> tests = (*bench_it)();
        cmp_tests(tests, ref_tests);
        std::cout << std::endl;

        if (write)
        {
            std::ofstream ofs(ref_filename, std::ios_base::app | std::ios_base::out);
            ofs << tests;
        }

        pid_t child = fork();

        if (!child)
            benchmark_and_fork(++bench_it);
        else
            waitpid(child, 0, 0);
    };

    benchmark_and_fork(std::cbegin(benchmarks));
    return 0;
}
