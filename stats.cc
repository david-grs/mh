#include "stats.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/extended_p_square_quantile.hpp>

namespace impl
{

namespace acc = boost::accumulators;

struct stats
{
    stats() :
      _acc(acc::extended_p_square_probabilities = percentiles)
    {}

    void add(double d) { _acc(d); }

    double percentile(double p)  { return acc::quantile(_acc, acc::quantile_probability = p); }
    double median() const { return acc::median(_acc); }
    double mean() const { return acc::mean(_acc); }
    double min() const { return acc::min(_acc); }
    double max() const { return acc::max(_acc); }
    double stddev() const { return std::sqrt(acc::variance(_acc)); }
    std::size_t count() const { return acc::count(_acc); }

    static constexpr const std::array<double, 24> percentiles = {{0.001, 0.01, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5,
                                                                   0.55, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85, 0.90, 0.95, 0.99, 0.999}};

private:
    using Acc = acc::accumulator_set<double,
                                     acc::stats<acc::tag::median,
                                                acc::tag::mean,
                                                acc::tag::min,
                                                acc::tag::max,
                                                acc::tag::count,
                                                acc::tag::variance,
                                                acc::tag::extended_p_square_quantile>>;
    Acc _acc;
};

constexpr const std::array<double, 24> stats::percentiles;

}

inline std::ostream& operator<<(std::ostream& oss, impl::stats& s)
{
    oss << "count: " << s.count() << " stddev: " << s.stddev() << " - min: " << s.min() << " - ";
    for (double d : impl::stats::percentiles)
        oss << d << "%: " << s.percentile(d) << " - ";
    return oss << "max: " << s.max();
}

stats::stats() :
  _stats(std::make_unique<impl::stats>())
{}

stats::~stats()
{}

void stats::add(double d) { _stats->add(d); }
double stats::percentile(double p) const { return _stats->percentile(p); }
double stats::median() const { return _stats->median(); }
double stats::mean() const { return _stats->mean(); }
double stats::min() const { return _stats->min(); }
double stats::max() const  { return _stats->max(); }
double stats::stddev() const  { return _stats->stddev(); }
std::size_t stats::count() const  { return _stats->count(); }

std::ostream& operator<<(std::ostream& oss, stats& s)
{
    return oss << *s._stats;
}
