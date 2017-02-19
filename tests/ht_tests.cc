#include "ht.h"
#include "utils.h"

#include <gtest/gtest.h>
#include <iostream>

TEST(ht, copy_ctor)
{
    ht<int, A> h(empty_key(0));

    std::cout << A() << std::endl;
    h.insert(std::make_pair(1,1));
    std::cout << A() << std::endl;

    A::reset();
    auto h2 = h;
    std::cout << "copy = " << A() << std::endl;

    A::reset();
    auto h3 = std::move(h);
    std::cout << "move = " << A() << std::endl;
}
