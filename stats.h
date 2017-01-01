#pragma once

#include "typemap.h"

#include <boost/serialization/strong_typedef.hpp>

#include <memory>
#include <array>
#include <sstream>
#include <vector>

#include <cstddef>
#include <cassert>

BOOST_STRONG_TYPEDEF(int64_t, sum_t);
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
using stats = typemap<sum_t, count_t, min_t, max_t, median_t, mean_t, stddev_t, quantiles_t>;

inline std::ostream& operator<<(std::ostream& os, quantiles_t& quantiles)
{
    for(const quantile_t& q : quantiles)
        os << q.first << " " << q.second << " ";
    return os;
}

inline std::istream& operator>>(std::istream& is, quantiles_t& quantiles)
{
    double d1, d2;
    while (is >> d1 >> d2)
        quantiles.emplace_back(d1, d1);
    return is;
}

struct lazy_acc
{
    lazy_acc(std::size_t reserve) :
      _samples(reserve, -1),
      _size(0)
    {}

    void add(int64_t x)
    {
        assert(_size < _samples.size());
        _samples[_size++] = x;
    }

    void clear() { _size = 0; }
    std::vector<int64_t>& data() { return _samples; }

    stats process();

private:
    std::vector<int64_t> _samples;
    std::size_t _size;
};
