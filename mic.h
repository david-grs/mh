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
    using index_type = void;
};

template <typename Object, typename Tag, typename Index>
struct mic_index<Object, unordered<Tag, Index>>
{
    using index = Index;
    using index_type = typename index::type;

    explicit mic_index() :
        __hashtable(empty_key_t<index_type>(index_type{}))
    {}

    template <typename K>
    auto find(K&& k) const
    {
        return __hashtable.find(std::forward<K>(k));
    }

    ht<index_type, Object*> __hashtable;
};


template <typename Object, typename Tag, typename Index>
struct mic_index<Object, ordered<Tag, Index>>
{
    using index_type = typename Index::type;

    template <typename K>
    auto find(K&&) const
    {
        return true;
    }
};

namespace detail
{

template <typename X, typename TupleT, std::size_t Index = 0>
struct get_index_from_t
{
    static_assert(Index < std::tuple_size<TupleT>::value, "type not found");

    static constexpr const std::size_t value =
         std::conditional<std::is_same<X, typename std::tuple_element_t<Index, TupleT>::index_type>::value,
                           std::integral_constant<std::size_t, Index>,
                           get_index_from_t<X, TupleT, Index + 1>>::type ::value;
};

}

template <typename Object, typename... Args>
struct mic
{
    using size_type = std::size_t;
    using indices = std::tuple<mic_index<Object, Args>...>;

    template <typename K>
    auto find(K&& k) const
    {
        constexpr const std::size_t index = get_index<K>();
        return std::get<index>(__indices).find(std::forward<K>(k));
    }

    size_type size() const { return _size; }

    template <typename T>
    static constexpr std::size_t get_index() { return detail::get_index_from_t<T, indices>::value; }

    std::vector<Object> _data;
    size_type _size;
    indices __indices;
};

// unordered< member<Person, std::string, &Person::first_name> >
// ht<std::string, Person*>
