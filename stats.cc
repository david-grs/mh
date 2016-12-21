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

struct lazy_acc
{
    void add(double d) { _points.push_back(d); }

    void reserve(std::size_t n)
    {
        _points.reserve(n);
    }

    samples process()
    {
        namespace acc = boost::accumulators;
        using Acc = acc::accumulator_set<double,
                                        acc::stats<acc::tag::median,
                                                   acc::tag::mean,
                                                   acc::tag::min,
                                                   acc::tag::max,
                                                   acc::tag::count,
                                                   acc::tag::variance,
                                                   acc::tag::extended_p_square_quantile>>;

        Acc accum(acc::extended_p_square_probabilities = samples::percentiles);
        std::for_each(std::cbegin(_points), std::cend(_points), accum);

        return get_samples(accum, samples::percentiles);
    }

private:
    template <typename AccumulatorT, typename PercentilesT, std::size_t... Is>
    samples get_samples(AccumulatorT&& accum, PercentilesT&& percentiles, std::index_sequence<Is...>)
    {
        namespace acc = boost::accumulators;
        return {count_t(acc::count(accum)), min_t(acc::min(accum)), max_t(acc::max(accum)),
                median_t(acc::median(accum)), mean_t(acc::mean(accum)), stddev_t(std::sqrt(acc::variance(accum))),
                {quantile_t(acc::quantile(accum, acc::quantile_probability = percentiles[Is]))...}};
    }

    template <typename AccumulatorT, std::size_t N>
    samples get_samples(AccumulatorT&& accum, const std::array<double, N>& percentiles)
    {
        return get_samples(std::forward<AccumulatorT>(accum), percentiles, std::make_index_sequence<N>());
    }

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
