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
            //pos = (pos + num_probes++) & (_table_sz - 1);

            //pos = (pos + 1) & (_table_sz - 1);
            //++num_probes;

            pos = (pos + (1ULL << num_probes) + 1) & (_table_sz - 1);
            ++num_probes;

            DEBUG(" - not empty, trying pos=" << pos);
            assert(num_probes < _table_sz);
        }

        max_num_probes = std::max(max_num_probes, (int)num_probes);

        insert_element(pos, std::forward<Pair>(p));
        return {{}, true};
    }

    int max_num_probes = 0;

    Value& operator[](const Key& key)
    {
        std::size_t pos = Hash()(key) & (_table_sz - 1);
        std::size_t num_probes = 1;

        DEBUG("lookup at pos=" << pos);

        while (!Equal()(_table[pos].first, key))
        {
            if (Equal()(_table[pos].first, EmptyKey::value))
            {
                resize(1);

                DEBUG("empty node, inserting value at pos=" << pos);
                insert_element(pos, std::make_pair(key, Value{}));
                break;
            }

            DEBUG("other value at pos=" << pos << ", continuing");

            pos = (pos + num_probes++) & (_table_sz - 1);
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

    void resize(std::size_t n)
    {
        if (_elements + n > _table_sz / 2)
        {
            DEBUG("resizing to " << _table_sz * 2 << "...")

            ht h(_table_sz * 2);
            std::for_each(&_table[0], &_table[_table_sz], [&h](auto&& p)
            {
                if (!Equal()(p.first, EmptyKey::value))
                    h.insert(p);
            });
            std::swap(*this, h);

            DEBUG("resized.")
        }
    }
    std::size_t _elements;
    std::size_t _table_sz;
    std::unique_ptr<Node[]> _table;
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

    {
        std::uniform_int_distribution<> rng(1, 1e6);

        benchmark([&]() { umap.insert(std::make_pair(rng(gen), 222.0)); }, "umap insert");

        gen.seed(*seed);
        benchmark([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); }, "mh insert");
    }

    std::cout << mh.max_num_probes << std::endl;
  //  std::exit(1);

    {
        std::uniform_int_distribution<> rng(1, std::min(umap.size(), mh.size()) - 1);

        benchmark([&]() { umap[rng(gen)] = 123; }, "umap operator[]");

        gen.seed(*seed);
        benchmark([&]() { mh[rng(gen)] = 123; }, "mh operator[]");
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
