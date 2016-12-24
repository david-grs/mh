#pragma once

#include "typemap.h"

#include <boost/serialization/strong_typedef.hpp>

#include <memory>
#include <array>
#include <cstddef>
#include <sstream>
#include <vector>

namespace acc
{

namespace detail
{
struct lazy_acc;
}

BOOST_STRONG_TYPEDEF(int, count_t);
BOOST_STRONG_TYPEDEF(int, min_t);
BOOST_STRONG_TYPEDEF(int, max_t);
BOOST_STRONG_TYPEDEF(int, median_t);
BOOST_STRONG_TYPEDEF(int, mean_t);
BOOST_STRONG_TYPEDEF(int, stddev_t);
BOOST_STRONG_TYPEDEF(double, quantile_t);
BOOST_STRONG_TYPEDEF(std::vector<quantile_t>, quantiles_t);

struct stats
{
    stats(count_t count, min_t min, max_t max, median_t median, mean_t mean, stddev_t stddev, std::initializer_list<quantile_t>&& quantiles) :
     _stats(count, min, max, median, mean, stddev, quantiles_t(quantiles))
    {}

    template <typename T>
    const T& get() const { return _stats.get<T>(); }

    friend std::ostream& operator<<(std::ostream& oss, stats& s);

private:
    typemap<count_t, min_t, max_t, median_t, mean_t, stddev_t, quantiles_t> _stats;
};


inline std::ostream& operator<<(std::ostream& oss, stats& s)
{
    return oss;
}

struct lazy_acc
{
    lazy_acc();
    ~lazy_acc();

    void reserve(std::size_t n);

    void add(double d);
    stats process();

private:
    std::unique_ptr<detail::lazy_acc> _acc;
};

}
