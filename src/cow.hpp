#pragma once

#include <concepts>
#include <functional>
#include <memory>

namespace crust {

template <std::copyable T>
class Cow {
public:
  Cow()
    requires std::default_initializable<T>
      : data_(std::make_shared<T>()) {}

  Cow(T const& value) : data_(std::make_shared<T>(value)) {}

  Cow(T&& value)
    requires std::move_constructible<T>
      : data_(std::make_shared<T>(std::move(value))) {}

  Cow(Cow const& other) = default;

  Cow(Cow&& other) noexcept = delete;

  auto operator=(Cow const& other) -> Cow& = default;

  auto operator=(Cow&& other) noexcept -> Cow& = delete;

  auto operator=(T const& value) -> Cow& {
    data_ = std::make_shared<T>(value);
    return *this;
  }

  auto operator=(T&& value) -> Cow&
    requires std::move_constructible<T>
  {
    data_ = std::make_shared<T>(std::move(value));
    return *this;
  }

  [[nodiscard]] auto get() const noexcept -> T const& {
    return *data_;
  }

  template <typename F>
  auto mutate(F&& func) -> std::invoke_result_t<F, T&> {
    make_unique();
    return std::invoke(std::forward<F>(func), *data_);
  }

  [[nodiscard]] auto operator*() const noexcept -> T const& {
    return get();
  }

  [[nodiscard]] auto operator->() const noexcept -> T const* {
    return data_.get();
  }

  [[nodiscard]] auto is_unique() const noexcept -> bool {
    return data_.use_count() == 1;
  }

  [[nodiscard]] auto clone() const -> Cow {
    return Cow(*data_);
  }

  friend auto operator==(Cow const& lhs, Cow const& rhs) -> bool
    requires std::equality_comparable<T>
  {
    return *lhs.data_ == *rhs.data_;
  }

  friend auto operator==(Cow const& lhs, T const& rhs) -> bool
    requires std::equality_comparable<T>
  {
    return *lhs.data_ == rhs;
  }

  friend auto operator==(T const& lhs, Cow const& rhs) -> bool
    requires std::equality_comparable<T>
  {
    return lhs == *rhs.data_;
  }

private:
  auto make_unique() -> void {
    if (!is_unique()) {
      data_ = std::make_shared<T>(*data_);
    }
  }

  std::shared_ptr<T> data_;
};

template <typename T, typename... Args>
auto make_cow(Args&&... args) -> Cow<T> {
  return Cow<T>(T{std::forward<Args>(args)...});
}

} // namespace crust