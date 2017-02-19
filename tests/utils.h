#pragma once

#include <sstream>

struct A
{
    explicit A(int i) noexcept : _i(i) {}
    A() =default;

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

inline std::ostream& operator<<(std::ostream& os, const A& a)
{
    return os << a._i << " copy_ctor=" << a.copy_ctor << " copy_assign=" << a.copy_assign <<
                         " move_ctor=" << a.move_ctor << " move_assign=" << a.move_assign;
}
