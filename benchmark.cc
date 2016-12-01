#include "benchmark.h"
#include "ht.h"
#include "mic.h"

#include <google/dense_hash_map>


#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/random_access_index.hpp>

#include <random>
#include <chrono>
#include <unordered_map>

using namespace boost::multi_index;

void benchmark(long unsigned seed)
{
    auto benchmark = [](auto&& operation, const char* desc)
    {
        static const int Iterations = 3e6;

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < Iterations; ++i)
            operation();
        auto end = std::chrono::steady_clock::now();

        std::cout << desc << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    };

    std::mt19937 gen(seed);

    //std::unordered_map<int, double> umap;
    ht<int, double, empty_key<int, 0>> mh, mh2;
    hash_array<int, double, empty_key<int, 0>> mha, mha2;

    boost::multi_index_container<
      std::pair<int, double>,
      indexed_by<
        hashed_unique<
          member<std::pair<int, double>, int, &std::pair<int, double>::first>
        >,
        sequenced<>
      >
    > mic_hs, mic_hs2;

    google::dense_hash_map<int, double> gd, gd2;
    gd.set_empty_key(0);
    gd2.set_empty_key(0);

    std::uniform_int_distribution<> rng(1, 1e9);

    {
        //gen.seed(seed);
        //benchmark([&]() { umap.insert(std::make_pair(rng(gen), 222.0)); }, "umap insert");

        gen.seed(seed);
        benchmark([&]() { mh2.insert(std::make_pair(rng(gen), 222.0)); }, "mh insert");

        gen.seed(seed);
        benchmark([&]() { mha2.insert(std::make_pair(rng(gen), 222.0)); }, "mha insert");

        gen.seed(seed);
        benchmark([&]() { mic_hs2.insert(std::make_pair(rng(gen), 222.0)); }, "mic insert");

        gen.seed(seed);
        benchmark([&]() { gd2.insert(std::make_pair(rng(gen), 222.0)); }, "google insert");
    }

    {
        //gen.seed(seed);
        //benchmark([&]() { umap.insert(std::make_pair(rng(gen), 222.0)); }, "umap insert");

        gen.seed(seed);
        benchmark([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); }, "mh insert");

        gen.seed(seed);
        benchmark([&]() { mha.insert(std::make_pair(rng(gen), 222.0)); }, "mha insert");

        gen.seed(seed);
        benchmark([&]() { mic_hs.insert(std::make_pair(rng(gen), 222.0)); }, "mic insert");

        gen.seed(seed);
        benchmark([&]() { gd.insert(std::make_pair(rng(gen), 222.0)); }, "google insert");
    }

    volatile int i = 0;

    {
        //std::uniform_int_distribution<> rng(1, std::min(umap.size(), mh.size()) - 1);

        //gen.seed(seed);
        //benchmark([&]() { i += umap.find(rng(gen)) != umap.end(); }, "umap lookup ex");

        gen.seed(seed);
        benchmark([&]() { i += mh.find(rng(gen)); }, "mh lookup ex");
#if 0
        gen.seed(seed);
        benchmark([&]() { i += mha.find(rng(gen)); }, "mha lookup ex");
#endif
        gen.seed(seed);
        benchmark([&]() { i += gd.find(rng(gen)) != gd.end(); }, "google lookup ex");
    }

    {
        std::uniform_int_distribution<> rng2(1e9 + 1, 2e9);

        //gen.seed(seed);
        //benchmark([&]() { i += umap.find(rng2(gen)) != umap.end(); }, "umap lookup inex");

        gen.seed(seed);
        benchmark([&]() { i += mh.find(rng2(gen)); }, "mh lookup inex");

#if 0
        gen.seed(seed);
        benchmark([&]() { i += mha.find(rng2(gen)); }, "mha lookup inex");
#endif

        gen.seed(seed);
        benchmark([&]() { i += gd.find(rng2(gen)) != gd.end(); }, "google lookup inex");
    }
}
