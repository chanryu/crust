#include <functional>
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

TEST(CowTest, CowCopyAssignment) {
  auto cow1 = Cow<int>{42};
  auto cow2 = Cow<int>{};
  cow2 = cow1;
  EXPECT_EQ(cow2.get(), 42);
}

TEST(CowTest, CowMoveAssignment) {
  auto cow1 = Cow<int>{42};
  auto cow2 = Cow<int>{};
  cow2 = std::move(cow1);
  EXPECT_EQ(cow2.get(), 42);
}

TEST(CowTest, ValueCopyAssignment) {
  auto cow = Cow<int>{};
  cow = 42;
  EXPECT_EQ(cow.get(), 42);
}

TEST(CowTest, ValueMoveAssignment) {
  auto cow = Cow<int>{};
  auto value = 42;
  cow = std::move(value);
  EXPECT_EQ(cow.get(), 42);
}

TEST(CowTest, MutateWithReturn) {
  auto cow = Cow<int>{42};
  auto result = cow.mutate([](int& value) {
    value = 100;
    return value * 2;
  });
  EXPECT_EQ(cow.get(), 100);
  EXPECT_EQ(result, 200);
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

  // Modifying the clone shouldn't affect the original
  clone.mutate([](int& value) {
    value = 100;
  });
  EXPECT_EQ(cow.get(), 42);
  EXPECT_EQ(clone.get(), 100);
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

TEST(CowTest, Equality) {
  auto cow1 = Cow<int>{42};
  auto cow2 = Cow<int>{42};
  auto cow3 = Cow<int>{100};

  EXPECT_TRUE(cow1 == cow2);
  EXPECT_FALSE(cow1 == cow3);
}

TEST(CowTest, MakeCow) {
  auto cow = make_cow<std::string>("Hello, World!");
  EXPECT_EQ(cow.get(), "Hello, World!");
}

} // namespace

} // namespace crust