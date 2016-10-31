#include <algorithm>
#include <string>
#include <type_traits>
#include <vector>

#include <cassert>

template <typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>>
struct ht
{
    using pointer = std::add_pointer<Value>;

    ht() :
      _elements(0),
      _table(10)
    {
    }

    std::size_t size() const { return _elements; }
    std::size_t capacity() const { return _table.size(); }

private:
    std::size_t _elements;
    std::vector<Value> _table;
};

int main()
{

    ht<int, std::string> h;

    assert(h.capacity() == 10);
    assert(h.size() == 0);
}
