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

using namespace boost::multi_index;

void benchmark_ha(long unsigned seed)
{
    std::cout << "============================\n";
    std::cout << "hash_array benchmark\n";
    std::cout << "============================" << std::endl;

    geiger::init();

    std::mt19937 gen(seed);

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

    std::uniform_int_distribution<> rng(1, 1e9);
#if 0
    {
        gen.seed(seed);
        bench_stats()([&]() { mh2.insert(std::make_pair(rng(gen), 222.0)); }, "mh insert");

        gen.seed(seed);
        bench_stats()([&]() { mha2.insert(std::make_pair(rng(gen), 222.0)); }, "mha insert");

        gen.seed(seed);
        bench_stats()([&]() { mic_hs2.insert(std::make_pair(rng(gen), 222.0)); }, "mic insert");
    }
#endif

    {
        gen.seed(seed);
        bench()([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); }, "ht insert");

        gen.seed(seed);
        bench()([&]() { mha.insert(std::make_pair(rng(gen), 222.0)); }, "mha insert");

        gen.seed(seed);
        bench()([&]() { mic_hs.insert(std::make_pair(rng(gen), 222.0)); }, "mic insert");
    }
}
