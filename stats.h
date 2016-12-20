#pragma once

#include <memory>
#include <array>
#include <cstddef>
#include <sstream>

#include <boost/serialization/strong_typedef.hpp>

namespace stats
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

struct samples
{
    samples(count_t count, min_t min, max_t max, median_t median, mean_t mean, stddev_t stddev) :
     _count(count), _min(min), _max(max), _median(median), _mean(mean), _stddev(stddev)
     {}

    double percentile(double p) const;
    double median() const;
    double mean() const;
    double min() const;
    double max() const;
    double stddev() const;
    std::size_t count() const;

    static constexpr const std::array<double, 24> percentiles =
        {{0.001, 0.01, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5,
           0.55, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85, 0.90, 0.95, 0.99, 0.999}};

    friend std::ostream& operator<<(std::ostream& oss, samples& s);

private:
    count_t _count;
    min_t _min;
    max_t _max;
    median_t _median;
    mean_t _mean;
    stddev_t _stddev;
};


inline std::ostream& operator<<(std::ostream& oss, samples& s)
{
    oss << "count: " << s.count() << " stddev: " << s.stddev() << " - min: " << s.min() << " - ";
    for (double d : samples::percentiles)
        oss << d << "%: " << s.percentile(d) << " - ";
    return oss << "max: " << s.max();
}

struct lazy_acc
{
    lazy_acc();
    ~lazy_acc();

    void reserve(std::size_t n);

    void add(double d);
    samples process();

private:
    std::unique_ptr<detail::lazy_acc> _acc;
};

}
