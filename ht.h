#pragma once

#include "type_traits.h"

#include <algorithm>
#include <type_traits>
#include <functional>
#include <memory>
#include <cassert>
#include <iostream>

#include <boost/operators.hpp>

#if defined _HT_DEBUG_IO
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

template <class, class, class, class> struct ht;

template <typename _Node>
struct __iterator_base
{
    using iterator_category = std::forward_iterator_tag;

    using value_type      = typename _Node::value_type;
    using difference_type = typename _Node::difference_type;

    explicit __iterator_base(_Node node) :
        _node(std::move(node))
    { }
    virtual ~__iterator_base() {}

    __iterator_base& operator+=(std::size_t inc) { _node.advance(inc); return *this;}
    __iterator_base& operator++()                { return operator+=(1); }
    __iterator_base& operator++(int)             { return operator+=(1); }

    bool operator==(const __iterator_base& it) const { return _node == it._node; }
    bool operator!=(const __iterator_base& it) const { return !(*this == it); }

    template <class, class, class, class> friend struct ht;

protected:
    _Node _node;
};

template <typename _Node>
struct __iterator : public __iterator_base<_Node>
{
    using reference       = typename _Node::value_type&;
    using pointer         = typename _Node::value_type*;

    using __iterator_base<_Node>::__iterator_base;

    reference operator*()  { return *this->_node.get_ptr(); }
    pointer   operator->() { return this->_node.get_ptr(); }
};

template <typename _Node>
struct __const_iterator : public __iterator_base<_Node>
{
    using reference       = typename _Node::value_type const&;
    using pointer         = typename _Node::value_type const*;

    using __iterator_base<_Node>::__iterator_base;

    __const_iterator(__iterator<_Node> it) :
        __iterator_base<_Node>(it)
    {}

    reference operator*()  { return *this->_node.get_ptr(); }
    pointer   operator->() { return this->_node.get_ptr(); }
};

template <typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>>
struct ht
{
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const Key, Value>;
    using difference_type = std::size_t;
    using size_type = std::size_t;
    using reference = value_type&; // TODO
    using pointer = value_type*; // TODO

private:
    struct __node_type
    {
        using size_type = ht::size_type;
        using value_type = ht::value_type;
        using difference_type = ht::difference_type;

        explicit __node_type(ht* hashtable, size_type pos) :
            _hashtable(hashtable),
            _pos(pos)
        {}

        pointer get_ptr()
        {
            return &_hashtable->_table[this->_pos];
        }

        void advance(size_type inc)
        {
            for (; _pos < _hashtable->_table_sz; ++_pos)
            {
                if (!Equal()(_hashtable->_table[_pos].first, _hashtable->_empty_key))
                {
                    if (inc)
                        --inc;
                    else
                        break;
                }
            }
        }

        bool operator==(const __node_type& n) const { return _hashtable == n._hashtable && _pos == n._pos; }

        template <class, class, class, class> friend struct ht;

    private:
        ht* _hashtable;
        size_type _pos;
    };

public:
    using iterator = __iterator<__node_type>;
    using const_iterator = __const_iterator<__node_type>;
    // TODO local_iterator, const_local_iterator

    template <typename K, typename X = std::enable_if_t<std::is_constructible<Key, K>::value>>
    ht(empty_key_t<K> k, std::size_t capacity = 16) :
      _empty_key(k.value),
      _elements(0),
      _table_sz(capacity),
      _table(std::make_unique<value_type[]>(_table_sz))
    {
        for (std::size_t i = 0; i < _table_sz; ++i)
            new (&_table[i]) value_type(_empty_key, Value());
    }

    ht(const ht& h) :
        _empty_key(h._empty_key),
        _elements(h._elements),
        _table_sz(h._table_sz),
        _table(std::make_unique<value_type[]>(_table_sz))
    {
        for (std::size_t i = 0; i < _table_sz; ++i)
            new (&_table[i]) value_type(h._table[i]);
    }

    ht& operator=(ht h)
    {
        swap(*this, h);
        return *this;
    }

    ht(ht&& h) noexcept :
        _empty_key(std::move(h._empty_key)),
        _elements(h._elements),
        _table_sz(h._table_sz),
        _table(std::move(h._table))
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

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        return emplace_unique(std::forward<Args>(args)...);
    }

    std::pair<iterator, bool> insert(const value_type& obj)
    {
        return emplace_unique(obj.first, obj.second);
    }

    template <typename Pair>
    std::pair<iterator, bool> insert(Pair&& obj)
    {
        return emplace_unique(std::forward<Pair>(obj));
    }

private:
    template <typename Pair>
    std::pair<iterator, bool> emplace_unique(Pair&& p)
    {
        return emplace_pair(std::forward<Pair>(p), __can_extract_key<Key, Pair>());
    }

    template <typename Pair>
    std::pair<iterator, bool> emplace_pair(Pair&& p, __extract_key_first)
    {
        return emplace_unique_key(p.first, std::forward<Pair>(p));
    }

    template <typename Pair>
    std::pair<iterator, bool> emplace_pair(Pair&& p, __extract_key_fail)
    {
        return emplace_unique_impl(std::forward<Pair>(p));
    }

    template <typename First, typename Second, typename RawFirst = typename std::decay<First>::type>
    typename std::enable_if_t<std::is_same<Key, RawFirst>::value, std::pair<iterator, bool>>
    emplace_unique(First&& first, Second&& second)
    {
        return emplace_unique_key(std::forward<First>(first), std::forward<First>(first), std::forward<Second>(second));
    }

    template <typename First, typename Second, typename RawFirst = typename std::decay<First>::type>
    typename std::enable_if_t<!std::is_same<Key, RawFirst>::value && std::is_constructible<Key, RawFirst>::value, std::pair<iterator, bool>>
    emplace_unique(First&& first, Second&& second)
    {
        return emplace_unique_key(Key(first), std::forward<First>(first), std::forward<Second>(second));
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace_unique(Args&&... args)
    {
        return emplace_unique_impl(std::forward<Args>(args)...);
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace_unique_impl(Args&&... args)
    {
        value_type p(std::forward<Args>(args)...);
        return emplace_unique_key(p.first, std::move(p));
    }

    template <typename K, typename... Args>
    std::pair<iterator, bool> emplace_unique_key(K&& key, Args&&... args)
    {
        resize(1);

        DEBUG("inserting " << key);
        assert(!Equal()(key, _empty_key));

        std::size_t pos = Hash()(key) & (_table_sz - 1);
        std::size_t num_probes = 1;

        DEBUG("trying to insert at pos=" << pos);

        while (!Equal()(_table[pos].first, _empty_key))
        {
            if (Equal()(_table[pos].first, key))
                return {iterator(__node_type(this, pos)), false}; // TODO check

            pos = next(pos, num_probes);

            DEBUG(" - not empty, trying pos=" << pos);
            assert(num_probes < _table_sz);
        }

        insert_element(pos, std::forward<Args>(args)...);
        return {iterator(__node_type(this, pos)), true};
    }

public:
    iterator erase(const_iterator it)
    {
        size_type pos = it._node._pos;

        const_cast<key_type&>(_table[pos].first) = _empty_key;
        _table[pos].second.~Value();

        ++it;
        return iterator(__node_type(this, it._node._pos));
    }

    size_type erase(const Key& key)
    {
        std::size_t pos = Hash()(key) & (_table_sz - 1);
        std::size_t num_probes = 1;

        while (!Equal()(_table[pos].first, key))
        {
            if (Equal()(_table[pos].first, _empty_key))
            {
                const_cast<key_type&>(_table[pos].first) = _empty_key;
                _table[pos].second.~Value();
                return 1;
            }

            pos = next(pos, num_probes);
            assert(num_probes < _table_sz);
        }

        return 0;
    }

    const_iterator find(const Key& key) const
    {
        return const_cast<ht&>(*this).find(key);
    }

    iterator find(const Key& key)
    {
        std::size_t pos = Hash()(key) & (_table_sz - 1);
        std::size_t num_probes = 1;

        DEBUG("first probing pos=" << pos);

        while (!Equal()(_table[pos].first, _empty_key))
        {
            if (Equal()(_table[pos].first, key))
                return iterator(__node_type(this, pos));

            pos = next(pos, num_probes);
            assert(num_probes < _table_sz);

            DEBUG("next probing pos=" << pos);
        }

        return end();
    }

    template <typename... Ts>
    std::size_t count(Ts&&... ts)
    {
        return find(ts...) != end();
    }

    template <typename K>
    Value& operator[](K&& key)
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
                    insert_element(pos, std::forward<K>(key), Value{});
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
    template <typename... Args>
    void insert_element(std::size_t pos, Args&&... args)
    {
        new (&_table[pos]) value_type(std::forward<Args>(args)...);
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

    iterator begin()
    {
        // TODO cache this value in a compressed_pair
        std::size_t pos = 0;
        for (; pos < _table_sz && Equal()(_table[pos].first, _empty_key); ++pos);
        return iterator(__node_type(this, pos));
    }

    iterator end()   { return iterator(__node_type(this, _table_sz)); }

    const_iterator begin() const { return const_cast<ht*>(this)->begin(); }
    const_iterator end()   const { return const_cast<ht*>(this)->end(); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend()   const { return end(); }

    void clear()
    {
        // TODO

    }

#ifdef _HT_DEBUG

    void dump(std::ostream& os)
    {
        for (size_type i = 0; i < _table_sz; ++i)
            os << _table[i].first << " ";
    }

#endif

    Key _empty_key;
    std::size_t _elements;
    std::size_t _table_sz;
    std::unique_ptr<value_type[]> _table;
};
