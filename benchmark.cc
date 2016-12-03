#include "stats.h"
#include "benchmark.h"
#include "probes.h"

#include <random>
#include <iostream>

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        long unsigned seed = std::atoll(argv[1]);
        std::cout << "seed = " << seed << std::endl;

        //probes(seed);
        benchmark_ht(seed);
        benchmark_ha(seed);
    }
    else
    {
        std::random_device rd;
        long unsigned seed = rd();
        std::cout << "seed = " << seed << std::endl;

        //probes(seed);
        benchmark_ht(seed);
        benchmark_ha(seed);
    }

    return 0;
}
