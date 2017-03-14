#pragma once

#include "ht.h"

#include <vector>


template <typename C, typename T, T C::* M>
struct member
{
    using type = T;
};

template <typename C, typename T, T(C::* M) ()>
struct mem_fun
{
    using type = T;
};


template <typename Tag, typename Index>
struct unordered
{
    using tag = Tag;
    using index = Index;
};

template <typename Tag, typename Index>
struct ordered
{
    using tag = Tag;
    using index = Index;
};

struct sequence {};

template <typename... Args>
struct mic_index;

template <typename Object>
struct mic_index<Object>
{
};

template <typename Object, typename Tag, typename Index, typename... Args>
struct mic_index<Object, unordered<Tag, Index>, Args...> : public mic_index<Object, Args...>
{
    explicit mic_index() :
        __hashtable(empty_key_t<index_type>(index_type{}))
    {}

    using index_type = typename Index::type;
    ht<index_type, Object*> __hashtable;
};


template <typename Object, typename Tag, typename Index, typename... Args>
struct mic_index<Object, ordered<Tag, Index>, Args...> : public mic_index<Object, Args...>
{

};


template <typename Object, typename... Args>
struct mic
{
    using indices = mic_index<Object, Args...>;

    template <typename K>
    bool find(K&& k)
    {
        return true;
    }

    std::vector<Object> _data;
    indices _indices;
};

// unordered< member<Person, std::string, &Person::first_name> >
// ht<std::string, Person*>
