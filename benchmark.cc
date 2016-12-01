#include "benchmark.h"
#include "ht.h"
#include "mic.h"
#include "stats.h"

#include <geiger/chrono.h>

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
#include <array>

using namespace boost::multi_index;

struct bench
{
    template <typename Callable>
    void operator()(Callable operation, const char* desc)
    {
        static const int Iterations = 3e6;

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < Iterations; ++i)
            operation();
        auto end = std::chrono::steady_clock::now();

        std::cout << desc << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    }
};

struct bench_stats
{
    template <typename Callable>
    void operator()(Callable operation, const char* desc)
    {
        _ts.reserve(Iterations);
        
        geiger::chrono chrono;
        for (int i = 0; i < Iterations; ++i)
        {
            chrono.start();
            operation();
            _ts[i] = chrono.elapsed();
            chrono.restart();
        }

        stats st;
        for (int i = 0; i < Iterations; ++i)
            st.add(_ts[i]);

        std::cout << desc << ": " << st << std::endl;
    }

    static constexpr const int Iterations = 3000000;
    std::vector<int64_t> _ts;
};

void benchmark(long unsigned seed)
{
    geiger::init();

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

#if 0
    std::cout << "mh = " << (void*)mh._table.get() << std::endl;
    std::cout << "mha = " << (void*)mha._hashtable._table.get() << std::endl;
    std::cout << "mha = " << (void*)mha._sequence.data() << std::endl;

    std::cout << "mh2 = " << (void*)mh2._table.get() << std::endl;
    std::cout << "mha2 = " << (void*)mha2._hashtable._table.get() << std::endl;
    std::cout << "mha2 = " << (void*)mha2._sequence.data() << std::endl;
#endif
//TODO see why mha outputs diff results with/without mha/mha2
    {
        //gen.seed(seed);
        //bench()([&]() { umap.insert(std::make_pair(rng(gen), 222.0)); }, "umap insert");

        gen.seed(seed);
        bench_stats()([&]() { mh2.insert(std::make_pair(rng(gen), 222.0)); }, "mh insert");

        gen.seed(seed);
        bench_stats()([&]() { mha2.insert(std::make_pair(rng(gen), 222.0)); }, "mha insert");

        gen.seed(seed);
        bench_stats()([&]() { mic_hs2.insert(std::make_pair(rng(gen), 222.0)); }, "mic insert");

        gen.seed(seed);
        bench_stats()([&]() { gd2.insert(std::make_pair(rng(gen), 222.0)); }, "google insert");
    }

    {
        //gen.seed(seed);
        //bench()([&]() { umap.insert(std::make_pair(rng(gen), 222.0)); }, "umap insert");

        gen.seed(seed);
        bench()([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); }, "mh insert");

        gen.seed(seed);
        bench()([&]() { mha.insert(std::make_pair(rng(gen), 222.0)); }, "mha insert");

        gen.seed(seed);
        bench()([&]() { mic_hs.insert(std::make_pair(rng(gen), 222.0)); }, "mic insert");

        gen.seed(seed);
        bench()([&]() { gd.insert(std::make_pair(rng(gen), 222.0)); }, "google insert");
    }

#if 0
    {
        //gen.seed(seed);
        //bench()([&]() { umap.insert(std::make_pair(rng(gen), 222.0)); }, "umap insert");

        gen.seed(seed);
        bench()([&]() { mh2.insert(std::make_pair(rng(gen), 222.0)); }, "mh insert");

        gen.seed(seed);
        bench()([&]() { mha2.insert(std::make_pair(rng(gen), 222.0)); }, "mha insert");

        gen.seed(seed);
        bench()([&]() { mic_hs2.insert(std::make_pair(rng(gen), 222.0)); }, "mic insert");

        gen.seed(seed);
        bench()([&]() { gd2.insert(std::make_pair(rng(gen), 222.0)); }, "google insert");
    }
#endif

    volatile int i = 0;

    {
        //std::uniform_int_distribution<> rng(1, std::min(umap.size(), mh.size()) - 1);

        //gen.seed(seed);
        //bench()([&]() { i += umap.find(rng(gen)) != umap.end(); }, "umap lookup ex");

        gen.seed(seed);
        bench()([&]() { i += mh.find(rng(gen)); }, "mh lookup ex");
#if 0
        gen.seed(seed);
        bench()([&]() { i += mha.find(rng(gen)); }, "mha lookup ex");
#endif
        gen.seed(seed);
        bench()([&]() { i += gd.find(rng(gen)) != gd.end(); }, "google lookup ex");
    }

    {
        std::uniform_int_distribution<> rng2(1e9 + 1, 2e9);

        //gen.seed(seed);
        //bench()([&]() { i += umap.find(rng2(gen)) != umap.end(); }, "umap lookup inex");

        gen.seed(seed);
        bench()([&]() { i += mh.find(rng2(gen)); }, "mh lookup inex");

#if 0
        gen.seed(seed);
        bench()([&]() { i += mha.find(rng2(gen)); }, "mha lookup inex");
#endif

        gen.seed(seed);
        bench()([&]() { i += gd.find(rng2(gen)) != gd.end(); }, "google lookup inex");
    }
}
