#include "benchmark.h"

#include "ht.h"
#include "mic.h"

#include <google/dense_hash_map>

#include <cstdint>
#include <unordered_map>

std::vector<test> benchmark_ht(long unsigned seed)
{
    std::mt19937 gen(seed);

    ht<int32_t, double, empty_key<int32_t, 0>> mh;

    google::dense_hash_map<int32_t, double> gd;
    gd.set_empty_key(0);

    std::uniform_int_distribution<> rng(1, 1e9);
    volatile int x = 0;

    benchmark bench;
    std::vector<test> tests;

    bench.tear_down([&](const stats& s, const char* desc)
    {
        tests.push_back({desc, seed, s});
        gen.seed(seed);
    });

    {
        bench([&]() { gd.insert(std::make_pair(rng(gen), 222.0)); }, "google insert");
        bench([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); }, "ht insert");
    }

    assert_throw(mh.size() == gd.size(), "diff number of elements in compared hashtables");

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

    return tests;
}
