#pragma once

#include <boost/operators.hpp>

#define STRONG_TYPEDEF(T, D)                                        \
    struct D                                                        \
        : boost::totally_ordered1<D                                 \
        , boost::totally_ordered2<D, T>>                            \
    {                                                               \
        using value_type = T;                                       \
        T t;                                                        \
        explicit D(const T t_) : t(t_) {}                           \
        D(): t() {}                                                 \
        D(const D & t_) : t(t_.t){}                                 \
        D & operator=(const D & rhs) { t = rhs.t; return *this;}    \
        D & operator=(const T & rhs) { t = rhs; return *this;}      \
        operator const T & () const { return t; }                   \
        operator T & () { return t; }                               \
        bool operator==(const D & rhs) const { return t == rhs.t; } \
        bool operator<(const D & rhs) const { return t < rhs.t; }   \
        static const char* name() { return # D; }                   \
    };

