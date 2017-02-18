#define HT_DEBUG_IO

#include "mic.h"

#include <gtest/gtest.h>
#include <string>

TEST(ha, empty)
{
    hash_array<std::string, int> mha(empty_key("bla"), 4);
    
    for (int i = 0; i < 100; ++i)
        mha.insert(std::make_pair("foo" + std::to_string(i), 1));

}


