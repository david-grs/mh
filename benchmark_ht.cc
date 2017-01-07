#include "benchmark.h"

#include "ht.h"
#include "mic.h"

#include <google/dense_hash_map>

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
    bench(1000, [&]() { c.insert(std::make_pair(rng(gen), 222.0)); }, desc + " insert 1K");
    bench(100000, [&]() { x += c.count(rng(gen)); }, desc + " lookup 1K");
    bench(9000, [&]() { c.insert(std::make_pair(rng(gen), 222.0)); }, desc + " insert 1-10K");
    bench(100000, [&]() { x += c.count(rng(gen)); }, desc + " lookup 10K");
    bench(90000, [&]() { c.insert(std::make_pair(rng(gen), 222.0)); }, desc + " insert 10-100K");
    bench(100000, [&]() { x += c.count(rng(gen)); }, desc + " lookup 100K");
    bench(900000, [&]() { c.insert(std::make_pair(rng(gen), 222.0)); }, desc + " insert 100K-1M");
    bench(100000, [&]() { x += c.count(rng(gen)); }, desc + " lookup 1M");

    std::cout << c.size() << std::endl;
#if 0
    {
        bench([&]() { x += gd.find(rng(gen)) != gd.end(); }, "google lookup ex");
        bench([&]() { x += mh.find(rng(gen)); }, "ht lookup ex");
    }

    //assert_throw(x == (int)umap.size() * 3, "lookup ex failed");

    {
        x = 0;
        rng = std::uniform_int_distribution<>(1e9 + 1, 2e9);
        bench([&]() { x += gd.find(rng(gen)) != gd.end(); }, "google lookup inex");
        bench([&]() { x += mh.find(rng(gen)); }, "ht lookup inex");
    }

    assert_throw(x == 0, "lookup unex failed");
#endif
    return bench.tests();
}

}

std::vector<test> benchmark_google(long unsigned seed)
{
    google::dense_hash_map<int32_t, double> gd;
    gd.set_empty_key(0);

    return detail::benchmark_container("google", gd, seed);
}

std::vector<test> benchmark_ht(long unsigned seed)
{
    ht<int32_t, double, empty_key<int32_t, 0>> mh;

    return detail::benchmark_container("ht", mh, seed);
}


