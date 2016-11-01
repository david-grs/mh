#include <algorithm>
#include <type_traits>
#include <vector>
#include <functional>
#include <memory>
#include <cassert>

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

    std::pair<iterator, bool> insert(Node& n)
    {
        resize(1);

        assert(!Equal()(n.first, EmptyKey::value));

        std::size_t pos = Hash()(n.first) & (_table_sz - 1);
        std::size_t num_probes = 1;

        while (!Equal()(_table[pos].first, EmptyKey::value))
        {
            pos = (pos + num_probes++) & (_table_sz - 1);
            assert(num_probes < _table_sz);
        }

        new (&_table[pos]) Node(n);
        return {{}, true};
    }

    std::size_t size() const { return _elements; }
    std::size_t capacity() const { return _table_sz; }

private:
    void resize(std::size_t n)
    {
        if (_elements + n > _table_sz / 2)
        {
            ht h(_table_sz * 2);
            std::for_each(&_table[0], &_table[_table_sz], [&h](auto&& p) { h.insert(p); });
            std::swap(*this, h);
        }
    }
    std::size_t _elements;
    std::size_t _table_sz;
    std::unique_ptr<Node[]> _table;
};

int main()
{

    ht<int, double, empty_key<int, 0>> h;

    assert(h.capacity() == 16);
    assert(h.size() == 0);

    auto p = std::make_pair(1, 5.0);
    auto ok = h.insert(p).second;

    assert(ok);

}
