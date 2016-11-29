#pragma once

#include "ht.h"

#include <boost/operators.hpp>
#include <vector>

template <typename Key, typename Value, typename EmptyKey>
struct hash_array
{
    using value_type = Value;
    using reference = value_type&;

    using hashtable = ht<Key, Value, EmptyKey>;

    struct iterator_base
    {


    protected:
        hash_array<Key, Value, EmptyKey>& _container;
        std::size_t _index;
    };

     struct iterator :
        public iterator_base,
        public boost::random_access_iterator_helper<iterator, value_type>
    {
        using iterator_base::iterator_base;

        reference operator*() { return (*this->m_container)[this->m_index]; }
    };

    template <typename Pair>
    std::pair<iterator, bool> insert(Pair&& pair)
    {
        auto p = _hashtable.insert(std::forward<Pair>(pair));

        if (p.second)
            _sequence.emplace_back(p.first);

        return {{}, true}; // TODO
    }

  //  auto cbegin() const { return _sequence.cbegin(); }
   // auto cend() const { return _sequence.cend(); }

    hashtable _hashtable;
    std::vector<typename hashtable::iterator> _sequence;
};
