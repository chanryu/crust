#pragma once

#include <concepts>
#include <functional>
#include <memory>

namespace crust {

template <std::copyable T>
class Cow {
public:
  Cow() : data_(std::make_shared<T>()) {}
  explicit Cow(T value) : data_(std::make_shared<T>(std::move(value))) {}
  explicit Cow(std::shared_ptr<T> ptr) : data_(std::move(ptr)) {}

  Cow(const Cow&) noexcept = default;
  Cow& operator=(const Cow&) noexcept = default;
  Cow(Cow&&) noexcept = default;
  Cow& operator=(Cow&&) noexcept = default;

  Cow& operator=(const T& value) {
    data_ = std::make_shared<T>(value);
    return *this;
  }

  Cow& operator=(T&& value) {
    data_ = std::make_shared<T>(std::move(value));
    return *this;
  }

  [[nodiscard]] const T& get() const noexcept {
    return *data_;
  }

  void mutate(std::invocable<T&> auto&& func) {
    make_unique();
    std::invoke(std::forward<decltype(func)>(func), *data_);
  }

  [[nodiscard]] const T& operator*() const noexcept {
    return get();
  }
  [[nodiscard]] const T* operator->() const noexcept {
    return data_.get();
  }

  [[nodiscard]] operator const T&() const noexcept {
    return get();
  }

  [[nodiscard]] bool is_unique() const noexcept {
    return data_.use_count() == 1;
  }

  [[nodiscard]] Cow clone() const {
    return Cow(*data_);
  }

  void swap(Cow& other) noexcept {
    data_.swap(other.data_);
  }

  [[nodiscard]] std::shared_ptr<T> release() && {
    return std::move(data_);
  }

  template <std::equality_comparable_with<T> U>
  [[nodiscard]] friend bool operator==(const Cow& lhs, const U& rhs) {
    return *lhs.data_ == rhs;
  }

private:
  void make_unique() {
    if (data_.use_count() > 1) {
      data_ = std::make_shared<T>(*data_);
    }
  }

  std::shared_ptr<T> data_;
};

} // namespace crust