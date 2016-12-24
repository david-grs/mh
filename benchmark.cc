#include "stats.h"
#include "benchmark.h"
#include "probes.h"
#include "tsc_chrono.h"

#include <random>
#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 2 && argc != 3)
    {
        std::cerr << argv[0] << ": -ht|-ha [seed]" << std::endl;
        return 1;
    }

    tsc_chrono::init();

    long unsigned seed = argc == 3 ? std::atoll(argv[2]) : std::random_device()();
    std::cout << "seed = " << seed << std::endl;

    //probes(seed);

    if (argv[1] == std::string("-ht"))
        benchmark_ht(seed);
    else
        benchmark_ha(seed);

    return 0;
}
