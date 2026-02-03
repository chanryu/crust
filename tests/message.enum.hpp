// Auto-generated. Do not edit by hand.

#pragma once

#include <string>
#include <utility>
#include <variant>

namespace crust::test {

struct Message;

namespace detail {
struct MessageTags {
    struct Quit {
        using Variant = Message;
        constexpr Quit() = default;
    };

    struct Move {
        using Variant = Message;
        int x;
        int y;

        constexpr Move() = default;
        constexpr Move(int x, int y) : x(std::move(x)), y(std::move(y)) {}
    };

    struct Write {
        using Variant = Message;
        std::string text;

        constexpr Write() = default;
        constexpr Write(std::string text) : text(std::move(text)) {}
    };
};
} // namespace detail

// ===== Message =====
struct Message : std::variant<detail::MessageTags::Quit, detail::MessageTags::Move, detail::MessageTags::Write> {
    using Base = std::variant<detail::MessageTags::Quit, detail::MessageTags::Move, detail::MessageTags::Write>;
    using Base::Base;

    constexpr Message() = default;
    constexpr Message(const Message&) = default;
    constexpr Message(Message&&) = default;
    constexpr Message& operator=(const Message&) = default;
    constexpr Message& operator=(Message&&) = default;

    // Re-export cases so you can write Message::Case
    using Quit = detail::MessageTags::Quit;
    using Move = detail::MessageTags::Move;
    using Write = detail::MessageTags::Write;

    // Convenience constructors: Message v = Message::Case{...};
    constexpr Message(Quit v) : Base(std::move(v)) {}
    constexpr Message(Move v) : Base(std::move(v)) {}
    constexpr Message(Write v) : Base(std::move(v)) {}
};

} // namespace crust::test
