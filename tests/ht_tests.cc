#include <gtest/gtest.h>

#include "ht.h"

#include <iostream>

struct A
{
    A() =default;
    A(int i) noexcept : _i(i) {}

    A(const A& a) { _i = a._i; ++copy_ctor;; }
    A& operator=(const A& a) { _i = a._i; ++copy_assign; return *this; }

    A(A&& a) { _i = a._i; ++move_ctor; }
    A& operator=(A&& a) { _i = a._i; ++move_assign; return *this; }

    static void reset()
    {
        copy_ctor = 0;
        copy_assign = 0;
        move_ctor = 0;
        move_assign = 0;
    }

    int _i = 0;

    static int copy_ctor;
    static int copy_assign;
    static int move_ctor;
    static int move_assign;
};

int A::copy_ctor = 0;
int A::copy_assign = 0;
int A::move_ctor = 0;
int A::move_assign = 0;


std::ostream& operator<<(std::ostream& os, const A& a)
{
    return os << a._i << " copy_ctor=" << a.copy_ctor << " copy_assign=" << a.copy_assign <<
                         " move_ctor=" << a.move_ctor << " move_assign=" << a.move_assign;
}


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


