#include "uuid.hpp"

#include <array>
#include <iomanip>
#include <random>
#include <sstream>

namespace sovereign
{

namespace
{

[[nodiscard]]
constexpr int HexValue(char character) noexcept
{
    if (character >= '0' && character <= '9')
    {
        return character - '0';
    }

    if (character >= 'a' && character <= 'f')
    {
        return character - 'a' + 10;
    }

    if (character >= 'A' && character <= 'F')
    {
        return character - 'A' + 10;
    }

    return -1;
}

} // namespace

Uuid::Uuid(Storage bytes) noexcept
    :
    bytes_(bytes)
{
}

Uuid Uuid::Generate()
{
    static thread_local std::random_device random_device;
    static thread_local std::mt19937_64 generator(
        random_device());

    std::uniform_int_distribution<unsigned int>
        distribution(0U, 255U);

    Storage bytes{};

    for (auto& byte : bytes)
    {
        byte = static_cast<std::uint8_t>(
            distribution(generator));
    }

    // RFC 4122 version 4.
    bytes[6] =
        static_cast<std::uint8_t>(
            (bytes[6] & 0x0FU) | 0x40U);

    // RFC 4122 variant.
    bytes[8] =
        static_cast<std::uint8_t>(
            (bytes[8] & 0x3FU) | 0x80U);

    return Uuid(bytes);
}

Uuid Uuid::Nil() noexcept
{
    return Uuid{};
}

std::optional<Uuid> Uuid::Parse(
    std::string_view value) noexcept
{
    if (value.size() != 36U)
    {
        return std::nullopt;
    }

    constexpr std::array<std::size_t, 4>
        hyphen_positions{8U, 13U, 18U, 23U};

    for (const auto position : hyphen_positions)
    {
        if (value[position] != '-')
        {
            return std::nullopt;
        }
    }

    Storage bytes{};
    std::size_t byte_index = 0U;

    for (std::size_t index = 0U;
         index < value.size();)
    {
        if (value[index] == '-')
        {
            ++index;
            continue;
        }

        if (index + 1U >= value.size() ||
            byte_index >= bytes.size())
        {
            return std::nullopt;
        }

        const int high =
            HexValue(value[index]);

        const int low =
            HexValue(value[index + 1U]);

        if (high < 0 || low < 0)
        {
            return std::nullopt;
        }

        bytes[byte_index] =
            static_cast<std::uint8_t>(
                (high << 4) | low);

        ++byte_index;
        index += 2U;
    }

    if (byte_index != bytes.size())
    {
        return std::nullopt;
    }

    return Uuid(bytes);
}

bool Uuid::IsNil() const noexcept
{
    for (const auto byte : bytes_)
    {
        if (byte != 0U)
        {
            return false;
        }
    }

    return true;
}

std::string Uuid::ToString() const
{
    std::ostringstream stream;

    stream
        << std::hex
        << std::setfill('0');

    for (std::size_t index = 0U;
         index < bytes_.size();
         ++index)
    {
        stream
            << std::setw(2)
            << static_cast<unsigned int>(
                bytes_[index]);

        if (index == 3U ||
            index == 5U ||
            index == 7U ||
            index == 9U)
        {
            stream << '-';
        }
    }

    return stream.str();
}

const Uuid::Storage&
Uuid::Bytes() const noexcept
{
    return bytes_;
}

} // namespace sovereign
