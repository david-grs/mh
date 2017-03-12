#pragma once

#include <string>

template <typename C, typename T, T C::* M>
struct unordered {};

template <typename X>
struct ordered {};

struct sequence {};

template <typename Key, typename... Args>
struct mic
{
};
