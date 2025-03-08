#include <gtest/gtest.h>

#include "cow.hpp"

namespace crust {
namespace {

TEST(CowTest, DefaultConstructor) {
  auto cow = Cow<int>{};
  EXPECT_EQ(cow.get(), 0);
}

TEST(CowTest, CopyConstructor) {
  auto cow1 = Cow<int>{42};
  auto cow2 = cow1;
  EXPECT_EQ(cow2.get(), 42);
}

TEST(CowTest, MoveConstructor) {
  auto cow1 = Cow<int>{42};
  auto cow2 = std::move(cow1);
  EXPECT_EQ(cow2.get(), 42);
}

TEST(CowTest, CopyAssignment) {
  auto cow1 = Cow<int>{42};
  auto cow2 = Cow<int>{};
  cow2 = cow1;
  EXPECT_EQ(cow2.get(), 42);
}

TEST(CowTest, MoveAssignment) {
  auto cow1 = Cow<int>{42};
  auto cow2 = Cow<int>{};
  cow2 = std::move(cow1);
  EXPECT_EQ(cow2.get(), 42);
}

TEST(CowTest, Mutate) {
  auto cow = Cow<int>{42};
  cow.mutate([](int& value) {
    value = 100;
  });
  EXPECT_EQ(cow.get(), 100);
}

TEST(CowTest, IsUnique) {
  auto cow = Cow<int>{42};
  EXPECT_TRUE(cow.is_unique());
  auto cow2 = cow;
  EXPECT_FALSE(cow.is_unique());
}

TEST(CowTest, Clone) {
  auto cow = Cow<int>{42};
  auto clone = cow.clone();
  EXPECT_EQ(clone.get(), 42);
}

TEST(CowTest, Release) {
  auto cow = Cow<int>{42};
  auto released = std::move(cow).release();
  EXPECT_EQ(*released, 42);
}

TEST(CowTest, Swap) {
  auto cow1 = Cow<int>{42};
  auto cow2 = Cow<int>{100};
  cow1.swap(cow2);
  EXPECT_EQ(cow1.get(), 100);
  EXPECT_EQ(cow2.get(), 42);
}

TEST(CowTest, CopyOnWrite) {
  auto cow1 = Cow<int>{42};
  auto cow2 = cow1;

  EXPECT_EQ(cow1.get(), 42);
  EXPECT_EQ(cow2.get(), 42);

  cow1.mutate([](int& value) {
    value = 100;
  });

  EXPECT_EQ(cow1.get(), 100);
  EXPECT_EQ(cow2.get(), 42);
}

} // namespace

} // namespace crust