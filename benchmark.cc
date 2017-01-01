#include "stats.h"
#include "benchmark.h"
#include "probes.h"
#include "tsc_chrono.h"

#include <random>
#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 4 && argc != 5)
    {
        std::cerr << argv[0] << ": -ht|-ha [-gen|-cmp] N [seed]" << std::endl;
        return 1;
    }

    tsc_chrono::init();

    auto benchmark = argv[1] == std::string("-ht") ? benchmark_ht : benchmark_ha;
    const bool gen = argv[2] == std::string("-gen");
    const int n = std::atoi(argv[3]);
    long unsigned seed = argc == 5 ? std::atoll(argv[4]) : std::random_device()();

    std::cout << "options: n=" << n << " seed=" << seed << " gen=" << std::boolalpha << gen << std::endl;
    benchmark(seed);

    return 0;
}
