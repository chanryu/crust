#pragma once

#include <mutex>
#include <shared_mutex>

namespace crust {

// Concept to check if a M has exclusive locking capabilities
template <typename M>
concept Lockable = requires(M m) {
  m.lock();
  m.unlock();
};

// Concept to check if a M has shared locking capabilities
template <typename M>
concept SharedLockable = requires(M m) {
  m.lock_shared();
  m.unlock_shared();
};

// Exclusive Lock Guard
template <typename T, Lockable M>
class LockGuard final {
public:
  LockGuard(M& mutex, T& data) : mutex_{mutex}, data_{data} {
    mutex_.lock();
  }

  ~LockGuard() {
    mutex_.unlock();
  }

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

// Shared Lock Guard
template <typename T, SharedLockable M>
class SharedLockGuard final {
public:
  SharedLockGuard(M& mutex, T const& data) : mutex_{mutex}, data_{data} {
    mutex_.lock_shared();
  }

  ~SharedLockGuard() {
    mutex_.unlock_shared();
  }

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
class Mutex {
public:
  explicit Mutex(T&& data) : data_{std::move(data)} {
  }

  explicit Mutex(T const& data) : data_{data} {
  }

  Mutex(Mutex const&) = delete;
  Mutex& operator=(Mutex const&) = delete;

  [[nodiscard]] auto lock() -> LockGuard<T, M> {
    return {mutex_, data_};
  }

  void lock(auto&& func) {
    auto guard = lock();
    std::forward<decltype(func)>(func)(*guard);
  }

  template <SharedLockable M_ = M>
  [[nodiscard]] auto lock_shared() const -> SharedLockGuard<T, M_> {
    return {mutex_, data_};
  }

  template <SharedLockable M_ = M>
  void lock_shared(auto&& func) const {
    auto const guard = lock_shared();
    std::forward<decltype(func)>(func)(*guard);
  }

private:
  mutable M mutex_;
  T data_;
};

template <typename T>
using RecursiveMutex = Mutex<T, std::recursive_mutex>;

template <typename T>
using SharedMutex = Mutex<T, std::shared_mutex>;

} // namespace crust
