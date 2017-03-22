#pragma once

#include <type_traits>
#include <utility>

template <std::size_t I>
struct value {};

template <std::size_t... Vals>
struct value_sequence {};


template <std::size_t... As, std::size_t... Bs>
constexpr value_sequence<As..., Bs...> operator+(value_sequence<As...>,
                                                 value_sequence<Bs...> )
{
    return {};
}

namespace detail
{

template <std::size_t Index, typename Is>
struct filter_impl;

template <std::size_t Index, std::size_t I, std::size_t... Is>
struct filter_impl<Index, value_sequence<I, Is...>>
{
    constexpr auto operator()()
    {
        using type = typename std::conditional<I == Index,
                        value_sequence<>,
                        value_sequence<I>>::type;

        return type{} + filter_impl<Index, value_sequence<Is...>>()();
    };
};

template <std::size_t Index>
struct filter_impl<Index, value_sequence<>>
{
    constexpr auto operator()() { return value_sequence<>{}; }
};

}

template <std::size_t... Is>
constexpr std::index_sequence<Is...> to_index_sequence(value_sequence<Is...>) { return {}; }

template <std::size_t Index, typename Is>
struct filter;

template <std::size_t Index, std::size_t... Is>
struct filter<Index, std::index_sequence<Is...>>
{
    constexpr auto operator()() { return to_index_sequence(detail::filter_impl<Index, value_sequence<Is...>>{}()); }
};
