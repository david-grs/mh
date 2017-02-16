#include "stats.h"
#include "benchmark.h"
#include "benchmark_io.h"
#include "probes.h"
#include "tsc_chrono.h"

#include <boost/tokenizer.hpp>

#include <fstream>
#include <random>
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


inline std::ostream& operator<<(std::ostream& oss, const std::vector<test>& tests)
{
    for (auto& t : tests)
        oss << t.container << "," << t.operation << "," << t.description << "," << t.seed << "," << t.results << std::endl;
    return oss;
}

int main(int argc, char** argv)
{
    if (argc != 3 && argc != 4)
    {
        std::cerr << argv[0] << ": [-v] [-gen] N" << std::endl;
        return 1;
    }

    tsc_chrono::init();

    const bool verbose = argv[1] == std::string("-v");
    const bool write = (argc == 3 && argv[1] == std::string("-gen"))
                        || (argc == 4 && argv[2] == std::string("-gen"));
    const int cfg_runs = std::atoi(argv[argc - 1]);
    const std::string tmpf = std::tmpnam(nullptr);

    const std::string ref_filename("../samples.ref");
    //const auto benchmark_fcts = {benchmark_umap, benchmark_google, benchmark_ht};
    const auto benchmark_fcts = {benchmark_boost_mic, benchmark_ha};

    std::vector<test> ref_tests = load_tests_file(ref_filename);

    std::unordered_set<uint64_t> seeds;

    if (write)
    {
        std::random_device rnd;
        std::mt19937 gen(rnd());

        std::uniform_int_distribution<> rng;

        for (int i = 0; i < cfg_runs; ++i)
            seeds.insert(rng(gen));
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

    for (int i = 0; i < (int)benchmarks.size(); ++i)
    {
        pid_t child = fork();
        if (child)
        {
             waitpid(child, 0, 0);
        }
        else
        {
            std::vector<test> tests = benchmarks[i]();
            std::ofstream ofs(tmpf, std::ios_base::app | std::ios_base::out);
            ofs << tests;
            std::exit(0);
        }
    }

    std::vector<test> tests = load_tests_file(tmpf);

    if (verbose)
        io::cmp_tests_full(tests, ref_tests);
    else
        io::cmp_tests_short(tests, ref_tests);

    if (write)
    {
        std::ofstream ofs(ref_filename);
        ofs << tests;
    }

    std::remove(tmpf.c_str());
    return 0;
}
