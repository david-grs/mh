#pragma once

#include "tsc_chrono.h"
#include "stats.h"

#include <chrono>
#include <vector>
#include <iostream>
#include <random>

static constexpr const int Iterations = 3000000;

struct benchmark
{
    benchmark(std::size_t iterations) :
      _acc(iterations)
    {}

    template <typename Callable>
    void operator()(Callable operation, const char* desc)
    {
        auto start = std::chrono::steady_clock::now();

        tsc_chrono chrono;
        chrono.start();

        for (int i = 0; i < Iterations; ++i)
        {
            operation();
            _acc.add(chrono.elapsed_and_restart());
        }
        auto end = std::chrono::steady_clock::now();

        stats s = _acc.process();
        std::cout << desc << ": " << s << std::endl;
    }

private:
    lazy_acc _acc;
};

void benchmark_ht(long unsigned seed);
void benchmark_ha(long unsigned seed);
