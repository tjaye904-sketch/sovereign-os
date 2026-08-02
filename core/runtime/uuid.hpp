#pragma once

#include <array>
#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace sovereign
{

class Uuid final
{
public:
    using Storage =
        std::array<std::uint8_t, 16>;

    Uuid() = default;

    explicit Uuid(Storage bytes) noexcept;

    [[nodiscard]]
    static Uuid Generate();

    [[nodiscard]]
    static Uuid Nil() noexcept;

    [[nodiscard]]
    static std::optional<Uuid> Parse(
        std::string_view value) noexcept;

    [[nodiscard]]
    bool IsNil() const noexcept;

    [[nodiscard]]
    std::string ToString() const;

    [[nodiscard]]
    const Storage& Bytes() const noexcept;

    auto operator<=>(const Uuid&) const = default;

private:
    Storage bytes_{};
};

} // namespace sovereign
