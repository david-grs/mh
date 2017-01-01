#include "benchmark.h"

#include "ht.h"
#include "mic.h"

#include <google/dense_hash_map>

#include <cstdint>
#include <unordered_map>

std::vector<test> benchmark_ht(long unsigned seed)
{
    std::mt19937 gen(seed);

    std::unordered_map<int32_t, double> umap;
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
        std::cout << desc << "," << seed << "," << s << std::endl;
        gen.seed(seed);
    });

    {
        bench([&]() { umap.insert(std::make_pair(rng(gen), 222.0)); }, "umap insert");
        bench([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); }, "ht insert");
        bench([&]() { gd.insert(std::make_pair(rng(gen), 222.0)); }, "google insert");
    }

    assert_throw(umap.size() == mh.size() && mh.size() == gd.size(), "diff number of elements in compared hashtables");

    {
        bench([&]() { x += umap.find(rng(gen)) != umap.end(); }, "umap lookup ex");
        bench([&]() { x += mh.find(rng(gen)); }, "mh lookup ex");
        bench([&]() { x += gd.find(rng(gen)) != gd.end(); }, "google lookup ex");
    }

    //assert_throw(x == (int)umap.size() * 3, "lookup ex failed");

    {
        x = 0;
        rng = std::uniform_int_distribution<>(1e9 + 1, 2e9);
        bench([&]() { x += umap.find(rng(gen)) != umap.end(); }, "umap lookup inex");
        bench([&]() { x += mh.find(rng(gen)); }, "mh lookup inex");
        bench([&]() { x += gd.find(rng(gen)) != gd.end(); }, "google lookup inex");
    }

    assert_throw(x == 0, "lookup unex failed");

    return tests;
}
