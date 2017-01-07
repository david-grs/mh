#pragma once

#include <tuple>
#include <type_traits>
#include <sstream>

template <typename T, typename... Ts>
struct typemap
{
  typemap() {}
  typemap(const T& t, const Ts&... ts) : _ts(t, ts...) {}

  template <typename X>
  const X& get() const { return std::get<get_index<X>()>(_ts); }

  template <typename X>
  X& get() { return std::get<get_index<X>()>(_ts); }

  template <typename F>
  void visit(F&& f)
  {
    visit_element<0>(std::forward<F>(f));
  }

  template <typename F>
  void visit(F&& f) const
  {
    visit_element<0>(std::forward<F>(f));
  }

private:
  template <std::size_t I, typename F>
  std::enable_if_t<I < sizeof...(Ts) + 1>
  visit_element(F&& f)
  {
    f(std::get<I>(_ts));
    visit_element<I + 1>(std::forward<F>(f));
  }

  template <std::size_t I, typename F>
  std::enable_if_t<I < sizeof...(Ts) + 1>
  visit_element(F&& f) const
  {
    f(std::get<I>(_ts));
    visit_element<I + 1>(std::forward<F>(f));
  }

  template <std::size_t I, typename F>
  std::enable_if_t<I == sizeof...(Ts) + 1>
  visit_element(F&&) const {}

  template <typename X>
  static constexpr std::size_t get_index() { return index<std::tuple<T, Ts...>, X, 0>(); }

  template <typename TupleT, typename X, std::size_t Index>
  static constexpr std::enable_if_t<std::is_same<X, std::tuple_element_t<Index, TupleT>>::value, std::size_t>
  index() { return Index; }

  template <typename TupleT, typename X, std::size_t Index>
  static constexpr std::enable_if_t<!std::is_same<X, std::tuple_element_t<Index, TupleT>>::value, std::size_t>
  index()
  {
    static_assert(Index < sizeof...(Ts), "type not found in map");
    return index<TupleT, X, Index + 1>();
  }

  std::tuple<T, Ts...> _ts;
};

template <typename... Ts>
auto make_typemap(Ts&&... ts)
{
    return typemap<Ts...>(std::forward<Ts>(ts)...);
}

template <typename... Ts>
inline std::ostream& operator<<(std::ostream& os, const typemap<Ts...>& m)
{
    m.visit([&](auto x) { os << x << " "; });
    return os;
}

template <typename... Ts>
inline std::istream& operator>>(std::istream& is, typemap<Ts...>& m)
{
    m.visit([&](auto& x) { is >> x; });
    return is;
}
