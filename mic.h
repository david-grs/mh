#pragma once

#include "ht.h"

#include <boost/operators.hpp>
#include <vector>

template <typename Key, typename Value>
struct hash_array
{
    using value_type = Value;
    using reference = value_type&;
    using size_type = std::size_t;
    using difference_type = std::size_t;

    using hashtable = ht<Key, Value>;

    template <typename K, typename X = std::enable_if_t<std::is_constructible<Key, K>::value>>
    hash_array(empty_key_t<K> k, std::size_t capacity = 16) :
        _hashtable(k, capacity)
    {}

    struct iterator_base
    {
        iterator_base(hash_array* h, std::size_t i) :
            _h(h),
            _i(i)
        {}
        virtual ~iterator_base() {}

        iterator_base& operator+=(std::size_t i) { _i += i; return *this; }
        iterator_base& operator-=(std::size_t i) { return operator+=(-i); }
        iterator_base& operator++()              { return operator+=(1); }
        iterator_base& operator--()              { return operator+=(-1); }

        difference_type operator-(const iterator_base& it) { assert(_h == it._h); return _i - it._i; }

        bool operator< (const iterator_base& it) const { assert(_h == it._h); return _i < it._i; }
        bool operator==(const iterator_base& it) const { return _h == it._h && _i == it._i; }

     protected:
        hash_array* _h;
        std::size_t _i;
    };

     struct iterator :
        public iterator_base,
        public boost::random_access_iterator_helper<iterator, value_type>
    {
        using iterator_base::iterator_base;

        auto& operator*() { return this->_h->_hashtable._table[this->_h->_sequence[this->_i]]; }
    };

    template <typename Pair>
    std::pair<iterator, bool> insert(Pair&& pair)
    {
        auto p = _hashtable.insert(std::forward<Pair>(pair));

        if (p.second)
            _sequence.emplace_back(p.first);

        return {end() - 1, true};
    }

    template <typename K>
    bool find(K&& key)
    {
        return _hashtable.find(std::forward<K>(key));
    }

    template <typename... Ts>
    bool count(Ts&&... ts)
    {
        return find(ts...);
    }

    iterator begin() { return iterator(this, 0); }
    iterator end()   { return iterator(this, _sequence.size()); }

    const Key& get_empty_key() const { return _hashtable.get_empty_key(); }

    bool empty() const { return _hashtable.empty(); }
    std::size_t size() const { return _hashtable.size(); }
    std::size_t capacity() const { return _hashtable.capacity(); }

    hashtable _hashtable;
    std::vector<typename hashtable::iterator> _sequence;
};
