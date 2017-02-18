#include <gtest/gtest.h>

#include "ht.h"

TEST(ht, copy_ctor)
{
    ht<int, int> h(empty_key(0));

    auto h2 = h;
}


