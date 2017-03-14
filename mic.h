#pragma once

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

template <>
struct mic_index<>
{
};

template <typename Tag, typename Index, typename... Args>
struct mic_index<unordered<Tag, Index>, Args...> : public mic_index<Args...>
{

};


template <typename Tag, typename Index, typename... Args>
struct mic_index<ordered<Tag, Index>, Args...> : public mic_index<Args...>
{

};


template <typename Object, typename... Args>
struct mic
{
    using indices = mic_index<Args...>;

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
