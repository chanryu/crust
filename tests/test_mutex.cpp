#include <chrono>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "mutex.hpp"

using namespace std::chrono_literals;

namespace crust {
namespace {

TEST(MutexTest, ExclusiveLock) {
  Mutex<int> mutex{42};
  mutex.lock([](int& data) {
    data = 100;
  });
  auto value = mutex.lock();
  EXPECT_EQ(*value, 100);
}

TEST(MutexTest, MultiThreadedExclusiveLock) {
  Mutex<int> mutex{0};
  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&]() {
      mutex.lock([](auto& data) {
        ++data;
      });
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }

  auto value = mutex.lock();
  EXPECT_EQ(*value, 10);
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
  SharedMutex<std::pair<int, int>> mutex{{0, 0}};
  std::vector<std::thread> threads;

  // producer - update first and second with 10ms interval
  threads.emplace_back([&]() {
    mutex.lock([](auto& pair) {
      for (int i = 0; i < 10; ++i) {
        pair.first += 1;
        std::this_thread::sleep_for(10ms);
        pair.second = pair.first;
      }
    });
  });

  // consumers - read first and second with 2ms interval
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&]() {
      mutex.lock_shared([](const auto& pair) {
        EXPECT_EQ(pair.first, pair.second);
        std::this_thread::sleep_for(2ms);
      });
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
}

} // namespace
} // namespace crust
