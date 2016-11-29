#pragma once

#include "ht.h"

#include <vector>
template <typename Key, typename Value, typename EmptyKey>
struct hash_array
{
    using hashtable = ht<Key, Value, EmptyKey>;
    using iterator = std::size_t; //TODO

    template <typename Pair>
    std::pair<iterator, bool> insert(Pair&& pair)
    {
        auto p = _hashtable.insert(std::forward<Pair>(pair));
        return p;
    }

    hashtable _hashtable;
    std::vector<int> _sequence;
};
