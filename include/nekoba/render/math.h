#pragma once

namespace nekoba {

namespace math {

template <typename T> constexpr auto Pi = T(3.14159265358979323846);
template <typename T> constexpr auto InvPi = T(0.31830988618379067154);
template <typename T> constexpr auto InvTwoPi = T(0.15915494309189533577);
template <typename T> constexpr auto InvFourPi = T(0.07957747154594766788);
template <typename T>
constexpr auto Infinity = std::numeric_limits<T>::infinity();
template <typename T>
constexpr auto Epsilon = std::numeric_limits<T>::epsilon() / 2;

template <typename T> inline auto deg_to_rag(const T &v) {
  return v * T(Pi<T> / 180);
}

template <typename T> inline auto rag_to_deg(const T &v) {
  return v * T(180 / Pi<T>);
}

} // namespace math
} // namespace nekoba