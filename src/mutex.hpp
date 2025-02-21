#pragma once

#include <mutex>
#include <shared_mutex>

namespace rscpp {

namespace detail {

// Concept to check if a Mutex has shared locking capabilities
template <typename Mutex>
concept Lockable = requires(Mutex m) {
  { m.lock() };
  { m.unlock() };
};

template <typename Mutex>
concept SharedLockable = requires(Mutex m) {
  { m.lock_shared() };
  { m.unlock_shared() };
};

template <typename Mutex, typename Data>
  requires detail::Lockable<Mutex>
class LockGuard final {
public:
  LockGuard(Mutex& mutex, Data& data) : mutex_{mutex}, data_{data} {
    mutex_.lock();
  }

  ~LockGuard() {
    mutex_.unlock();
  }

  Data* operator->() {
    return &data_;
  }
  Data& operator*() {
    return data_;
  }

private:
  Mutex& mutex_;
  Data& data_;
};

template <typename Mutex, typename Data>
  requires detail::SharedLockable<Mutex>
class SharedLockGuard final {
public:
  SharedLockGuard(Mutex& mutex, const Data& data) : mutex_{mutex}, data_{data} {
    mutex_.lock_shared();
  }

  ~SharedLockGuard() {
    mutex_.unlock_shared();
  }

  const Data* operator->() const {
    return &data_;
  }
  const Data& operator*() const {
    return data_;
  }

private:
  Mutex& mutex_;
  const Data& data_;
};

} // namespace detail

template <typename Mutex, typename Data>
  requires detail::Lockable<Mutex>
class BasicMutex {
public:
  explicit BasicMutex(Data&& data) : data_{std::move(data)} {
  }

  explicit BasicMutex(const Data& data) : data_{data} {
  }

  // Non-copyable and non-movable
  BasicMutex(const BasicMutex&) = delete;
  BasicMutex& operator=(const BasicMutex&) = delete;

  auto lock() {
    return detail::LockGuard<Mutex, Data>{mutex_, data_};
  }

  template <typename Func>
  void lock(Func&& func) {
    std::forward<Func>(func)(*lock());
  }

  auto lock_shared() const
    requires detail::SharedLockable<Mutex>
  {
    return detail::SharedLockGuard<Mutex, Data>{mutex_, data_};
  }

  template <typename Func>
    requires detail::SharedLockable<Mutex>
  void lock_shared(Func&& func) const {
    std::forward<Func>(func)(*lock_shared());
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
