#pragma once

#include <type_traits>
#include <utility>

struct __extract_key_first {};
struct __extract_key_self {};
struct __extract_key_fail {};

template <typename K, typename V, typename RawV = typename std::decay<V>::type>
struct __can_extract_key
    : std::conditional<std::is_same<K, RawV>::value, __extract_key_self, __extract_key_fail>::type {};

template <typename K, typename Pair, typename First, typename Second>
struct __can_extract_key<K, Pair, std::pair<First, Second>>
    : std::conditional<std::is_same<K, typename std::remove_const<First>::type>::value, __extract_key_first, __extract_key_fail>::type {};

