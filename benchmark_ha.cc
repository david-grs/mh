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

#include <cstdint>

using namespace boost::multi_index;

void benchmark_ha(long unsigned seed)
{
    std::mt19937 gen(seed);

    ht<int32_t, double, empty_key<int32_t, 0>> mh;
    hash_array<int32_t, double, empty_key<int32_t, 0>> mha;

    boost::multi_index_container<
      std::pair<int32_t, double>,
      indexed_by<
        hashed_unique<
          member<std::pair<int32_t, double>, int32_t, &std::pair<int32_t, double>::first>
        >,
        sequenced<>
      >
    > mic_hs;

    std::uniform_int_distribution<> rng(1, 1e9);

    benchmark bench;
    bench.tear_down([&](const stats& s, const char* desc)
    {
        std::cout << desc << "," << seed << "," << s << std::endl;
        gen.seed(seed);
    });

    {
        bench([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); }, "ht insert");
        bench([&]() { mha.insert(std::make_pair(rng(gen), 222.0)); }, "mha insert");
        bench([&]() { mic_hs.insert(std::make_pair(rng(gen), 222.0)); }, "mic insert");
    }
}
