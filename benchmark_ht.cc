#include "benchmark.h"

#include "ht.h"
#include "mic_old.h"

#include <google/dense_hash_map>
#include <unordered_map>
#include <cstdint>

namespace detail
{

template <typename Container>
std::vector<test> benchmark_container(std::string desc, Container&& c, long unsigned seed)
{
    std::mt19937 gen(seed);

    std::uniform_int_distribution<> rng(1, 1e9);
    volatile int x = 0;

    benchmark bench(1000000, seed);
    bench(1000, [&]() { c.insert(std::make_pair(rng(gen), 222.0)); }, desc, "insert", "1K");
    bench(100000, [&]() { x += c.count(rng(gen)); }, desc, "lookup", "1K");
    bench(9000, [&]() { c.insert(std::make_pair(rng(gen), 222.0)); }, desc, "insert", "1-10K");
    bench(100000, [&]() { x += c.count(rng(gen)); }, desc, "lookup", "10K");
    bench(90000, [&]() { c.insert(std::make_pair(rng(gen), 222.0)); }, desc, "insert", "10-100K");
    bench(100000, [&]() { x += c.count(rng(gen)); }, desc, "lookup", "100K");
    bench(900000, [&]() { c.insert(std::make_pair(rng(gen), 222.0)); }, desc, "insert", "100K-1M");
    bench(100000, [&]() { x += c.count(rng(gen)); }, desc, "lookup", "1M");

    return bench.tests();
}

}

std::vector<test> benchmark_google(long unsigned seed)
{
    google::dense_hash_map<int32_t, double> gd;
    gd.set_empty_key(0);

    return detail::benchmark_container("google", gd, seed);
}

std::vector<test> benchmark_umap(long unsigned seed)
{
    return detail::benchmark_container("umap", std::unordered_map<int32_t, double>(), seed);
}

std::vector<test> benchmark_ht(long unsigned seed)
{
    ht<int32_t, double> mh(empty_key(0));

    return detail::benchmark_container("ht", mh, seed);
}


