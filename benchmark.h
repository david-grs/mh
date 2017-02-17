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
    std::string name() const { return container + " " + operation + " " + description; }

    std::string container;
    std::string operation;
    std::string description;
    unsigned long seed;
    stats results;
};

struct benchmark
{
    benchmark(int total_iterations, long unsigned seed) :
      _acc(total_iterations / K),
      _seed(seed)
    {}

    template <typename Callable>
    void operator()(int iterations, Callable operation, std::string container, std::string op_desc, std::string desc)
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
        // we also discard the first samples if we have a lot, because they're usually noisy
        const int DiscardedSamples = nb_samples > 100 ? 3 : 0;
        std::transform(std::cbegin(samples) + DiscardedSamples, std::cbegin(samples) + nb_samples, std::begin(samples), [&](int64_t cycles)
        {
            return tsc_chrono::from_cycles(cycles / static_cast<double>(K)).count();
        });
        nb_samples -= DiscardedSamples;

        {
            std::ofstream ofs(container + "_" + op_desc + "_" + desc + ".stat");
            for (int i = 0; i < nb_samples; ++i)
            {
                ofs << std::to_string(samples[i]);

                if (i + 1 != nb_samples)
                    ofs << ",";
            }
        }

        stats s = _acc.process(nb_samples);
        _tests.push_back({container, op_desc, desc, _seed, s});
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
std::vector<test> benchmark_umap(long unsigned seed);

std::vector<test> benchmark_ha(long unsigned seed);
std::vector<test> benchmark_boost_mic(long unsigned seed);

