#include "stats.h"
#include "benchmark.h"

int main(int argc, char** argv)
{
    if (argc == 2)
        benchmark(std::atoll(argv[1]));
    else
        benchmark();

    return 0;
}
