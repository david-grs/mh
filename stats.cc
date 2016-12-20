#include "stats.h"

#include <algorithm>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/extended_p_square_quantile.hpp>

namespace stats { namespace detail {

namespace acc = boost::accumulators;

struct lazy_acc
{
    void add(double d) { _points.push_back(d); }

    samples process()
    {
        using Acc = acc::accumulator_set<double,
                                        acc::stats<acc::tag::median,
                                                    acc::tag::mean,
                                                    acc::tag::min,
                                                    acc::tag::max,
                                                    acc::tag::count,
                                                    acc::tag::variance,
                                                    acc::tag::extended_p_square_quantile>>;

        Acc acc(acc::extended_p_square_probabilities = samples::percentiles);
        std::for_each(std::cbegin(_points), std::cend(_points), acc);

        return {};
    }

    void reserve(std::size_t n)
    {
        _points.reserve(n);
    }
#if 0
    double percentile(double p)  { return acc::quantile(_acc, acc::quantile_probability = p); }
    double median() const { return acc::median(_acc); }
    double mean() const { return acc::mean(_acc); }
    double min() const { return acc::min(_acc); }
    double max() const { return acc::max(_acc); }
    double stddev() const { return std::sqrt(acc::variance(_acc)); }
    std::size_t count() const { return acc::count(_acc); }
#endif

private:
    std::vector<double> _points;
};

}

constexpr const std::array<double, 24> samples::percentiles;

lazy_acc::lazy_acc() :
  _acc(std::make_unique<detail::lazy_acc>())
{}

lazy_acc::~lazy_acc()
{}

void lazy_acc::add(double d) { _acc->add(d); }
void lazy_acc::reserve(std::size_t n) { _acc->reserve(n); }
samples lazy_acc::process() { return _acc->process(); }

}

