//#define HT_DEBUG_IO

#include "mic.h"

#include <gtest/gtest.h>
#include <string>

struct test_ha : public ::testing::Test
{
    explicit test_ha() :
        ha(empty_key("bla"), 2)
    {}

    using ha_type = hash_array<std::string, int>;
    ha_type ha;
};

TEST_F(test_ha, empty)
{
    EXPECT_TRUE(ha.empty());

    ha.insert(std::make_pair("foo", 1));
    EXPECT_FALSE(ha.empty());

    ha = ha_type(empty_key("bla"), 4);
    EXPECT_TRUE(ha.empty());
}

TEST_F(test_ha, size)
{
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i, (int)ha.size());
        ha.insert(std::make_pair("foo" + std::to_string(i), 1));
    }
}

