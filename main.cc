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
    using Node = std::pair<Key, Value>;

    ht() :
      _elements(0),
      _max_elements(16),
      _table(std::make_unique<Node[]>(_max_elements))
    {
    }

    void insert(Node& n)
    {
        std::size_t pos = Hash()(n.first) & (_table.size() - 1);
        std::size_t num_probes = 1;

        while (!Equal()(_table[pos].first, EmptyKey::value))
            pos = (pos + num_probes++) & (_table.size() - 1);

        new (&_table[pos]) Node(n);
    }

    std::size_t size() const { return _elements; }
    std::size_t capacity() const { return _max_elements; }

private:
    std::size_t _elements;
    std::size_t _max_elements;
    std::unique_ptr<Node[]> _table;
};

int main()
{

    ht<int, double, empty_key<int, 0>> h;

    assert(h.capacity() == 10);
    assert(h.size() == 0);
}
