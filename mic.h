#pragma once

#include "ht.h"

#include <vector>
#include <tuple>
#include <type_traits>

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

template <typename Object, typename Tag, typename Index>
struct mic_index<Object, unordered<Tag, Index>>
{
    explicit mic_index() :
        __hashtable(empty_key_t<index_type>(index_type{}))
    {}

    template <typename K>
    bool find(K&& k)
    {
        return true;
    }

    using index_type = typename Index::type;
    ht<index_type, Object*> __hashtable;
};


template <typename Object, typename Tag, typename Index>
struct mic_index<Object, ordered<Tag, Index>>
{
    template <typename K>
    bool find(K&& k)
    {
        return true;
    }
};


template <typename Object, typename... Args>
struct mic
{
    using indices = std::tuple<mic_index<Object, Args>...>;

    template <typename K>
    bool find(K&& k)
    {
        constexpr const int index = get_index<K>();
        return std::get<index>(__indices).find(std::forward<K>(k));
    }

    template <typename K>
    constexpr std::size_t get_index() const { return get_index<K, decltype(__indices), 0>(); }

    template <typename X, typename TupleT, std::size_t Index>
    static constexpr std::enable_if_t<std::is_same<X, std::tuple_element_t<Index, TupleT>>::value, std::size_t> get_index()
    {
        return Index;
    }

    template <typename X, typename TupleT, std::size_t Index>
    static constexpr std::enable_if_t<!std::is_same<X, std::tuple_element_t<Index, TupleT>>::value, std::size_t> get_index()
    {
        static_assert(Index < std:: tuple_size<TupleT>::value, "");
        return get_index<X, TupleT, Index + 1>();
    }

    std::vector<Object> _data;
    indices __indices;
};

// unordered< member<Person, std::string, &Person::first_name> >
// ht<std::string, Person*>
