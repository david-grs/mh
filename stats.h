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
BOOST_STRONG_TYPEDEF(double, quantile_prob_t);
BOOST_STRONG_TYPEDEF(double, quantile_value_t);

using quantile_t = std::pair<quantile_prob_t, quantile_value_t>;
using quantiles_t = std::vector<quantile_t>;

struct stats
{
    stats(count_t count, min_t min, max_t max, median_t median, mean_t mean, stddev_t stddev, std::initializer_list<quantile_t>&& quantiles) :
     _stats(count, min, max, median, mean, stddev, quantiles)
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
    lazy_acc(std::size_t reserve) :
      _samples(reserve, -1.0),
      _size(0)
    {}

    void add(double d)
    {
        _samples[_size++] = d;
    }

    stats process();

private:
    std::vector<double> _samples;
    std::size_t _size;
};

}
