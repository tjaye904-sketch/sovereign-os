#include "node_registry.hpp"

#include <chrono>
#include <fstream>
#include <random>
#include <sstream>

namespace sovereign::node
{
namespace
{

std::string Esc(const std::string& s)
{
    std::string o;
    for (char c : s)
    {
        if (c == '|' || c == '\\' || c == '\n' || c == ',')
        {
            o.push_back('\\');
            if (c == '\n')
                o.push_back('n');
            else
                o.push_back(c);
        }
        else
            o.push_back(c);
    }
    return o;
}

std::string Unesc(const std::string& s)
{
    std::string o;
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '\\' && i + 1 < s.size())
        {
            ++i;
            o.push_back(s[i] == 'n' ? '\n' : s[i]);
        }
        else
            o.push_back(s[i]);
    }
    return o;
}

std::vector<std::string> SplitPipe(const std::string& line)
{
    std::vector<std::string> p;
    std::string cur;
    for (std::size_t i = 0; i < line.size(); ++i)
    {
        if (line[i] == '\\' && i + 1 < line.size())
        {
            cur.push_back(line[i++]);
            cur.push_back(line[i]);
        }
        else if (line[i] == '|')
        {
            p.push_back(Unesc(cur));
            cur.clear();
        }
        else
            cur.push_back(line[i]);
    }
    p.push_back(Unesc(cur));
    return p;
}

} // namespace

std::string NodeRegistry::IdHex(const NodeId& id)
{
    static constexpr char k[] = "0123456789abcdef";
    std::string o(32, '0');
    for (std::size_t i = 0; i < 16; ++i)
    {
        o[i * 2] = k[(id[i] >> 4) & 0xF];
        o[i * 2 + 1] = k[id[i] & 0xF];
    }
    return o;
}

bool NodeRegistry::IdFromHex(const std::string& h, NodeId& out)
{
    if (h.size() != 32)
        return false;
    auto n = [](char c) -> int {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < 16; ++i)
    {
        int hi = n(h[i * 2]), lo = n(h[i * 2 + 1]);
        if (hi < 0 || lo < 0)
            return false;
        out[i] = static_cast<std::uint8_t>((hi << 4) | lo);
    }
    return true;
}

NodeId NodeRegistry::NewId()
{
    NodeId id{};
    std::random_device rd;
    for (auto& x : id)
        x = static_cast<std::uint8_t>(rd());
    return id;
}

std::uint64_t NodeRegistry::Now()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

NodeError NodeRegistry::Open(const std::filesystem::path& data_root)
{
    std::lock_guard lock(mutex_);
    dir_ = data_root / "node";
    std::error_code ec;
    std::filesystem::create_directories(dir_, ec);
    if (ec)
        return NodeError::StorageFailure;
    open_ = true;
    return Load();
}

void NodeRegistry::Close() noexcept
{
    std::lock_guard lock(mutex_);
    if (open_)
        static_cast<void>(Save());
    open_ = false;
}

NodeError NodeRegistry::Load()
{
    nodes_.clear();
    events_.clear();
    {
        std::ifstream in(dir_ / "nodes.dat");
        std::string line;
        while (in && std::getline(in, line))
        {
            if (line.empty())
                continue;
            auto p = SplitPipe(line);
            if (p.size() < 7)
                continue;
            NodeRecord n;
            if (!IdFromHex(p[0], n.id))
                continue;
            n.name = p[1];
            n.kind = static_cast<NodeKind>(std::stoi(p[2]));
            n.health = static_cast<NodeHealth>(std::stoi(p[3]));
            n.last_heartbeat_unix = std::stoull(p[4]);
            n.endpoint = p[5];
            n.workspace_name = p[6];
            if (p.size() >= 8 && !p[7].empty())
            {
                std::stringstream ss(p[7]);
                std::string cap;
                while (std::getline(ss, cap, ','))
                {
                    if (!cap.empty())
                        n.capabilities.push_back(Unesc(cap));
                }
            }
            nodes_.push_back(std::move(n));
        }
    }
    {
        std::ifstream in(dir_ / "events.dat");
        std::string line;
        while (in && std::getline(in, line))
        {
            if (!line.empty())
                events_.push_back(line);
        }
    }
    return NodeError::Ok;
}

NodeError NodeRegistry::Save() const
{
    {
        std::ofstream out(dir_ / "nodes.dat", std::ios::trunc);
        if (!out)
            return NodeError::StorageFailure;
        for (const auto& n : nodes_)
        {
            std::string caps;
            for (std::size_t i = 0; i < n.capabilities.size(); ++i)
            {
                if (i)
                    caps.push_back(',');
                caps += Esc(n.capabilities[i]);
            }
            out << IdHex(n.id) << '|' << Esc(n.name) << '|'
                << static_cast<int>(n.kind) << '|' << static_cast<int>(n.health) << '|'
                << n.last_heartbeat_unix << '|' << Esc(n.endpoint) << '|'
                << Esc(n.workspace_name) << '|' << caps << '\n';
        }
    }
    {
        std::ofstream out(dir_ / "events.dat", std::ios::trunc);
        if (!out)
            return NodeError::StorageFailure;
        for (const auto& e : events_)
            out << e << '\n';
    }
    return NodeError::Ok;
}

Result<NodeId> NodeRegistry::RegisterLocal(const std::string& name)
{
    std::lock_guard lock(mutex_);
    Result<NodeId> r;
    if (!open_ || name.empty())
    {
        r.error = NodeError::InvalidArgument;
        return r;
    }
    for (const auto& n : nodes_)
    {
        if (n.name == name)
        {
            r.error = NodeError::AlreadyExists;
            return r;
        }
    }
    NodeRecord n;
    n.id = NewId();
    n.name = name;
    n.kind = NodeKind::LocalHost;
    n.health = NodeHealth::Healthy;
    n.last_heartbeat_unix = Now();
    n.endpoint = "local";
    nodes_.push_back(n);
    events_.push_back(std::to_string(Now()) + " register_local " + name);
    if (Save() != NodeError::Ok)
    {
        nodes_.pop_back();
        r.error = NodeError::StorageFailure;
        return r;
    }
    r.value = n.id;
    return r;
}

Result<NodeId> NodeRegistry::RegisterPeer(const std::string& name, const std::string& endpoint)
{
    std::lock_guard lock(mutex_);
    Result<NodeId> r;
    if (!open_ || name.empty())
    {
        r.error = NodeError::InvalidArgument;
        return r;
    }
    NodeRecord n;
    n.id = NewId();
    n.name = name;
    n.kind = NodeKind::Peer;
    n.health = NodeHealth::Unknown;
    n.last_heartbeat_unix = 0;
    n.endpoint = endpoint;
    nodes_.push_back(n);
    events_.push_back(std::to_string(Now()) + " register_peer " + name);
    if (Save() != NodeError::Ok)
    {
        nodes_.pop_back();
        r.error = NodeError::StorageFailure;
        return r;
    }
    r.value = n.id;
    return r;
}

Result<NodeRecord> NodeRegistry::Get(const NodeId& id) const
{
    std::lock_guard lock(mutex_);
    Result<NodeRecord> r;
    for (const auto& n : nodes_)
    {
        if (n.id == id)
        {
            r.value = n;
            return r;
        }
    }
    r.error = NodeError::NotFound;
    return r;
}

Result<NodeRecord> NodeRegistry::FindByName(const std::string& name) const
{
    std::lock_guard lock(mutex_);
    Result<NodeRecord> r;
    for (const auto& n : nodes_)
    {
        if (n.name == name)
        {
            r.value = n;
            return r;
        }
    }
    r.error = NodeError::NotFound;
    return r;
}

Result<std::vector<NodeRecord>> NodeRegistry::List() const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<NodeRecord>> r;
    if (!open_)
    {
        r.error = NodeError::NotOpen;
        return r;
    }
    r.value = nodes_;
    return r;
}

Result<void> NodeRegistry::Heartbeat(const NodeId& id, NodeHealth health)
{
    std::lock_guard lock(mutex_);
    for (auto& n : nodes_)
    {
        if (n.id == id)
        {
            n.health = health;
            n.last_heartbeat_unix = Now();
            events_.push_back(std::to_string(Now()) + " heartbeat " + n.name + " " + HealthName(health));
            if (Save() != NodeError::Ok)
                return Result<void>::Err(NodeError::StorageFailure);
            return Result<void>::OkRes();
        }
    }
    return Result<void>::Err(NodeError::NotFound);
}

Result<void> NodeRegistry::MarkStale(std::uint64_t max_age_seconds)
{
    std::lock_guard lock(mutex_);
    const auto now = Now();
    for (auto& n : nodes_)
    {
        if (n.last_heartbeat_unix == 0)
            continue;
        if (now > n.last_heartbeat_unix && (now - n.last_heartbeat_unix) > max_age_seconds)
        {
            if (n.health != NodeHealth::Offline)
            {
                n.health = NodeHealth::Offline;
                events_.push_back(std::to_string(now) + " stale " + n.name);
            }
        }
    }
    if (Save() != NodeError::Ok)
        return Result<void>::Err(NodeError::StorageFailure);
    return Result<void>::OkRes();
}

Result<void> NodeRegistry::BindWorkspace(const NodeId& id, const std::string& workspace_name)
{
    std::lock_guard lock(mutex_);
    if (workspace_name.empty())
        return Result<void>::Err(NodeError::InvalidArgument);
    for (auto& n : nodes_)
    {
        if (n.id == id)
        {
            n.workspace_name = workspace_name;
            if (n.kind == NodeKind::LocalHost || n.kind == NodeKind::Peer)
                n.kind = NodeKind::WorkspaceBound;
            events_.push_back(std::to_string(Now()) + " bind " + n.name + " " + workspace_name);
            if (Save() != NodeError::Ok)
                return Result<void>::Err(NodeError::StorageFailure);
            return Result<void>::OkRes();
        }
    }
    return Result<void>::Err(NodeError::NotFound);
}

Result<void> NodeRegistry::UnbindWorkspace(const NodeId& id)
{
    std::lock_guard lock(mutex_);
    for (auto& n : nodes_)
    {
        if (n.id == id)
        {
            n.workspace_name.clear();
            events_.push_back(std::to_string(Now()) + " unbind " + n.name);
            if (Save() != NodeError::Ok)
                return Result<void>::Err(NodeError::StorageFailure);
            return Result<void>::OkRes();
        }
    }
    return Result<void>::Err(NodeError::NotFound);
}

Result<std::vector<NodeRecord>> NodeRegistry::ListByWorkspace(const std::string& workspace_name) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<NodeRecord>> r;
    for (const auto& n : nodes_)
    {
        if (n.workspace_name == workspace_name)
            r.value.push_back(n);
    }
    return r;
}

Result<void> NodeRegistry::SetCapabilities(const NodeId& id, const std::vector<std::string>& caps)
{
    std::lock_guard lock(mutex_);
    for (auto& n : nodes_)
    {
        if (n.id == id)
        {
            n.capabilities = caps;
            events_.push_back(std::to_string(Now()) + " caps " + n.name + " n=" + std::to_string(caps.size()));
            if (Save() != NodeError::Ok)
                return Result<void>::Err(NodeError::StorageFailure);
            return Result<void>::OkRes();
        }
    }
    return Result<void>::Err(NodeError::NotFound);
}

Result<bool> NodeRegistry::NodeHasCapability(const NodeId& id, const std::string& cap_id) const
{
    std::lock_guard lock(mutex_);
    Result<bool> r;
    for (const auto& n : nodes_)
    {
        if (n.id == id)
        {
            for (const auto& c : n.capabilities)
            {
                if (c == cap_id)
                {
                    r.value = true;
                    return r;
                }
            }
            r.value = false;
            return r;
        }
    }
    r.error = NodeError::NotFound;
    return r;
}

Result<void> NodeRegistry::AppendEvent(const std::string& kind, const std::string& detail)
{
    std::lock_guard lock(mutex_);
    if (!open_)
        return Result<void>::Err(NodeError::NotOpen);
    events_.push_back(std::to_string(Now()) + " " + kind + " " + detail);
    if (Save() != NodeError::Ok)
        return Result<void>::Err(NodeError::StorageFailure);
    return Result<void>::OkRes();
}

Result<std::vector<std::string>> NodeRegistry::RecentEvents(std::size_t limit) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<std::string>> r;
    if (events_.size() <= limit)
        r.value = events_;
    else
        r.value.assign(events_.end() - static_cast<std::ptrdiff_t>(limit), events_.end());
    return r;
}

Result<PlanRouteDecision> NodeRegistry::RouteWorkspaceAction(
    const std::string& workspace_name,
    const std::string& action,
    const std::string& required_cap_id) const
{
    PlanRouteDecision out;
    out.workspace = workspace_name;
    out.action = action;
    out.capability_id = required_cap_id;
    const auto bound = ListByWorkspace(workspace_name);
    if (!bound.Ok()) return Result<PlanRouteDecision>{bound.error, out};
    if (bound.value.empty())
    {
        out.reason = "no_node_bound_to_workspace";
        return Result<PlanRouteDecision>{NodeError::Ok, out};
    }
    const NodeRecord* chosen = nullptr;
    for (const auto& n : bound.value)
    {
        if (n.health == NodeHealth::Healthy) { chosen = &n; break; }
    }
    if (chosen == nullptr) chosen = &bound.value.front();
    out.node_id_hex = IdHex(chosen->id);
    out.node_name = chosen->name;
    out.endpoint = chosen->endpoint.empty() ? "local" : chosen->endpoint;
    out.health = chosen->health;
    if (chosen->health != NodeHealth::Healthy)
    {
        out.reason = std::string("node_unhealthy:") + HealthName(chosen->health);
        return Result<PlanRouteDecision>{NodeError::Ok, out};
    }
    if (!required_cap_id.empty())
    {
        bool has_cap = false;
        for (const auto& c : chosen->capabilities)
            if (c == required_cap_id) { has_cap = true; break; }
        if (!has_cap)
        {
            out.reason = "node_missing_capability:" + required_cap_id;
            return Result<PlanRouteDecision>{NodeError::Ok, out};
        }
    }
    out.allowed = true;
    out.reason = "routed";
    return Result<PlanRouteDecision>{NodeError::Ok, out};
}

Result<NodeExecutionReport> NodeRegistry::ExecuteOnBoundNode(
    const std::string& workspace_name,
    const std::string& action,
    const std::string& required_cap_id,
    const std::string& payload)
{
    NodeExecutionReport rep;
    rep.workspace = workspace_name;
    rep.action = action;
    rep.capability_id = required_cap_id;
    const auto route = RouteWorkspaceAction(workspace_name, action, required_cap_id);
    if (!route.Ok()) return Result<NodeExecutionReport>{route.error, rep};
    rep.node_id_hex = route.value.node_id_hex;
    rep.node_name = route.value.node_name;
    rep.endpoint = route.value.endpoint;
    if (!route.value.allowed)
    {
        rep.reason = route.value.reason;
        (void)AppendEvent("node_exec_denied", workspace_name + ":" + action + " reason=" + rep.reason);
        return Result<NodeExecutionReport>{NodeError::Ok, rep};
    }
    rep.executed = true;
    rep.reason = "executed_on_bound_node";
    rep.output = "node=" + rep.node_name + "(" + rep.node_id_hex + ") endpoint=" + rep.endpoint + " action=" + action + (payload.empty() ? "" : (" payload=" + payload));
    (void)AppendEvent("node_exec", rep.output);
    return Result<NodeExecutionReport>{NodeError::Ok, rep};
}

} // namespace sovereign::node
