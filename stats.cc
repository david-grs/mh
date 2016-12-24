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

#include <algorithm>

template <typename T, typename... Ts>
constexpr auto make_array(T t, Ts... ts)
{
    return std::array<T, sizeof...(Ts) + 1>{t, ts...};
}

namespace acc
{

static constexpr auto quantiles =
    make_array(0.001, 0.01, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5,
               0.55, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85, 0.90, 0.95, 0.99, 0.999);

template <typename AccumulatorT, typename QuantilesT, std::size_t... Is>
stats get_stats(AccumulatorT&& accum, QuantilesT&& quantiles, std::index_sequence<Is...>)
{
    namespace boost_acc = boost::accumulators;
    return {
        count_t(boost_acc::count(accum)),
        min_t(boost_acc::min(accum)),
        max_t(boost_acc::max(accum)),
        median_t(boost_acc::median(accum)),
        mean_t(boost_acc::mean(accum)),
        stddev_t(std::sqrt(boost_acc::variance(accum))),
        {quantile_t(quantiles[Is], boost_acc::quantile(accum, boost_acc::quantile_probability = quantiles[Is]))...}
    };
}

stats lazy_acc::process()
{
    namespace boost_acc = boost::accumulators;
    using Acc = boost_acc::accumulator_set<double,
                                            boost_acc::stats<
                                                boost_acc::tag::median,
                                                boost_acc::tag::mean,
                                                boost_acc::tag::min,
                                                boost_acc::tag::max,
                                                boost_acc::tag::count,
                                                boost_acc::tag::variance,
                                                boost_acc::tag::extended_p_square_quantile>>;

    Acc accum(boost_acc::extended_p_square_probabilities = quantiles);
    std::for_each(std::cbegin(_samples), std::cend(_samples), accum);

    return get_stats(accum, quantiles, std::make_index_sequence<quantiles.size()>());
}

}
