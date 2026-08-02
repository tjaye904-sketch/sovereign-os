#pragma once

#include "uuid.hpp"

#include <compare>
#include <utility>

namespace sovereign
{

template<typename Tag>
class Identifier final
{
public:
    Identifier() = default;

    explicit Identifier(Uuid value) noexcept
        :
        value_(std::move(value))
    {
    }

    [[nodiscard]]
    static Identifier Generate()
    {
        return Identifier(Uuid::Generate());
    }

    [[nodiscard]]
    static Identifier Nil() noexcept
    {
        return Identifier(Uuid::Nil());
    }

    [[nodiscard]]
    bool IsNil() const noexcept
    {
        return value_.IsNil();
    }

    [[nodiscard]]
    const Uuid& Value() const noexcept
    {
        return value_;
    }

    [[nodiscard]]
    std::string ToString() const
    {
        return value_.ToString();
    }

    auto operator<=>(const Identifier&) const = default;

private:
    Uuid value_;
};

} // namespace sovereign
