#pragma once

#include "principal_id.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::goals
{

using ::sovereign::PrincipalId;

struct GoalId
{
    std::array<std::uint8_t, 16> bytes{};
};

inline bool operator==(const GoalId& a, const GoalId& b) noexcept
{
    return a.bytes == b.bytes;
}

enum class GoalStatus : std::uint8_t
{
    Active = 1,
    Paused = 2,
    Completed = 3,
    Cancelled = 4
};

enum class GoalHorizon : std::uint8_t
{
    Day = 1,
    Week = 2,
    Month = 3,
    Year = 4,
    Life = 5
};

struct Goal
{
    GoalId id{};
    PrincipalId owner{};
    std::string title;
    std::string description;
    GoalStatus status{GoalStatus::Active};
    GoalHorizon horizon{GoalHorizon::Month};
    std::string workspace_name; // optional domain binding
    int priority{50};           // 0-100, higher = more important
    std::uint64_t created_unix_seconds{0};
    std::uint64_t updated_unix_seconds{0};
};

enum class GoalsError : std::uint8_t
{
    Ok = 0,
    NotFound,
    InvalidArgument,
    StorageFailure
};

template <typename T>
struct Result
{
    GoalsError error{GoalsError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == GoalsError::Ok;
    }
};

template <>
struct Result<void>
{
    GoalsError error{GoalsError::Ok};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == GoalsError::Ok;
    }
};

} // namespace sovereign::goals
