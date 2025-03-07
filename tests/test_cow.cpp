#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "cow.hpp"

namespace crust {
namespace {

class CowTest : public ::testing::Test {
protected:
  using StringCow = Cow<std::string>;
  using VectorCow = Cow<std::vector<int>>;
};

// Test constructors
TEST_F(CowTest, DefaultConstructor) {
  StringCow cow;
  EXPECT_TRUE(cow.is_unique());
  EXPECT_EQ(*cow, "");
}

TEST_F(CowTest, ValueConstructor) {
  StringCow cow("test");
  EXPECT_TRUE(cow.is_unique());
  EXPECT_EQ(*cow, "test");
}

// Test copy/move operations
TEST_F(CowTest, CopyConstructor) {
  StringCow cow1("original");
  StringCow cow2(cow1);

  EXPECT_FALSE(cow1.is_unique());
  EXPECT_FALSE(cow2.is_unique());
  EXPECT_EQ(*cow1, *cow2);
}

TEST_F(CowTest, MoveConstructor) {
  StringCow cow1("original");
  StringCow cow2(std::move(cow1));

  EXPECT_TRUE(cow2.is_unique());
  EXPECT_EQ(*cow2, "original");
}

TEST_F(CowTest, CopyAssignment) {
  StringCow cow1("original");
  StringCow cow2;
  cow2 = cow1;

  EXPECT_FALSE(cow1.is_unique());
  EXPECT_FALSE(cow2.is_unique());
  EXPECT_EQ(*cow1, *cow2);
}

TEST_F(CowTest, MoveAssignment) {
  StringCow cow1("original");
  StringCow cow2;
  cow2 = std::move(cow1);

  EXPECT_TRUE(cow2.is_unique());
  EXPECT_EQ(*cow2, "original");
}

// Test value assignment
TEST_F(CowTest, ValueAssignment) {
  StringCow cow;
  cow = "new value";

  EXPECT_TRUE(cow.is_unique());
  EXPECT_EQ(*cow, "new value");

  std::string rvalue = "rvalue test";
  cow = std::move(rvalue);
  EXPECT_TRUE(cow.is_unique());
  EXPECT_EQ(*cow, "rvalue test");
}

// Test copy-on-write behavior
TEST_F(CowTest, CopyOnWriteBehavior) {
  VectorCow cow1({1, 2, 3});
  VectorCow cow2 = cow1;

  EXPECT_FALSE(cow1.is_unique());
  EXPECT_FALSE(cow2.is_unique());

  // Modify cow2, should trigger copy-on-write
  cow2.mutate([](std::vector<int>& v) {
    v.push_back(4);
  });

  EXPECT_TRUE(cow1.is_unique());
  EXPECT_TRUE(cow2.is_unique());
  EXPECT_EQ(cow1->size(), 3);
  EXPECT_EQ(cow2->size(), 4);
}

// Test methods
TEST_F(CowTest, GetMethod) {
  StringCow cow("test");
  const auto& str = cow.get();
  EXPECT_EQ(str, "test");
}

TEST_F(CowTest, MutateMethod) {
  StringCow cow("test");
  cow.mutate([](std::string& s) {
    s += " modified";
  });
  EXPECT_EQ(*cow, "test modified");
}

TEST_F(CowTest, CloneMethod) {
  StringCow cow1("original");
  StringCow cow2 = cow1;
  StringCow cow3 = cow2.clone();

  EXPECT_FALSE(cow1.is_unique());
  EXPECT_FALSE(cow2.is_unique());
  EXPECT_TRUE(cow3.is_unique());

  EXPECT_EQ(*cow1, *cow3);
}

TEST_F(CowTest, SwapMethod) {
  StringCow cow1("first");
  StringCow cow2("second");

  cow1.swap(cow2);

  EXPECT_EQ(*cow1, "second");
  EXPECT_EQ(*cow2, "first");
}

TEST_F(CowTest, ReleaseMethod) {
  StringCow cow("test");
  auto ptr = std::move(cow).release();

  EXPECT_EQ(*ptr, "test");
  EXPECT_EQ(ptr.use_count(), 1);
}

// Test operators
TEST_F(CowTest, DereferenceOperator) {
  StringCow cow("test");
  EXPECT_EQ(*cow, "test");
}

TEST_F(CowTest, ArrowOperator) {
  StringCow cow("test");
  EXPECT_EQ(cow->size(), 4);
}

// Test use_count behavior
TEST_F(CowTest, UseCount) {
  StringCow cow1("test");
  EXPECT_EQ(cow1.is_unique(), true);

  auto cow2 = cow1;
  EXPECT_EQ(cow1.is_unique(), false);

  cow2 = "different";
  EXPECT_EQ(cow1.is_unique(), true);
  EXPECT_EQ(cow2.is_unique(), true);
}
} // namespace

} // namespace crust