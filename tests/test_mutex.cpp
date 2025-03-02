#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "mutex.hpp"

using namespace std::chrono_literals;

namespace crust {
namespace {

// Test fixture for Mutex tests
class MutexTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Basic test for Mutex construction and data access
TEST_F(MutexTest, BasicConstruction) {
  Mutex<int> mutex_int(42);

  {
    auto guard = mutex_int.lock();
    EXPECT_EQ(*guard, 42);
    *guard = 100;
  }

  {
    auto guard = mutex_int.lock();
    EXPECT_EQ(*guard, 100);
  }
}

// Test move construction and assignment
TEST_F(MutexTest, MoveOperations) {
  Mutex<std::string> mutex_str("test");

  // Test move construction
  Mutex<std::string> moved_mutex(std::move(mutex_str));

  {
    auto guard = moved_mutex.lock();
    EXPECT_EQ(*guard, "test");
  }

  // Test move assignment
  Mutex<std::string> assigned_mutex("");
  assigned_mutex = std::move(moved_mutex);

  {
    auto guard = assigned_mutex.lock();
    EXPECT_EQ(*guard, "test");
  }
}

// Test in-place construction
TEST_F(MutexTest, InPlaceConstruction) {
  struct TestStruct {
    int a;
    std::string b;

    TestStruct(int a_, std::string b_) : a(a_), b(std::move(b_)) {}
  };

  Mutex<TestStruct> mutex(10, "hello");

  {
    auto guard = mutex.lock();
    EXPECT_EQ(guard->a, 10);
    EXPECT_EQ(guard->b, "hello");
  }
}

// Test concurrent access to mutex
TEST_F(MutexTest, ConcurrentAccess) {
  Mutex<int> counter(0);
  std::atomic<bool> ready(false);
  constexpr int num_threads = 10;
  constexpr int iterations = 1000;

  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&counter, &ready] {
      // Wait until all threads are ready
      while (!ready.load()) {
        std::this_thread::yield();
      }

      for (int j = 0; j < iterations; ++j) {
        counter.lock([](int& value) {
          return ++value;
        });
      }
    });
  }

  // Start all threads simultaneously
  ready.store(true);

  // Join all threads
  for (auto& thread : threads) {
    thread.join();
  }

  // Check final counter value
  {
    auto guard = counter.lock();
    EXPECT_EQ(*guard, num_threads * iterations);
  }
}

// Test for SharedMutex
TEST_F(MutexTest, SharedMutex) {
  SharedMutex<std::string> shared_data("shared text");
  std::atomic<int> readers_done(0);
  std::atomic<bool> writer_done(false);
  std::atomic<bool> start(false);

  // Start multiple reader threads
  std::vector<std::thread> reader_threads;
  constexpr int num_readers = 5;

  for (int i = 0; i < num_readers; ++i) {
    reader_threads.emplace_back([&shared_data, &start, &readers_done] {
      while (!start.load()) {
        std::this_thread::yield();
      }

      // Read the shared data
      {
        auto guard = shared_data.lock_shared();
        // Just verify we can read the data
        EXPECT_FALSE(guard->empty());
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }

      readers_done.fetch_add(1);
    });
  }

  // Start a writer thread
  std::thread writer_thread([&shared_data, &start, &readers_done, &writer_done] {
    while (!start.load()) {
      std::this_thread::yield();
    }

    // Wait until readers have started
    std::this_thread::sleep_for(10ms);

    // Modify the shared data
    {
      auto guard = shared_data.lock();
      *guard = "modified shared text";
      // Simulate some work
      std::this_thread::sleep_for(100ms);
    }

    writer_done.store(true);
  });

  // Start all threads
  start.store(true);

  // Join all threads
  for (auto& thread : reader_threads) {
    thread.join();
  }
  writer_thread.join();

  // Verify all readers completed
  EXPECT_EQ(readers_done.load(), num_readers);
  EXPECT_TRUE(writer_done.load());

  // Verify the data was modified
  {
    auto guard = shared_data.lock_shared();
    EXPECT_EQ(*guard, "modified shared text");
  }
}

// Test for ScopedLockGuard with multiple mutexes
TEST_F(MutexTest, ScopedLockGuard) {
  Mutex<int> mutex1(1);
  Mutex<std::string> mutex2("two");
  Mutex<double> mutex3(3.0);

  {
    auto guard = scoped_lock(mutex1, mutex2, mutex3);

    // Test access by index
    EXPECT_EQ(guard.get<0>(), 1);
    EXPECT_EQ(guard.get<1>(), "two");
    EXPECT_EQ(guard.get<2>(), 3.0);

    // Modify the values
    guard.get<0>() = 10;
    guard.get<1>() = "twenty";
    guard.get<2>() = 30.0;
  }

  // Verify changes persisted
  {
    auto guard1 = mutex1.lock();
    EXPECT_EQ(*guard1, 10);
  }
  {
    auto guard2 = mutex2.lock();
    EXPECT_EQ(*guard2, "twenty");
  }
  {
    auto guard3 = mutex3.lock();
    EXPECT_EQ(*guard3, 30.0);
  }
}

// Test for with_scoped_lock helper function
TEST_F(MutexTest, WithScopedLock) {
  Mutex<int> mutex1(1);
  Mutex<std::string> mutex2("two");

  // Use with_scoped_lock to perform an operation on multiple mutexes
  auto result = with_scoped_lock(
      [](int& a, std::string& b) {
        a += 10;
        b += " modified";
        return a * 2;
      },
      mutex1, mutex2);

  // Check the result of the lambda
  EXPECT_EQ(result, 22); // (1 + 10) * 2 = 22

  // Verify the modifications were applied
  {
    auto guard1 = mutex1.lock();
    EXPECT_EQ(*guard1, 11);
  }
  {
    auto guard2 = mutex2.lock();
    EXPECT_EQ(*guard2, "two modified");
  }
}

// Test for RecursiveMutex
TEST_F(MutexTest, RecursiveMutex) {
  RecursiveMutex<int> recursive_mutex(0);

  // Define a recursive function that acquires the lock multiple times
  std::function<void(int)> recursive_func;
  recursive_func = [&recursive_mutex, &recursive_func](int depth) {
    if (depth == 0)
      return;

    auto guard = recursive_mutex.lock();
    (*guard)++;

    // Recursive call with the lock held
    recursive_func(depth - 1);
  };

  // Call with depth of 5
  recursive_func(5);

  // Verify the counter was incremented 5 times
  {
    auto guard = recursive_mutex.lock();
    EXPECT_EQ(*guard, 5);
  }
}

// Test for deadlock prevention (this is a bit tricky to test thoroughly)
TEST_F(MutexTest, DeadlockPrevention) {
  Mutex<int> mutex1(1);
  Mutex<int> mutex2(2);

  // This tests that we can lock mutexes in different orders without deadlock
  // This is possible because scoped_lock acquires all locks at once in an implementation-defined order
  bool success = true;

  std::thread t1([&mutex1, &mutex2, &success] {
    try {
      auto guard = scoped_lock(mutex1, mutex2);
      std::this_thread::sleep_for(50ms);
      guard.get<0>() += 10;
      guard.get<1>() += 20;
    }
    catch (...) {
      success = false;
    }
  });

  std::thread t2([&mutex1, &mutex2, &success] {
    try {
      // Intentionally reverse order from t1
      std::this_thread::sleep_for(10ms);
      auto guard = scoped_lock(mutex2, mutex1);
      guard.get<0>() += 5;
      guard.get<1>() += 5;
    }
    catch (...) {
      success = false;
    }
  });

  t1.join();
  t2.join();

  EXPECT_TRUE(success);

  // Check final values (exact values depend on execution order)
  int val1, val2;
  {
    auto guard = mutex1.lock();
    val1 = *guard;
  }
  {
    auto guard = mutex2.lock();
    val2 = *guard;
  }

  // Both mutexes should have been incremented by the sum of the increments
  EXPECT_EQ(val1, 1 + 10 + 5);
  EXPECT_EQ(val2, 2 + 20 + 5);
}

} // namespace
} // namespace crust
