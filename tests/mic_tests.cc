//#define HT_DEBUG_IO

#include "mic.h"

#include <gtest/gtest.h>
#include <string>

template <typename Key>
struct TestHashArray : public ::testing::Test
{
    explicit TestHashArray() :
        ha(empty_key("bla"), 2)
    {}

    using ha_type = hash_array<std::string, int>;
    ha_type ha;
};

TYPED_TEST_CASE_P(TestHashArray);

TYPED_TEST_P(TestHashArray, empty)
{
    EXPECT_TRUE(this->ha.empty());

    this->ha.insert(std::make_pair("foo", 1));
    EXPECT_FALSE(this->ha.empty());

    this->ha = typename TestHashArray<TypeParam>::ha_type(empty_key("bla"), 4);
    EXPECT_TRUE(this->ha.empty());
}

TYPED_TEST_P(TestHashArray, size)
{
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i, (int)this->ha.size());
        this->ha.insert(std::make_pair("foo" + std::to_string(i), 1));
    }
}

TYPED_TEST_P(TestHashArray, walk)
{
    int i = 0;
    for (const auto& p : this->ha)
        std::cout << i++ << ": " << p.first << std::endl;
}

using KeyTypes = testing::Types<std::string, int, double>;

REGISTER_TYPED_TEST_CASE_P(TestHashArray, empty, size, walk);
INSTANTIATE_TYPED_TEST_CASE_P(KeyTypes, TestHashArray, KeyTypes);

