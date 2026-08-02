#pragma once

#include "principal_id.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::memory
{

using ::sovereign::PrincipalId;

struct RecordId
{
    std::array<std::uint8_t, 16> bytes{};
};

inline bool operator==(const RecordId& a, const RecordId& b) noexcept
{
    return a.bytes == b.bytes;
}

struct MemoryRecord
{
    RecordId id{};
    PrincipalId author{};
    std::uint64_t created_unix_seconds{0};
    std::vector<std::string> tags;
    std::string body;
    // Phase V
    double importance{0.5}; // 0..1
    std::uint64_t last_access_unix_seconds{0};
    std::uint32_t access_count{0};
};

struct ScoredRecord
{
    MemoryRecord record;
    double score{0}; // aged importance
};

enum class MemoryError : std::uint8_t
{
    Ok = 0,
    NotFound,
    InvalidArgument,
    StorageFailure
};

template <typename T>
struct Result
{
    MemoryError error{MemoryError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == MemoryError::Ok;
    }
};

template <>
struct Result<void>
{
    MemoryError error{MemoryError::Ok};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == MemoryError::Ok;
    }
};

} // namespace sovereign::memory
