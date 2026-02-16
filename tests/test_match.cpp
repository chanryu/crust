#include <crust/match.hpp>
#include <gtest/gtest.h>

#include "message.enum.hpp"

namespace crust::test {
namespace {

TEST(MatchTest, match_variant) {
  Message msg = Message::Move{10, 20};

  static_assert(std::is_same_v<decltype(msg), Message>);

  match(
      msg,
      [](const Message::Quit&) {
        FAIL() << "Should not match Quit";
      },
      [](const Message::Move& m) {
        EXPECT_EQ(m.x, 10);
        EXPECT_EQ(m.y, 20);
      },
      [](const Message::Write&) {
        FAIL() << "Should not match Write";
      });
}

TEST(MatchTest, match_case) {
  auto msg = Message::Write{"test message"};

  static_assert(std::is_same_v<decltype(msg), Message::Write>);

  match(
      msg,
      [](const Message::Quit&) {
        FAIL() << "Should not match Quit";
      },
      [](const Message::Move&) {
        FAIL() << "Should not match Move";
      },
      [](const Message::Write& m) {
        ASSERT_EQ(m.text, "test message");
      });
}

} // namespace

} // namespace crust::test