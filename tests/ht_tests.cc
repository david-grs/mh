#include "ht.h"
#include "utils.h"

#include <gtest/gtest.h>
#include <iostream>

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

