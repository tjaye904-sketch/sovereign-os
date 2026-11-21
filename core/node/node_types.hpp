#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::node
{

using NodeId = std::array<std::uint8_t, 16>;

enum class NodeKind : std::uint8_t
{
    LocalHost = 1,
    Peer = 2,
    WorkspaceBound = 3,
};

enum class NodeHealth : std::uint8_t
{
    Unknown = 0,
    Healthy = 1,
    Degraded = 2,
    Offline = 3,
};

enum class NodeError : std::uint8_t
{
    Ok = 0,
    NotFound,
    AlreadyExists,
    InvalidArgument,
    StorageFailure,
    NotOpen,
};

template <typename T>
struct Result
{
    NodeError error{NodeError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept { return error == NodeError::Ok; }
};

template <>
struct Result<void>
{
    NodeError error{NodeError::Ok};
    [[nodiscard]] bool Ok() const noexcept { return error == NodeError::Ok; }
    static Result OkRes() { return {}; }
    static Result Err(NodeError e) { return Result{e}; }
};

struct NodeRecord
{
    NodeId id{};
    std::string name;
    NodeKind kind{NodeKind::LocalHost};
    NodeHealth health{NodeHealth::Unknown};
    std::uint64_t last_heartbeat_unix{0};
    std::string endpoint; // optional host:port or local
    std::string workspace_name; // L2.3 binding (empty = unbound)
    std::vector<std::string> capabilities; // L2.4 projected capability ids
};

inline const char* KindName(NodeKind k)
{
    switch (k)
    {
    case NodeKind::LocalHost: return "local_host";
    case NodeKind::Peer: return "peer";
    case NodeKind::WorkspaceBound: return "workspace_bound";
    }
    return "unknown";
}

inline const char* HealthName(NodeHealth h)
{
    switch (h)
    {
    case NodeHealth::Unknown: return "unknown";
    case NodeHealth::Healthy: return "healthy";
    case NodeHealth::Degraded: return "degraded";
    case NodeHealth::Offline: return "offline";
    }
    return "unknown";
}

} // namespace sovereign::node
