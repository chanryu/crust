#pragma once

#include <utility>
#include <variant>

namespace crust {

template <typename... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename V, typename... Fs>
decltype(auto) match(V&& v, Fs&&... fs) {
  return std::visit(overloaded{std::forward<Fs>(fs)...}, std::forward<V>(v));
}

} // namespace crust