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
#include <cstdio>
#include <map>

extern "C"
{
    #include <sys/types.h>
    #include <sys/wait.h>
}

auto load_tests_file(const std::string& filename)
{
    std::vector<test> ref_tests;
    std::ifstream ifs(filename);
    boost::char_separator<char> sep(",");

    for (std::string line; std::getline(ifs, line); )
    {
        boost::tokenizer<boost::char_separator<char>> tok(line, sep);

        if (std::distance(tok.begin(), tok.end()) != 5)
            throw std::runtime_error("wrong test file");

        auto it = tok.begin();
        std::string container = *it; ++it;
        std::string operation = *it; ++it;
        std::string desc = *it; ++it;

        unsigned long seed = std::stoll(*it);
        ++it;

        stats s;
        std::istringstream iss(*it);
        iss >> s;

        ref_tests.push_back({std::move(container), std::move(operation), std::move(desc), seed, std::move(s)});
    }

    return ref_tests;
}

void cmp_tests(const std::vector<test>& tests, const std::vector<test>& ref_tests)
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

        std::cout << desc << ": " << std::fixed << std::setprecision(2) << mean << " (";

        if (diff <= .0)
            std::cout << rang::fg::green;
        else
            std::cout << rang::fg::red << '+';

        std::cout << std::setprecision(2) << std::fixed << diff << '%' << rang::fg::reset <<  ")" << std::endl;
    }
}

inline std::ostream& operator<<(std::ostream& oss, const std::vector<test>& tests)
{
    for (auto& t : tests)
        oss << t.container << "," << t.operation << "," << t.description << "," << t.seed << "," << t.results << std::endl;
    return oss;
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << argv[0] << ": [-gen] N" << std::endl;
        return 1;
    }

    tsc_chrono::init();

    const bool write = argv[1] == std::string("-gen");
    const int cfg_runs = std::atoi(argv[2]);
    const std::string tmpf = std::tmpnam(nullptr);

    const std::string ref_filename("../samples.ref");
    //const auto benchmark_fcts = {benchmark_umap, benchmark_google, benchmark_ht};
    const auto benchmark_fcts = {benchmark_boost_mic, benchmark_ha};

    std::vector<test> ref_tests = load_tests_file(ref_filename);

    std::unordered_set<uint64_t> seeds;

    if (write)
    {
        for (int i = 0; i < cfg_runs; ++i)
            seeds.insert(std::random_device()());
    }
    else
    {
        for (const auto& rt : ref_tests)
            seeds.insert(rt.seed);
    }

    const int runs = std::min(cfg_runs, (int)seeds.size());

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

        {
            std::ofstream ofs(tmpf, std::ios_base::app | std::ios_base::out);
            ofs << tests;
        }

        pid_t child = fork();
        if (!child)
        {
            benchmark_and_fork(++bench_it);
        }
        else
        {
            waitpid(child, 0, 0);
            std::exit(0);
        }
    };

    benchmark_and_fork(std::cbegin(benchmarks));

    std::vector<test> tests = load_tests_file(tmpf);
    //cmp_tests(tests, ref_tests);
    cmp_tests_short(tests, ref_tests);

    if (write)
    {
        std::ofstream ofs(ref_filename);
        ofs << tests;
    }

    std::remove(tmpf.c_str());

    return 0;
}
