#pragma once

#include <cstddef>
#include <memory>

namespace impl
{
struct stats;
}

struct stats
{
    stats();
    ~stats();

    void add(double d);

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

private:
    std::unique_ptr<impl::stats> _stats;

    friend std::ostream& operator<<(std::ostream& oss, stats& s);
};

