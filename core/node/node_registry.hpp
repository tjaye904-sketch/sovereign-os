#pragma once

#include "node_types.hpp"

#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

namespace sovereign::node
{

// L2.1 registry + L2.2 heartbeat + L2.3 workspace bind + L2.4 capability projection
class NodeRegistry
{
public:
    NodeError Open(const std::filesystem::path& data_root);
    void Close() noexcept;

    Result<NodeId> RegisterLocal(const std::string& name);
    Result<NodeId> RegisterPeer(const std::string& name, const std::string& endpoint);
    Result<NodeRecord> Get(const NodeId& id) const;
    Result<NodeRecord> FindByName(const std::string& name) const;
    Result<std::vector<NodeRecord>> List() const;

    // L2.2
    Result<void> Heartbeat(const NodeId& id, NodeHealth health = NodeHealth::Healthy);
    Result<void> MarkStale(std::uint64_t max_age_seconds);

    // L2.3
    Result<void> BindWorkspace(const NodeId& id, const std::string& workspace_name);
    Result<void> UnbindWorkspace(const NodeId& id);
    Result<std::vector<NodeRecord>> ListByWorkspace(const std::string& workspace_name) const;

    // L2.4
    Result<void> SetCapabilities(const NodeId& id, const std::vector<std::string>& caps);
    Result<bool> NodeHasCapability(const NodeId& id, const std::string& cap_id) const;
    Result<PlanRouteDecision> RouteWorkspaceAction(const std::string& workspace_name, const std::string& action, const std::string& required_cap_id) const;
    Result<NodeExecutionReport> ExecuteOnBoundNode(const std::string& workspace_name, const std::string& action, const std::string& required_cap_id, const std::string& payload);

    // L2.5 — append-only lifecycle events under data_root/node/events.dat
    Result<void> AppendEvent(const std::string& kind, const std::string& detail);
    Result<std::vector<std::string>> RecentEvents(std::size_t limit = 20) const;

    static std::string IdHex(const NodeId& id);
    static bool IdFromHex(const std::string& hex, NodeId& out);

private:
    NodeError Load();
    NodeError Save() const;
    static NodeId NewId();
    static std::uint64_t Now();

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path dir_;
    std::vector<NodeRecord> nodes_;
    std::vector<std::string> events_;
};

} // namespace sovereign::node
