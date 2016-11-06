#include "probes.h"
#include "ht.h"
#include "stats.h"

#include <random>
#include <iostream>

void probes(unsigned long seed)
{
    auto benchmark = [](auto&& operation)
    {
        static const int Iterations = 1e6;

        for (int i = 0; i < Iterations; ++i)
            operation();
    };

    ht<int, double, empty_key<int, 0>> mh;

    std::uniform_int_distribution<> rng(1, 1e9);
    std::mt19937 gen(seed);
    volatile int i = 0;
    stats s;

    gen.seed(seed);
    benchmark([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); });

    gen.seed(seed);
    benchmark([&]()
    {
        i += mh.find(rng(gen), [&](std::size_t probes)
        {
            s.add(probes);
        });
    });

    std::cout << s << std::endl;
}
