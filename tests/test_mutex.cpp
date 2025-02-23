#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "mutex.hpp"

namespace crust {
namespace {

TEST(MutexTest, ExclusiveLock) {
  Mutex<int> mutex{42};
  mutex.lock([](int& data) {
    data = 100;
  });
  int value = 0;
  mutex.lock([&value](int& data) {
    value = data;
  });
  EXPECT_EQ(value, 100);
}

TEST(MutexTest, MultiThreadedExclusiveLock) {
  Mutex<int> mutex{0};
  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&]() {
      mutex.lock([](int& data) {
        ++data;
      });
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
  int value = 0;
  mutex.lock([&value](int& data) {
    value = data;
  });
  EXPECT_EQ(value, 10);
}

TEST(SharedMutexTest, SharedLock) {
  SharedMutex<int> mutex{42};
  int value = 0;
  mutex.lock_shared([&value](const int& data) {
    value = data;
  });
  EXPECT_EQ(value, 42);
}

TEST(SharedMutexTest, MultiThreadedSharedLock) {
  SharedMutex<int> mutex{42};
  std::vector<std::thread> threads;
  std::vector<int> results(10, 0);
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&, i]() {
      mutex.lock_shared([&](const int& data) {
        results[i] = data;
      });
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
  for (const auto& result : results) {
    EXPECT_EQ(result, 42);
  }
}

TEST(MutexTest, LockGuard) {
  Mutex<int> mutex{42};
  {
    auto guard = mutex.lock();
    *guard = 100;
  }
  auto guard = mutex.lock();
  EXPECT_EQ(*guard, 100);
}

TEST(SharedMutexTest, SharedLockGuard) {
  SharedMutex<int> mutex{42};
  {
    auto guard = mutex.lock_shared();
    EXPECT_EQ(*guard, 42);
  }
}

} // namespace
} // namespace crust
