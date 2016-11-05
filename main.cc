#include <algorithm>
#include <type_traits>
#include <vector>
#include <functional>
#include <memory>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <random>
#include <chrono>

#include <boost/optional.hpp>
#include <google/dense_hash_map>

#if defined NDEBUG
#define DEBUG(x)
#else
#define DEBUG(x)
//#define DEBUG(x) std::cout << x << std::endl;
#endif

template <typename T, T t>
struct empty_key
{
    enum { value = t };
};

template <typename Key, typename Value, typename EmptyKey, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>>
struct ht
{
    using Container = ht<Key, Value, EmptyKey, Hash, Equal>;
    using Node = std::pair<Key, Value>;
    using iterator = void*; // TODO

    ht(std::size_t capacity = 16) :
      _elements(0),
      _table_sz(capacity),
      _table(std::make_unique<Node[]>(_table_sz))
    {
    }

    std::size_t next_quadratic(std::size_t pos, std::size_t& num_probes) const
    {
        return (pos + (1ULL << num_probes++) + 1) & (_table_sz - 1);
    }

    std::size_t next_quadratic_2(std::size_t pos, std::size_t& num_probes) const
    {
        return (pos + num_probes++) & (_table_sz - 1);
    }

    std::size_t next_linear(std::size_t pos, std::size_t& num_probes) const
    {
        ++num_probes;
        return (pos + 1) & (_table_sz - 1);
    }

    template <typename... Args> auto next(Args&&... args) { return next_quadratic_2(std::forward<Args>(args)...); }

    template <typename Pair>
    std::pair<iterator, bool> insert(Pair&& p)
    {
        resize(1);

        DEBUG("inserting " << p.first);
        assert(!Equal()(p.first, EmptyKey::value));

        std::size_t pos = Hash()(p.first) & (_table_sz - 1);
        std::size_t num_probes = 1;

        DEBUG("trying to insert at pos=" << pos);

        while (!Equal()(_table[pos].first, EmptyKey::value))
        {
            if (Equal()(_table[pos].first, p.first))
                return {{}, false};

            pos = next(pos, num_probes);

            DEBUG(" - not empty, trying pos=" << pos);
            assert(num_probes < _table_sz);
        }

        _max_num_probes = std::max(_max_num_probes, num_probes);
        insert_element(pos, std::forward<Pair>(p));
        return {{}, true};
    }

    bool find(const Key& key)
    {
        std::size_t pos = Hash()(key) & (_table_sz - 1);
        std::size_t num_probes = 1;

        DEBUG("lookup at pos=" << pos);

        while (!Equal()(_table[pos].first, key))
        {
            if (Equal()(_table[pos].first, EmptyKey::value))
                return false;

            pos = next(pos, num_probes);
            assert(num_probes < _table_sz);
        }

        return true;
    }

    Value& operator[](const Key& key)
    {
        std::size_t pos = Hash()(key) & (_table_sz - 1);
        std::size_t num_probes = 1;

        DEBUG("lookup at pos=" << pos);

        while (!Equal()(_table[pos].first, key))
        {
            if (Equal()(_table[pos].first, EmptyKey::value))
            {
                const bool resized = resize(1);

                if (resized)
                {
                    return operator[](key);
                }
                else
                {
                    DEBUG("empty node, inserting value at pos=" << pos);
                    insert_element(pos, std::make_pair(key, Value{}));
                }

                break;
            }

            DEBUG("other value at pos=" << pos << ", continuing");

            pos = next(pos, num_probes);
            assert(num_probes < _table_sz);
        }

        DEBUG("found existing key at pos=" << pos);

        return _table[pos].second;
    }

    std::size_t size() const { return _elements; }
    std::size_t capacity() const { return _table_sz; }

private:
    template <typename Pair>
    void insert_element(std::size_t pos, Pair&& p)
    {
        new (&_table[pos]) Node(std::forward<Pair>(p));
        ++_elements;
    }

    bool resize(std::size_t n)
    {
        if (_elements + n > _table_sz / 2)
        {
            DEBUG("resizing to " << _table_sz * 2  << "...")

            ht h(_table_sz * 2);
            std::for_each(&_table[0], &_table[_table_sz], [&h](auto&& p)
            {
                if (!Equal()(p.first, EmptyKey::value))
                    h.insert(p);
            });

            std::swap(*this, h);
            _max_num_probes = h._max_num_probes;

            DEBUG("resized.")
            return true;
        }

        return false;
    }
    std::size_t _elements;
    std::size_t _table_sz;
    std::unique_ptr<Node[]> _table;

public:
    std::size_t _max_num_probes = 0;
};


void benchmark(boost::optional<long unsigned> seed = boost::none)
{
    auto benchmark = [](auto&& operation, const char* desc)
    {
        static const int Iterations = 1e6;

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < Iterations; ++i)
            operation();
        auto end = std::chrono::steady_clock::now();

        std::cout << desc << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    };


    std::random_device rd;
    if (!seed)
        seed = rd();

    std::mt19937 gen(*seed);
    std::cout << "seed = " << *seed << std::endl;

    std::unordered_map<int, double> umap;
    ht<int, double, empty_key<int, 0>> mh;
    google::dense_hash_map<int, double> gd;
    gd.set_empty_key(0);

    std::uniform_int_distribution<> rng(1, 1e6);

    {
        gen.seed(*seed);
        benchmark([&]() { umap.insert(std::make_pair(rng(gen), 222.0)); }, "umap insert");

        gen.seed(*seed);
        benchmark([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); }, "mh insert");

        gen.seed(*seed);
        benchmark([&]() { gd.insert(std::make_pair(rng(gen), 222.0)); }, "google insert");
    }

    std::cout << mh._max_num_probes << std::endl;

    volatile int i = 0;

    {
        //std::uniform_int_distribution<> rng(1, std::min(umap.size(), mh.size()) - 1);

        gen.seed(*seed);
        benchmark([&]() { i += umap.find(rng(gen)) != umap.end(); }, "umap lookup ex");

        gen.seed(*seed);
        benchmark([&]() { i += mh.find(rng(gen)); }, "mh lookup ex");

        gen.seed(*seed);
        benchmark([&]() { i += gd.find(rng(gen)) != gd.end(); }, "google lookup ex");
    }

    {
        std::uniform_int_distribution<> rng2(1e6 + 1, 2e9);

        gen.seed(*seed);
        benchmark([&]() { i += umap.find(rng2(gen)) != umap.end(); }, "umap lookup inex");

        gen.seed(*seed);
        benchmark([&]() { i += mh.find(rng2(gen)); }, "mh lookup inex");

        gen.seed(*seed);
        benchmark([&]() { i += gd.find(rng2(gen)) != gd.end(); }, "google lookup inex");
    }
}

int main(int argc, char** argv)
{

    ht<int, double, empty_key<int, 0>> h;

    assert(h.capacity() == 16);
    assert(h.size() == 0);

    double& d = h[1];
    d = 123.0;

    h[17] = 2;
    h[16*2 + 1] = 2;
    h[16*3 + 1] = 2;
    h[16*4 + 1] = 2;
    h[16*5 + 1] = 2;
    h[16*6 + 1] = 2;
    h[16*7 + 1] = 2;
    h[16*8 + 1] = 2;
    h[16*9 + 1] = 2;
    h[16*10 + 1] = 2;

    if (argc == 2)
        benchmark(std::atoll(argv[1]));
    else
        benchmark();

    return 0;
}
