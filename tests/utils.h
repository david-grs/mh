#pragma once

#include <sstream>
#include <random>
#include <functional>

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

inline bool operator==(const A& a1, const A& a2) { return a1._i == a2._i; }

struct HashA
{
    std::size_t operator()(const A& a) const { return std::hash<int>()(a._i); }
};


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
