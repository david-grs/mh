#pragma once

#include "tsc_chrono.h"
#include "stats.h"

#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <fstream>
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
        int nb_samples = iterations / K;

        //using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
        //                                 std::chrono::high_resolution_clock,
        //                                 std::chrono::steady_clock>;

        for (int i = 0; i < nb_samples; ++i)
        {
            _chrono.start();
            for (int j = 0; j < K; ++j)
                operation();

            _acc.add(_chrono.elapsed());
        }

        auto& samples = _acc.data();

        // from TSC to nanoseconds
        // we also discard the first samples, they're usually crappy
        std::transform(std::cbegin(samples) + 3, std::cbegin(samples) + nb_samples, std::begin(samples), [&](int64_t cycles)
        {
            return tsc_chrono::from_cycles(cycles / static_cast<double>(K)).count();
        });
        nb_samples -= 3;

        {
            std::ofstream ofs(desc + ".stat");
            for (int i = 0; i < nb_samples; ++i)
            {
                ofs << std::to_string(samples[i]);

                if (i + 1 != nb_samples)
                    ofs << ",";
            }
        }

        stats s = _acc.process(nb_samples);
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

