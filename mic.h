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
    using key_type = void;
};

template <typename Index>
struct index_traits;

template <typename Index>
struct index_base
{
    using key_type = typename index_traits<Index>::key_type;
    using value_type = typename index_traits<Index>::value_type;
    using iterator = typename index_traits<Index>::iterator;
    using const_iterator = typename index_traits<Index>::const_iterator;

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        return emplace_unique(std::forward<Args>(args)...);
    }

private:
    template <typename Pair>
    std::pair<iterator, bool> emplace_unique(Pair&& p)
    {
        return emplace_pair(std::forward<Pair>(p), __can_extract_key<key_type, Pair>());
    }

    template <typename Pair>
    std::pair<iterator, bool> emplace_pair(Pair&& p, __extract_key_first)
    {
        return index_traits<Index>::emplace_unique_key(static_cast<Index&>(*this), p.first, std::forward<Pair>(p));
    }

    template <typename Pair>
    std::pair<iterator, bool> emplace_pair(Pair&& p, __extract_key_fail)
    {
        return emplace_unique_impl(std::forward<Pair>(p));
    }

    template <typename First, typename Second, typename RawFirst = typename std::decay<First>::type>
    typename std::enable_if_t<std::is_same<key_type, RawFirst>::value, std::pair<iterator, bool>>
    emplace_unique(First&& first, Second&& second)
    {
        return index_traits<Index>::emplace_unique_key(static_cast<Index&>(*this), std::forward<First>(first), std::forward<First>(first), std::forward<Second>(second));
    }

    template <typename First, typename Second, typename RawFirst = typename std::decay<First>::type>
    typename std::enable_if_t<!std::is_same<key_type, RawFirst>::value && std::is_constructible<key_type, RawFirst>::value, std::pair<iterator, bool>>
    emplace_unique(First&& first, Second&& second)
    {
        return index_traits<Index>::emplace_unique_key(static_cast<Index&>(*this), key_type(first), std::forward<First>(first), std::forward<Second>(second));
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
        return index_traits<Index>::emplace_unique_key(static_cast<Index&>(*this), p.first, std::move(p));
    }
};

template <typename Object, typename Tag, typename Index>
struct mic_index<Object, unordered<Tag, Index>> : public index_base<mic_index<Object, unordered<Tag, Index>>>
{
    using index = Index;
    using index_type = typename index::type;
    using iterator = typename ht<index_type, Object*>::iterator;
    using const_iterator = typename ht<index_type, Object*>::const_iterator;

    explicit mic_index() :
        __hashtable(empty_key_t<index_type>(index_type{}))
    {}

    template <typename K>
    auto find(K&& k) const
    {
        return __hashtable.find(std::forward<K>(k));
    }

    template <typename Pair>
    std::pair<iterator, bool> insert(Pair&& p)
    {
        return __hashtable.insert(std::forward<Pair>(p));
    }

    ht<index_type, Object*> __hashtable;
};


template <typename Object, typename Tag, typename Index>
struct mic_index<Object, ordered<Tag, Index>> : public index_base<mic_index<Object, ordered<Tag, Index>>>
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

    template <std::size_t Index>
    auto index()
    {
        return std::get<Index>(__indices);
    }

    template <std::size_t Index>
    auto index() const
    {
        return std::get<Index>(__indices);
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
