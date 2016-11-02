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

#if defined NDEBUG
#define DEBUG(x)
#else
#define DEBUG(x) std::cout << x << std::endl;
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
            DEBUG("position pos=" << pos << " not empty");

            pos = (pos + num_probes++) & (_table_sz - 1);
            DEBUG("trying to insert at pos=" << pos << " probes" << num_probes);
            assert(num_probes < _table_sz);
        }

        insert_element(pos, std::forward<Pair>(p));
        return {{}, true};
    }

    Value& operator[](const Key& key)
    {
        std::size_t pos = Hash()(key) & (_table_sz - 1);
        std::size_t num_probes = 1;

        while (!Equal()(_table[pos].first, key))
        {
            if (Equal()(_table[pos].first, EmptyKey::value))
            {
                insert_element(pos, std::make_pair(key, Value{}));
                break;
            }

            pos = (pos + num_probes++) & (_table_sz - 1);
            assert(num_probes < _table_sz);
        }

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
            ht h(_table_sz * 2);
            std::for_each(&_table[0], &_table[_table_sz], [&h](auto&& p)
            {
                if (!Equal()(p.first, EmptyKey::value))
                    h.insert(p);
            });
            std::swap(*this, h);

            DEBUG("resizing to " << _table_sz * 2)
        }
    }
    std::size_t _elements;
    std::size_t _table_sz;
    std::unique_ptr<Node[]> _table;
};


void benchmark()
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
    auto seed = rd();

    std::mt19937 gen(seed);
    std::uniform_int_distribution<> rng(0, 1e6);

    std::unordered_map<int, double> umap;
    ht<int, double, empty_key<int, 0>> mh;

    benchmark([&]() { umap.insert(std::make_pair(rng(gen), 222.0)); }, "umap");

    gen.seed(seed);
    benchmark([&]() { mh.insert(std::make_pair(rng(gen), 222.0)); }, "mh");
}

int main()
{

    ht<int, double, empty_key<int, 0>> h;

    assert(h.capacity() == 16);
    assert(h.size() == 0);

    auto p = std::make_pair(1, 5.0);
    auto ok = h.insert(p).second;

    assert(ok);
    (void)ok;

    h.insert(std::make_pair(1, 4));

    double& d = h[1];

    benchmark();

    return 0;
}
