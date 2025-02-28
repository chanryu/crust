#pragma once

#include <mutex>
#include <shared_mutex>
#include <tuple>
#include <type_traits>

namespace crust {

template <typename M>
concept Lockable = requires(M m) {
  m.lock();
  m.unlock();
};

template <typename M>
concept SharedLockable = requires(M m) {
  m.lock_shared();
  m.unlock_shared();
};

template <typename T, Lockable M>
class LockGuard final {
public:
  LockGuard(M& mutex, T& data) noexcept : mutex_{mutex}, data_{data} {
    mutex_.lock();
  }

  ~LockGuard() noexcept {
    mutex_.unlock();
  }

  LockGuard(const LockGuard&) = delete;
  LockGuard& operator=(const LockGuard&) = delete;
  LockGuard(LockGuard&&) = delete;
  LockGuard& operator=(LockGuard&&) = delete;

  auto operator->() -> T* {
    return &data_;
  }

  auto operator*() -> T& {
    return data_;
  }

private:
  M& mutex_;
  T& data_;
};

template <typename T, SharedLockable M>
class SharedLockGuard final {
public:
  SharedLockGuard(M& mutex, T const& data) noexcept : mutex_{mutex}, data_{data} {
    mutex_.lock_shared();
  }

  ~SharedLockGuard() noexcept {
    mutex_.unlock_shared();
  }

  SharedLockGuard(const SharedLockGuard&) = delete;
  SharedLockGuard& operator=(const SharedLockGuard&) = delete;
  SharedLockGuard(SharedLockGuard&&) = delete;
  SharedLockGuard& operator=(SharedLockGuard&&) = delete;

  auto operator->() const -> T const* {
    return &data_;
  }

  auto operator*() const -> T const& {
    return data_;
  }

private:
  M& mutex_;
  T const& data_;
};

template <typename T, Lockable M = std::mutex>
class Mutex;

template <typename... MutexTypes>
class ScopedLockGuard final {
private:
  using TupleType = std::tuple<MutexTypes&...>;
  TupleType mutexes_;
  std::tuple<typename MutexTypes::value_type&...> data_;
  std::scoped_lock<typename MutexTypes::mutex_type...> lock_;

public:
  ScopedLockGuard(MutexTypes&... mutexes) : mutexes_(mutexes...), data_(mutexes.data_...), lock_(mutexes.mutex_...) {}

  ScopedLockGuard(const ScopedLockGuard&) = delete;
  ScopedLockGuard& operator=(const ScopedLockGuard&) = delete;
  ScopedLockGuard(ScopedLockGuard&&) = delete;
  ScopedLockGuard& operator=(ScopedLockGuard&&) = delete;

  auto& get_data() {
    return data_;
  }

  template <size_t I>
  auto& get() {
    return std::get<I>(data_);
  }
};

template <typename T, Lockable M>
class Mutex {
public:
  using value_type = T;
  using mutex_type = M;

  static_assert(!std::is_reference_v<T>, "T cannot be a reference type");
  static_assert(std::is_object_v<T>, "T must be an object type");

  template <typename... Args>
  explicit Mutex(Args&&... args) : data_(std::forward<Args>(args)...) {}

  explicit Mutex(T const& data) : data_{data} {}
  explicit Mutex(T&& data) : data_{std::move(data)} {}

  Mutex(Mutex&& other) noexcept(std::is_nothrow_move_constructible_v<T>) : data_{std::move(other.data_)} {}

  Mutex& operator=(Mutex&& other) noexcept(std::is_nothrow_move_assignable_v<T>) {
    if (this != &other) {
      data_ = std::move(other.data_);
    }
    return *this;
  }

  Mutex(Mutex const&) = delete;
  Mutex& operator=(Mutex const&) = delete;

  [[nodiscard]] auto lock() {
    return LockGuard<T, M>{mutex_, data_};
  }

  template <typename F>
  auto lock(F&& func) -> decltype(auto) {
    auto guard = lock();
    return std::forward<F>(func)(*guard);
  }

  [[nodiscard]] auto lock_shared() const
    requires SharedLockable<M>
  {
    return SharedLockGuard<T, M>{mutex_, data_};
  }

  template <typename F>
  auto lock_shared(F&& func) const -> decltype(auto)
    requires SharedLockable<M>
  {
    auto const guard = lock_shared();
    return std::forward<F>(func)(*guard);
  }

  template <typename... MutexTypes>
  friend class ScopedLockGuard;

  template <typename... MutexTypes>
  friend auto scoped_lock(MutexTypes&... mutexes);

  template <typename F, typename... MutexTypes>
  friend auto with_scoped_lock(F&& func, MutexTypes&... mutexes) -> decltype(auto);

private:
  mutable M mutex_;
  T data_;
};

template <typename... MutexTypes>
auto scoped_lock(MutexTypes&... mutexes) {
  return ScopedLockGuard<MutexTypes...>(mutexes...);
}

template <typename F, typename... MutexTypes>
auto with_scoped_lock(F&& func, MutexTypes&... mutexes) -> decltype(auto) {
  auto guard = scoped_lock(mutexes...);
  return std::apply(std::forward<F>(func), guard.get_data());
}

template <typename T>
using RecursiveMutex = Mutex<T, std::recursive_mutex>;

template <typename T>
using SharedMutex = Mutex<T, std::shared_mutex>;

} // namespace crust
