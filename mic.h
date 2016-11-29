#pragma once

#include "ht.h"

#include <boost/operators.hpp>
#include <vector>

template <typename Key, typename Value, typename EmptyKey>
struct hash_array
{
    using value_type = Value;
    using reference = value_type&;
    using size_type = std::size_t;
    using difference_type = std::size_t;

    using hashtable = ht<Key, Value, EmptyKey>;

    struct iterator_base
    {
        iterator_base(hashtable& h, std::size_t i) :
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
        hashtable& _h;
        std::size_t _i;
    };

     struct iterator :
        public iterator_base,
        public boost::random_access_iterator_helper<iterator, value_type>
    {
        using iterator_base::iterator_base;

        reference operator*() { return this->_h._sequence[this->_i]; }
    };

    template <typename Pair>
    std::pair<iterator, bool> insert(Pair&& pair)
    {
        auto p = _hashtable.insert(std::forward<Pair>(pair));

        if (p.second)
            _sequence.emplace_back(p.first);

        return {{}, true}; // TODO
    }

    iterator begin() const { return iterator(*this, 0); }
    iterator end() const { return iterator(*this, _sequence.size()); }

    hashtable _hashtable;
    std::vector<typename hashtable::iterator> _sequence;
};
