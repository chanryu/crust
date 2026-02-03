#include <gtest/gtest.h>

#include "match.hpp"
#include "message.enum.hpp"

namespace crust {
namespace {

TEST(MatchTest, match) {
  Message msg = Message::Move{10, 20};

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

} // namespace

} // namespace crust