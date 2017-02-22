#define _HT_DEBUG_IO
#define _HT_DEBUG

#include "ht.h"
#include "utils.h"

#include <gtest/gtest.h>
#include <iostream>
#include <unordered_map>

TEST(HashTableTest_Iterator, begin)
{
    ht<int, A> h(empty_key(-1));

    {
        auto it = h.cbegin();
        EXPECT_EQ(h.cend(), it);
    }

    {
        auto it = h.emplace(5, A(10)).first;
        EXPECT_EQ(h.cbegin(), it);
        EXPECT_EQ(h.begin(), it);
        EXPECT_EQ(5, it->first);
        EXPECT_EQ(10, it->second._i);
    }

    {
        auto it = h.cbegin();

        const std::pair<const int, A>& p = *it;
        const A& a = it->second;

        EXPECT_EQ(&p.second, &a);
        EXPECT_EQ(&p.second, &a);
    }
}

TEST(HashTableTest, emplace)
{
    ht<int, A> h(empty_key(-1));

    const int NumElements = 1000;
    for (int i = 0; i < NumElements; ++i)
        h.emplace(i, i  * 10);

    {
        std::unordered_map<int, int> m;

        for (const auto& p : h)
        {
            EXPECT_TRUE(m.find(p.first) == m.cend());
            EXPECT_EQ(p.first * 10, p.second._i);
            m[p.first] = p.second._i;
        }

        EXPECT_EQ(NumElements, (int)m.size());
    }
}

TEST(HashTableTest, copy_ctor)
{
    ht<int, A> h(empty_key(-1));
    h.emplace(1, 2);
    h.emplace(2, 3);
    h.emplace(3, 4);
    A::reset();

    ht<int, A> h2(std::move(h));

    ASSERT_EQ(3, (int)h2.size());

    for (auto&& p : h2)
        std::cout << p.first << std::endl;

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(0, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(HashTableTest, MoveConstructor)
{
    ht<int, A> h(empty_key(-1));
    h.emplace(1, 2);
    h.emplace(2, 3);
    h.emplace(3, 4);
    A::reset();

    ht<int, A> h2(std::move(h));

    ASSERT_EQ(3, (int)h2.size());

    for (auto&& p : h2)
        std::cout << p.first << std::endl;

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(0, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(HashTableTest, InsertRValue_ValueMoveCount)
{
    ht<int, A> h(empty_key(-1));
    A::reset();
    h.insert(std::make_pair(5, A()));

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(2, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(HashTableTest, InsertMoved_ValueMoveCount)
{
    ht<int, A> h(empty_key(-1));
    auto p = std::make_pair(5, A());

    A::reset();
    h.insert(std::move(p));

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(1, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(HashTableTest, Emplace_ValueMoveCount)
{
    ht<int, A> h(empty_key(-1));

    A::reset();
    h.emplace(1, 2);

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(0, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(HashTableTest, InsertRValue_KeyMoveCount)
{
    ht<A, int, HashA> h(empty_key(A(-1)));

    A::reset();
    h.insert(std::make_pair(A(2), 2));

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(2, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(HashTableTest, InsertMoved_KeyMoveCount)
{
    ht<A, int, HashA> h(empty_key(A(-1)));

    auto m = std::make_pair(A(2), 2);
    A::reset();
    h.insert(std::move(m));

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(1, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(HashTableTest, Emplace_KeyMoveCount)
{
    ht<A, int, HashA> h(empty_key(A(-1)));

    A::reset();
    h.emplace(A(1), 2);

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(0, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(HashTableTest, OperatorSqBck_InsertRValue_KeyMoveCount)
{
    ht<A, int, HashA> h(empty_key(A(-1)));

    A::reset();
    h[A(1)] = 1;

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(1, A::move_ctor);
    ASSERT_EQ(0, A::move_assign);
}

TEST(HashTableTest, OperatorSqBck_InsertMoved_ValueMoveCount)
{
    ht<int, A> h(empty_key(-1));

    A::reset();
    h[1] = A(1);

    ASSERT_EQ(0, A::copy_ctor);
    ASSERT_EQ(0, A::copy_assign);
    ASSERT_EQ(1, A::move_ctor);
    ASSERT_EQ(1, A::move_assign);
}

