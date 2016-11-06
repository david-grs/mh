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
    double min() const;
    double max() const;
    std::size_t count() const;

private:
    std::unique_ptr<impl::stats> _stats;

    friend std::ostream& operator<<(std::ostream& oss, stats& s);
};

