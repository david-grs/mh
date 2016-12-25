#pragma once

#include "tsc_chrono.h"
#include "stats.h"

#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>

static constexpr const int Iterations = 3000000;

struct benchmark
{
    benchmark(std::size_t iterations) :
      _acc(iterations),
      _iterations(iterations)
    {}

    template <typename Callable>
    void operator()(Callable operation, const char* desc)
    {
        auto start = std::chrono::steady_clock::now();

        tsc_chrono chrono;
        chrono.start();

        for (int i = 0; i < _iterations; ++i)
        {
            operation();
            _acc.add(chrono.elapsed_and_restart());
        }
        auto end = std::chrono::steady_clock::now();

        // from TSC to nanoseconds
        auto& data = _acc.data();
        std::transform(std::begin(data), std::end(data), std::begin(data), [&](int64_t cycles) { return tsc_chrono::from_cycles(cycles).count(); });

        stats s = _acc.process();
        std::cout << desc << ": " << s << std::endl;
        _acc.clear();
    }

private:
    lazy_acc _acc;
    std::size_t _iterations;
};

void benchmark_ht(long unsigned seed);
void benchmark_ha(long unsigned seed);
