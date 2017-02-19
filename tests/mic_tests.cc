//#define HT_DEBUG_IO

#include "mic.h"

#include <gtest/gtest.h>

#include <string>
#include <cmath>
#include <random>
#include <type_traits>

template <typename K> auto EmptyKey();
template <> auto EmptyKey<std::string>() { return empty_key_t<std::string>("bla"); }
template <> auto EmptyKey<int>() { return empty_key_t<int>(-1); }
template <> auto EmptyKey<double>() { return empty_key_t<double>(-1.0); }

template <typename N>
struct Rand
{
    explicit Rand() :
      _gen(_rd())
      {}

    N operator()() { return _rng(_gen); }

private:
    using rng_t = std::conditional_t<std::is_floating_point<N>::value,
                     std::uniform_real_distribution<N>,
                     std::uniform_int_distribution<N>>;

    std::random_device _rd;
    std::mt19937 _gen;
    rng_t _rng;
};

template <>
struct Rand<std::string>
{
    std::string operator()() { return "foo_" + std::to_string(_rand()); }

private:
    Rand<int> _rand;
};

template <typename Key>
struct TestHashArray : public ::testing::Test
{
    explicit TestHashArray() :
        ha(EmptyKey<Key>(), 4)
    {}

    Key next_key() { return gen(); }

    using ha_type = hash_array<Key, int>;
    ha_type ha;

private:
    Rand<Key> gen;
};

TYPED_TEST_CASE_P(TestHashArray);

TYPED_TEST_P(TestHashArray, empty_init)
{
    EXPECT_TRUE(this->ha.empty());
}

TYPED_TEST_P(TestHashArray, empty_insert)
{
    this->ha.insert(std::make_pair(this->next_key(), 1));
    EXPECT_FALSE(this->ha.empty());
}

TYPED_TEST_P(TestHashArray, empty_copy)
{
    this->ha.insert(std::make_pair(this->next_key(), 1));

    auto h = this->ha;
    EXPECT_FALSE(h.empty());
}

TYPED_TEST_P(TestHashArray, size)
{
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i, (int)this->ha.size());
        this->ha.insert(std::make_pair(this->next_key(), 1));
    }
}

TYPED_TEST_P(TestHashArray, walk)
{
    for (int i = 0; i < 100; ++i)
        this->ha.insert(std::make_pair(this->next_key(), i));

    int i = 0;
    for (const auto& p : this->ha)
        ASSERT_EQ(i++, p.second);
}

using KeyTypes = testing::Types<std::string, int, double>;

REGISTER_TYPED_TEST_CASE_P(TestHashArray, empty_init, empty_insert, empty_copy, size, walk);
INSTANTIATE_TYPED_TEST_CASE_P(KeyTypes, TestHashArray, KeyTypes);

