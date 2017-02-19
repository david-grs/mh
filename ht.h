#pragma once

#include <algorithm>
#include <type_traits>
#include <functional>
#include <memory>
#include <cassert>
#include <iostream>

#if defined HT_DEBUG_IO
#define DEBUG(x) std::cout << x << std::endl;
#else
#define DEBUG(x)
#endif

template <typename T>
struct empty_key_t
{
    template <typename X>
    explicit empty_key_t(X&& x) :
        value(std::forward<X>(x)) {}

    T value;
};

template <typename K>
auto empty_key(K&& k) { return empty_key_t<K>(std::forward<K>(k)); }

namespace detail
{
    auto empty_callback = [](std::size_t) {};
}

template <typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>>
struct ht
{
    using Container = ht<Key, Value, Hash, Equal>;
    using Node = std::pair<Key, Value>;
    using iterator = std::size_t; // TODO

    template <typename K, typename X = std::enable_if_t<std::is_constructible<Key, K>::value>>
    ht(empty_key_t<K> k, std::size_t capacity = 16) :
      _empty_key(k.value),
      _elements(0),
      _table_sz(capacity),
      _table(std::make_unique<Node[]>(_table_sz))
    {
        for (std::size_t i = 0; i < _table_sz; ++i)
            new (&_table[i]) Node(_empty_key, Value());
    }

    ht(const ht& h) :
        _empty_key(h._empty_key),
        _elements(h._elements),
        _table_sz(h._table_sz),
        _table(std::make_unique<Node[]>(_table_sz))
    {
        for (std::size_t i = 0; i < _table_sz; ++i)
            new (&_table[i]) Node(h._table[i]);
    }

    ht& operator=(ht h)
    {
        std::swap(*this, h);
        return *this;
    }

    ht(ht&& h) noexcept :
        _empty_key(std::move(h._empty_key)),
        _elements(h._elements),
        _table_sz(h._table_sz),
        _table(std::move(std::make_unique<Node[]>(_table_sz)))
    {
    }

    friend void swap(ht& lhs, ht& rhs)
    {
        using std::swap;

        swap(lhs._empty_key, rhs._empty_key);
        swap(lhs._elements, rhs._elements);
        swap(lhs._table_sz, rhs._table_sz);
        swap(lhs._table, rhs._table);
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

    template <typename... Args> auto next(Args&&... args) { return next_linear(std::forward<Args>(args)...); }

    template <typename Pair>
    std::pair<iterator, bool> insert(Pair&& p)
    {
        resize(1);

        DEBUG("inserting " << p.first);
        assert(!Equal()(p.first, _empty_key));

        std::size_t pos = Hash()(p.first) & (_table_sz - 1);
        std::size_t num_probes = 1;

        DEBUG("trying to insert at pos=" << pos);

        while (!Equal()(_table[pos].first, _empty_key))
        {
            if (Equal()(_table[pos].first, p.first))
                return {{}, false};

            pos = next(pos, num_probes);

            DEBUG(" - not empty, trying pos=" << pos);
            assert(num_probes < _table_sz);
        }

        insert_element(pos, std::forward<Pair>(p));
        return {pos, true};
    }

    template <typename F = decltype(detail::empty_callback)>
    bool find(const Key& key, F f = detail::empty_callback)
    {
        std::size_t pos = Hash()(key) & (_table_sz - 1);
        std::size_t num_probes = 1;

        DEBUG("lookup at pos=" << pos);

        while (!Equal()(_table[pos].first, key))
        {
            if (Equal()(_table[pos].first, _empty_key))
                return false;

            pos = next(pos, num_probes);
            assert(num_probes < _table_sz);
        }

        f(num_probes);
        return true;
    }

    template <typename... Ts>
    bool count(Ts&&... ts)
    {
        return find(ts...);
    }

    Value& operator[](const Key& key)
    {
        std::size_t pos = Hash()(key) & (_table_sz - 1);
        std::size_t num_probes = 1;

        DEBUG("lookup at pos=" << pos);

        while (!Equal()(_table[pos].first, key))
        {
            if (Equal()(_table[pos].first, _empty_key))
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

    bool empty() const { return _elements == 0; }
    std::size_t size() const { return _elements; }
    std::size_t capacity() const { return _table_sz; }

// TODO private:
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

            ht h(empty_key_t<Key>(_empty_key), _table_sz * 2);
            std::for_each(&_table[0], &_table[_table_sz], [&h](auto&& p)
            {
                if (!Equal()(p.first, h.get_empty_key()))
                    h.insert(p);
            });

            swap(*this, h);

            DEBUG("resized.")
            return true;
        }

        return false;
    }

    const Key& get_empty_key() const { return _empty_key; }

    Key _empty_key;
    std::size_t _elements;
    std::size_t _table_sz;
    std::unique_ptr<Node[]> _table;
};
