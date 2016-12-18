#include "stats.h"
#include "benchmark.h"
#include "probes.h"

#include <random>
#include <iostream>

geiger::chrono mem_timer::_chrono = {};
double mem_timer::_elapsed_time_malloc = {};
double mem_timer::_elapsed_time_free = {};
double mem_timer::_elapsed_time_realloc = {};

int main(int argc, char** argv)
{
    geiger::init();

    long unsigned seed = argc == 2 ? std::atoll(argv[1]) : std::random_device()();
    std::cout << "seed = " << seed << std::endl;

    //probes(seed);
    benchmark_ht(seed);
    benchmark_ha(seed);

    return 0;
}
