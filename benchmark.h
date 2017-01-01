#pragma once

#include "tsc_chrono.h"
#include "stats.h"

#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <type_traits>
#include <cassert>

static constexpr const int Iterations = 3000000;
static constexpr const int K = 16;

struct benchmark
{
    benchmark() :
      _iterations(Iterations / K),
      _acc(_iterations)
    {}

    template <typename Callable>
    void operator()(Callable operation, const char* desc)
    {
        tsc_chrono chrono;
        chrono.start();

        using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
                                         std::chrono::high_resolution_clock,
                                         std::chrono::steady_clock>;

        auto start = Clock::now();
        for (std::size_t i = 0; i < _iterations; ++i)
        {
            for (int j = 0; j < K; ++j)
              operation();

            _acc.add(chrono.elapsed_and_restart());
        }
        auto end = Clock::now();

        // from TSC to nanoseconds
        auto& data = _acc.data();
        std::transform(std::begin(data), std::end(data), std::begin(data), [&](int64_t cycles) { return tsc_chrono::from_cycles(cycles).count(); });

        stats s = _acc.process();
        assert(std::llabs((int64_t)s.get<sum_t>() - (end - start).count()) > 1e6);

        std::cout << desc << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " - " << s << std::endl;
        _acc.clear();
    }

private:
    std::size_t _iterations;
    lazy_acc _acc;
};

void benchmark_ht(long unsigned seed);
void benchmark_ha(long unsigned seed);
