#pragma once

#include "tsc_chrono.h"
#include "stats.h"

#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <type_traits>

static constexpr const int Iterations = 3000000;
static constexpr const int K = 16;

#define assert_throw(x, msg) if(!(x)) throw std::runtime_error(msg)

struct benchmark
{
    benchmark() :
      _iterations(Iterations / K),
      _acc(_iterations)
    {}

    template <typename Callable>
    void tear_down(Callable c)
    {
        _tear_down = c;
    }

    template <typename Callable>
    void operator()(Callable operation, const char* desc)
    {
        using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
                                         std::chrono::high_resolution_clock,
                                         std::chrono::steady_clock>;

        auto start = Clock::now();
        _chrono.start();
        for (std::size_t i = 0; i < _iterations; ++i)
        {
            for (int j = 0; j < K; ++j)
              operation();

            _acc.add(_chrono.elapsed_and_restart());
        }
        auto end = Clock::now();

        // from TSC to nanoseconds
        auto& data = _acc.data();
        std::transform(std::begin(data), std::end(data), std::begin(data), [&](int64_t cycles) { return tsc_chrono::from_cycles(cycles).count(); });

        stats s = _acc.process();
        assert_throw(std::llabs((int64_t)s.get<sum_t>() - (end - start).count()) < 1e6, "tsc_chrono and std::chrono::clock not synced");

        _tear_down(s, desc);
        _acc.clear();
    }

private:
    tsc_chrono _chrono;
    std::size_t _iterations;
    lazy_acc _acc;
    std::function<void(const stats&, const char*)> _tear_down;
};

struct test
{
    std::string name;
    unsigned long seed;
    stats results;
};

std::vector<test> benchmark_ht(long unsigned seed);
std::vector<test> benchmark_ha(long unsigned seed);

