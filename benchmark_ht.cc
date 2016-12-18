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

    std::unordered_map<int, double> umap, umap2;
    ht<int, double, empty_key<int, 0>> mh, mh2;

    google::dense_hash_map<int, double> gd, gd2;
    gd.set_empty_key(0);
    gd2.set_empty_key(0);

    std::uniform_int_distribution<> rng(1, 1e9);

    {
        gen.seed(seed);
        bench_stats()([&]() { umap2.insert(std::make_pair(rng(gen), 222.0)); }, "umap insert");

        gen.seed(seed);
        bench_stats()([&]() { mh2.insert(std::make_pair(rng(gen), 222.0)); }, "ht insert");

        gen.seed(seed);
        bench_stats()([&]() { gd2.insert(std::make_pair(rng(gen), 222.0)); }, "google insert");
    }

    {
        gen.seed(seed);
        bench()([&]() { umap.insert(std::make_pair(rng(gen), 222.0)); }, "umap insert");

        gen.seed(seed);
        bench()([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); }, "ht insert");

        gen.seed(seed);
        bench()([&]() { gd.insert(std::make_pair(rng(gen), 222.0)); }, "google insert");
    }

    volatile int i = 0;

    {
        //std::uniform_int_distribution<> rng(1, std::min(umap.size(), mh.size()) - 1);

        gen.seed(seed);
        bench()([&]() { i += umap.find(rng(gen)) != umap.end(); }, "umap lookup ex");

        gen.seed(seed);
        bench()([&]() { i += mh.find(rng(gen)); }, "mh lookup ex");

        gen.seed(seed);
        bench()([&]() { i += gd.find(rng(gen)) != gd.end(); }, "google lookup ex");
    }

    {
        std::uniform_int_distribution<> rng2(1e9 + 1, 2e9);

        gen.seed(seed);
        bench()([&]() { i += umap.find(rng2(gen)) != umap.end(); }, "umap lookup inex");

        gen.seed(seed);
        bench()([&]() { i += mh.find(rng2(gen)); }, "mh lookup inex");

        gen.seed(seed);
        bench()([&]() { i += gd.find(rng2(gen)) != gd.end(); }, "google lookup inex");
    }
}
