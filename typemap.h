#pragma once

#include <tuple>
#include <type_traits>

template <typename... Ts>
struct typemap
{
  typemap(const Ts&... ts) : _ts(ts...) {}

  template <typename T>
  T get() { return std::get<get_index<T>()>(_ts); }

  template <typename T>
  void set(T&& t) { std::get<get_index<T>()>(_ts) = std::forward<T>(t); }

private:
  template <typename T>
  static constexpr std::size_t get_index() { return index<std::tuple<Ts...>, T, 0>(); }

  template <typename TupleT, typename T, std::size_t Index>
  static constexpr std::enable_if_t<std::is_same<T, std::tuple_element_t<Index, TupleT>>::value, std::size_t>
  index() { return Index; }

  template <typename TupleT, typename T, std::size_t Index>
  static constexpr std::enable_if_t<!std::is_same<T, std::tuple_element_t<Index, TupleT>>::value, std::size_t>
  index()
  {
    static_assert(Index + 1 < sizeof...(Ts), "type not found in map");
    return index<TupleT, T, Index + 1>();
  }

  std::tuple<Ts...> _ts;
};




