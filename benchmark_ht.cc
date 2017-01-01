#include "benchmark.h"

#include "ht.h"
#include "mic.h"

#include <google/dense_hash_map>
#include <unordered_map>

void benchmark_ht(long unsigned seed)
{
    std::cout << "============================\n";
    std::cout << "hash_table benchmark\n";
    std::cout << "============================" << std::endl;

    std::mt19937 gen(seed);

    std::unordered_map<int, double> umap;
    ht<int, double, empty_key<int, 0>> mh;

    google::dense_hash_map<int, double> gd;
    gd.set_empty_key(0);

    std::uniform_int_distribution<> rng(1, 1e9);
    volatile int x = 0;

    benchmark bench;
    bench.tear_down([&](const stats& s, const char* desc)
    {
        std::cout << desc << " " << seed << " " << s << std::endl;
        gen.seed(seed);
    });

    {
        bench([&]() { umap.insert(std::make_pair(rng(gen), 222.0)); }, "umap insert");
        bench([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); }, "ht insert");
        bench([&]() { gd.insert(std::make_pair(rng(gen), 222.0)); }, "google insert");
    }

    {
        //std::uniform_int_distribution<> rng(1, std::min(umap.size(), mh.size()) - 1);
        bench([&]() { x += umap.find(rng(gen)) != umap.end(); }, "umap lookup ex");
        bench([&]() { x += mh.find(rng(gen)); }, "mh lookup ex");
        bench([&]() { x += gd.find(rng(gen)) != gd.end(); }, "google lookup ex");
    }

    {
        std::uniform_int_distribution<> rng2(1e9 + 1, 2e9);
        bench([&]() { x += umap.find(rng2(gen)) != umap.end(); }, "umap lookup inex");
        bench([&]() { x += mh.find(rng2(gen)); }, "mh lookup inex");
        bench([&]() { x += gd.find(rng2(gen)) != gd.end(); }, "google lookup inex");
    }
}
