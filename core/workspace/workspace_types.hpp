#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::workspace
{

struct WorkspaceId
{
    std::array<std::uint8_t, 16> bytes{};
};

inline bool operator==(const WorkspaceId& a, const WorkspaceId& b) noexcept
{
    return a.bytes == b.bytes;
}

enum class WorkspaceStatus : std::uint8_t
{
    Active = 1,
    Archived = 2
};

struct Workspace
{
    WorkspaceId id{};
    std::string name;
    std::string kind; // e.g. development, knowledge
    WorkspaceStatus status{WorkspaceStatus::Active};
    std::uint64_t created_unix_seconds{0};
};

enum class WorkspaceError : std::uint8_t
{
    Ok = 0,
    NotFound,
    AlreadyExists,
    InvalidArgument,
    StorageFailure
};

template <typename T>
struct Result
{
    WorkspaceError error{WorkspaceError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == WorkspaceError::Ok;
    }
};

template <>
struct Result<void>
{
    WorkspaceError error{WorkspaceError::Ok};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == WorkspaceError::Ok;
    }
};

} // namespace sovereign::workspace
