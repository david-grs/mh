#pragma once

#include "stats.h"

#include <geiger/chrono.h>

#include <chrono>
#include <vector>
#include <iostream>
#include <random>

static constexpr const int Iterations = 3000000;

struct bench
{
    template <typename Callable>
    void operator()(Callable operation, const char* desc)
    {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < Iterations; ++i)
            operation();
        auto end = std::chrono::steady_clock::now();

        std::cout << desc << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    }
};

struct bench_stats
{
    bench_stats() :
      _ts(Iterations)
    {}

    template <typename Callable>
    void operator()(Callable operation, const char* desc)
    {
        geiger::chrono chrono;
        for (int i = 0; i < Iterations; ++i)
        {
            chrono.start();
            operation();
            _ts[i] = chrono.elapsed();
            chrono.restart();
        }

        stats st;
        for (int i = 0; i < Iterations; ++i)
            st.add(_ts[i]);

        std::cout << desc << ": " << st << std::endl;
    }

    std::vector<int64_t> _ts;
};


void benchmark_ht(long unsigned seed);
void benchmark_ha(long unsigned seed);
