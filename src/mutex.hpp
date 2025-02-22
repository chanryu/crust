#pragma once

#include <mutex>
#include <shared_mutex>

namespace rscpp {

namespace detail {

// Concept to check if a Mutex has exclusive locking capabilities
template <typename Mutex>
concept Lockable = requires(Mutex m) {
  m.lock();
  m.unlock();
};

// Concept to check if a Mutex has shared locking capabilities
template <typename Mutex>
concept SharedLockable = requires(Mutex m) {
  m.lock_shared();
  m.unlock_shared();
};

// Exclusive Lock Guard
template <Lockable Mutex, typename Data>
class LockGuard final {
public:
  LockGuard(Mutex& mutex, Data& data) : mutex_{mutex}, data_{data} {
    mutex_.lock();
  }
  ~LockGuard() {
    mutex_.unlock();
  }

  auto operator->() -> Data* {
    return &data_;
  }
  auto operator*() -> Data& {
    return data_;
  }

private:
  Mutex& mutex_;
  Data& data_;
};

// Shared Lock Guard
template <SharedLockable Mutex, typename Data>
class SharedLockGuard final {
public:
  SharedLockGuard(Mutex& mutex, Data const& data) : mutex_{mutex}, data_{data} {
    mutex_.lock_shared();
  }
  ~SharedLockGuard() {
    mutex_.unlock_shared();
  }

  auto operator->() const -> Data const* {
    return &data_;
  }
  auto operator*() const -> Data const& {
    return data_;
  }

private:
  Mutex& mutex_;
  Data const& data_;
};

} // namespace detail

// BasicMutex implementation
template <detail::Lockable Mutex, typename Data>
class BasicMutex {
public:
  explicit BasicMutex(Data&& data) : data_{std::move(data)} {
  }

  explicit BasicMutex(Data const& data) : data_{data} {
  }

  // Non-copyable and non-movable
  BasicMutex(BasicMutex const&) = delete;
  BasicMutex& operator=(BasicMutex const&) = delete;

  [[nodiscard]] auto lock() -> detail::LockGuard<Mutex, Data> {
    return {mutex_, data_};
  }

  void lock(auto&& func) {
    auto guard = lock();
    std::forward<decltype(func)>(func)(*guard);
  }

  template <detail::SharedLockable M = Mutex>
  [[nodiscard]] auto lock_shared() const -> detail::SharedLockGuard<M, Data> {
    return {mutex_, data_};
  }

  template <detail::SharedLockable M = Mutex>
  void lock_shared(auto&& func) const {
    auto guard = lock_shared();
    std::forward<decltype(func)>(func)(*guard);
  }

private:
  mutable Mutex mutex_;
  Data data_;
};

// Type aliases
template <typename Data>
using Mutex = BasicMutex<std::mutex, Data>;

template <typename Data>
using RecursiveMutex = BasicMutex<std::recursive_mutex, Data>;

template <typename Data>
using SharedMutex = BasicMutex<std::shared_mutex, Data>;

} // namespace rscpp
