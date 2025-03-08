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

  Cow(T const& value)
    requires std::copy_constructible<T>
      : data_(std::make_shared<T>(value)) {}

  Cow(T&& value)
    requires std::move_constructible<T>
      : data_(std::make_shared<T>(std::move(value))) {}

  auto operator=(T const& value) -> Cow&
    requires std::copy_constructible<T>
  {
    if (this != &value) {
      data_ = std::make_shared<T>(value);
    }
    return *this;
  }

  auto operator=(T&& value) -> Cow&
    requires std::move_constructible<T>
  {
    if (this != &value) {
      data_ = std::make_shared<T>(std::move(value));
    }
    return *this;
  }

  [[nodiscard]] auto get() const noexcept -> T const& {
    return *data_;
  }

  auto mutate(std::invocable<T&> auto&& func) {
    make_unique();
    std::invoke(std::forward<decltype(func)>(func), *data_);
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

  auto swap(Cow& other) noexcept {
    std::swap(data_, other.data_);
  }

  [[nodiscard]] auto release() && -> std::shared_ptr<T> {
    return std::move(data_);
  }

private:
  auto make_unique() {
    if (data_.use_count() > 1) {
      data_ = std::make_shared<T>(*data_);
    }
  }

  std::shared_ptr<T> data_;
};

} // namespace crust
