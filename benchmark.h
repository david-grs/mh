#pragma once

#include "tsc_chrono.h"
#include "stats.h"

#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <type_traits>

static constexpr const int K = 32;

#define assert_throw(x, msg) if(!(x)) throw std::runtime_error(msg)

struct test
{
    std::string name;
    unsigned long seed;
    stats results;
};

struct benchmark
{
    benchmark(int total_iterations, long unsigned seed) :
      _acc(total_iterations / K),
      _seed(seed)
    {}

    template <typename Callable, typename StringT>
    void operator()(int iterations, Callable operation, StringT&& desc)
    {
        const int nb_samples = iterations / K;

        using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
                                         std::chrono::high_resolution_clock,
                                         std::chrono::steady_clock>;

        auto start = Clock::now();
        _chrono.start();
        for (int i = 0; i < nb_samples; ++i)
        {
            for (int j = 0; j < K; ++j)
                operation();

            _acc.add(_chrono.elapsed_and_restart());
        }
        auto end = Clock::now();

        // from TSC to nanoseconds
        auto& data = _acc.data();
        std::transform(std::begin(data), std::end(data), std::begin(data), [&](int64_t cycles) { return tsc_chrono::from_cycles(cycles).count(); });

        stats s = _acc.process(nb_samples);
        std::cout << (end - start).count() << std::endl;
//        assert_throw(std::llabs((int64_t)s.get<sum_t>() - (end - start).count()) < 1e6, "tsc_chrono and std::chrono::clock not synced");

        _tests.push_back({desc, _seed, s});
        _acc.clear();
    }

    auto& tests() const { return _tests; }

private:
    tsc_chrono _chrono;
    std::size_t _iterations;
    lazy_acc _acc;
    long unsigned _seed;
    std::vector<test> _tests;
};

std::vector<test> benchmark_ht(long unsigned seed);
std::vector<test> benchmark_google(long unsigned seed);
std::vector<test> benchmark_ha(long unsigned seed);

