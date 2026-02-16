#pragma once

#include <utility>
#include <variant>

namespace crust {

template <typename... Ts>
struct OverloadedCallable : Ts... {
  using Ts::operator()...;
};

template <typename... Ts>
OverloadedCallable(Ts...) -> OverloadedCallable<Ts...>;

template <typename V, typename... Fs>
decltype(auto) match(V&& v, Fs&&... fs) {
  auto vistor = OverloadedCallable{std::forward<Fs>(fs)...};
  if constexpr (requires { typename std::remove_cvref_t<V>::Variant; }) {
    using Case = std::remove_cvref_t<V>;
    using Variant = typename Case::Variant;
    return std::visit(vistor, Variant{std::forward<V>(v)});
  }
  else {
    return std::visit(vistor, std::forward<V>(v));
  }
}

} // namespace crust
