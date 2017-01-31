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

    return bench.tests();
}

}

std::vector<test> benchmark_boost_mic(long unsigned seed)
{
    namespace mic = boost::multi_index;

    mic::multi_index_container<
      std::pair<int32_t, double>,
      mic::indexed_by<
        mic::hashed_unique<
          mic::member<std::pair<int32_t, double>, int32_t, &std::pair<int32_t, double>::first>
        >,
        mic::sequenced<>
      >
    > mic_hs;

    return ::detail::benchmark_container("boost.mic", mic_hs, seed);
}

std::vector<test> benchmark_ha(long unsigned seed)
{
    hash_array<int32_t, double, empty_key<int32_t, 0>> mha;

    return ::detail::benchmark_container("ha", mha, seed);
}

