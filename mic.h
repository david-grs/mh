#pragma once

#include <string>


template <typename C, typename T, T C::* M>
struct member {};

template <typename C, typename T, T(C::* M) ()>
struct mem_fun {};


template <typename Tag, typename X>
struct unordered {};

template <typename Tag, typename X>
struct ordered {};

struct sequence {};

template <typename Key, typename... Args>
struct mic
{
};
