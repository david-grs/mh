#include <array>

template <typename T, typename... Ts>
constexpr auto make_array(T t, Ts... ts)
{
    return std::array<T, sizeof...(Ts) + 1>{t, ts...};
}
