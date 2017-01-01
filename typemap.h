#pragma once

#include <tuple>
#include <type_traits>
#include <sstream>

template <typename... Ts>
struct typemap
{
  typemap(const Ts&... ts) : _ts(ts...) {}

  template <typename T>
  const T& get() const { return std::get<get_index<T>()>(_ts); }

  template <typename T>
  T& get() { return std::get<get_index<T>()>(_ts); }

  template <typename F>
  void for_each(F&& f)
  {
    for_each_arg<0>(std::forward<F>(f));
  }

  template <typename F>
  void for_each(F&& f) const
  {
    for_each_arg<0>(std::forward<F>(f));
  }

private:
  template <std::size_t I, typename F>
  std::enable_if_t<I < sizeof...(Ts)>
  for_each_arg(F&& f)
  {
    f(std::get<I>(_ts));
    for_each_arg<I + 1>(std::forward<F>(f));
  }

  template <std::size_t I, typename F>
  std::enable_if_t<I < sizeof...(Ts)>
  for_each_arg(F&& f) const
  {
    f(std::get<I>(_ts));
    for_each_arg<I + 1>(std::forward<F>(f));
  }

  template <std::size_t I, typename F>
  std::enable_if_t<I == sizeof...(Ts)>
  for_each_arg(F&&) const {}

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

template <typename... Ts>
inline std::ostream& operator<<(std::ostream& os, const typemap<Ts...>& m)
{
    m.for_each([&](auto x) { os << x << " "; });
    return os;
}

template <typename... Ts>
inline std::istream& operator>>(std::istream& is, typemap<Ts...>& m)
{
    m.for_each([&](auto& x) { is >> x; });
    return is;
}
