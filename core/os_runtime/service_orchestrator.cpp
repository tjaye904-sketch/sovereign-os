#include "service_orchestrator.hpp"

#include <algorithm>
#include <queue>
#include <set>

namespace sovereign::os_runtime
{

void ServiceOrchestrator::Register(ServiceSpec spec)
{
    std::lock_guard lock(mutex_);
    ServiceStatus st;
    st.name = spec.name;
    status_[spec.name] = st;
    specs_[spec.name] = std::move(spec);
}

std::vector<std::string> ServiceOrchestrator::TopoOrder() const
{
    std::map<std::string, int> indeg;
    std::map<std::string, std::vector<std::string>> adj;
    for (const auto& [n, sp] : specs_)
    {
        indeg[n] = 0;
    }
    for (const auto& [n, sp] : specs_)
    {
        for (const auto& dep : sp.after)
        {
            if (specs_.count(dep))
            {
                adj[dep].push_back(n);
                indeg[n]++;
            }
        }
    }
    std::queue<std::string> q;
    for (const auto& [n, d] : indeg)
        if (d == 0)
            q.push(n);
    std::vector<std::string> order;
    while (!q.empty())
    {
        auto u = q.front();
        q.pop();
        order.push_back(u);
        for (const auto& v : adj[u])
        {
            if (--indeg[v] == 0)
                q.push(v);
        }
    }
    // append any missing (cycles → start order residual)
    for (const auto& [n, _] : specs_)
        if (std::find(order.begin(), order.end(), n) == order.end())
            order.push_back(n);
    return order;
}

bool ServiceOrchestrator::StartAll()
{
    std::lock_guard lock(mutex_);
    bool ok = true;
    for (const auto& name : TopoOrder())
    {
        auto& sp = specs_[name];
        auto& st = status_[name];
        st.state = ServiceState::Starting;
        if (!sp.start)
        {
            st.state = ServiceState::Failed;
            st.last_error = "no start handler";
            ok = false;
            continue;
        }
        try
        {
            if (sp.start())
            {
                st.state = ServiceState::Running;
                st.last_error.clear();
            }
            else
            {
                st.state = ServiceState::Failed;
                st.last_error = "start returned false";
                ok = false;
            }
        }
        catch (const std::exception& e)
        {
            st.state = ServiceState::Failed;
            st.last_error = e.what();
            ok = false;
        }
    }
    return ok;
}

void ServiceOrchestrator::StopAll() noexcept
{
    std::lock_guard lock(mutex_);
    auto order = TopoOrder();
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        auto& sp = specs_[*it];
        auto& st = status_[*it];
        st.state = ServiceState::Stopping;
        if (sp.stop)
        {
            try
            {
                sp.stop();
            }
            catch (...)
            {
            }
        }
        st.state = ServiceState::Stopped;
    }
}

bool ServiceOrchestrator::StartOne(const std::string& name)
{
    std::lock_guard lock(mutex_);
    auto it = specs_.find(name);
    if (it == specs_.end())
        return false;
    auto& st = status_[name];
    st.state = ServiceState::Starting;
    if (!it->second.start || !it->second.start())
    {
        st.state = ServiceState::Failed;
        st.last_error = "start failed";
        return false;
    }
    st.state = ServiceState::Running;
    return true;
}

void ServiceOrchestrator::StopOne(const std::string& name) noexcept
{
    std::lock_guard lock(mutex_);
    auto it = specs_.find(name);
    if (it == specs_.end())
        return;
    if (it->second.stop)
    {
        try
        {
            it->second.stop();
        }
        catch (...)
        {
        }
    }
    status_[name].state = ServiceState::Stopped;
}

void ServiceOrchestrator::WatchdogTick()
{
    std::lock_guard lock(mutex_);
    for (auto& [name, sp] : specs_)
    {
        auto& st = status_[name];
        if (st.state != ServiceState::Running && st.state != ServiceState::Degraded)
            continue;
        bool healthy = true;
        if (sp.health)
        {
            try
            {
                healthy = sp.health();
            }
            catch (...)
            {
                healthy = false;
            }
        }
        if (healthy)
        {
            st.state = ServiceState::Running;
            continue;
        }
        st.state = ServiceState::Degraded;
        if (sp.restart == RestartPolicy::No)
            continue;
        if (st.restarts >= sp.max_restarts)
        {
            st.state = ServiceState::Failed;
            st.last_error = "max restarts exceeded";
            continue;
        }
        if (sp.stop)
        {
            try
            {
                sp.stop();
            }
            catch (...)
            {
            }
        }
        ++st.restarts;
        st.state = ServiceState::Starting;
        if (sp.start && sp.start())
            st.state = ServiceState::Running;
        else
        {
            st.state = ServiceState::Failed;
            st.last_error = "restart failed";
        }
    }
}

std::vector<ServiceStatus> ServiceOrchestrator::List() const
{
    std::lock_guard lock(mutex_);
    std::vector<ServiceStatus> out;
    for (const auto& [_, st] : status_)
        out.push_back(st);
    return out;
}

ServiceStatus ServiceOrchestrator::Get(const std::string& name) const
{
    std::lock_guard lock(mutex_);
    auto it = status_.find(name);
    if (it == status_.end())
        return ServiceStatus{.name = name, .state = ServiceState::Failed, .last_error = "unknown"};
    return it->second;
}

std::vector<SessionServiceEndpoint> ServiceOrchestrator::DefaultSessionTopology()
{
    return {
        SessionServiceEndpoint{.name = "daemon", .unit_or_bin = "sovereign-daemon", .health_url = "", .port = 0, .ready = false},
        SessionServiceEndpoint{.name = "computer_use", .unit_or_bin = "sovereign-cu-server", .health_url = "http://localhost", .port = 8765, .ready = false},
        SessionServiceEndpoint{.name = "sys", .unit_or_bin = "sovereign-sys-server", .health_url = "http://localhost", .port = 8767, .ready = false},
        SessionServiceEndpoint{.name = "hub", .unit_or_bin = "sovereign-hub-server", .health_url = "http://localhost", .port = 8766, .ready = false},
        SessionServiceEndpoint{.name = "caps", .unit_or_bin = "caps_sidecar.py", .health_url = "http://localhost", .port = 8769, .ready = false},
    };
}

SessionReadinessReport ServiceOrchestrator::EvaluateSessionReadiness(
    const std::function<bool(const SessionServiceEndpoint&)>& probe)
{
    SessionReadinessReport report;
    report.services = DefaultSessionTopology();
    for (auto& svc : report.services)
    {
        svc.ready = probe ? probe(svc) : false;
        if (svc.ready)
        {
            ++report.ready_count;
        }
    }
    report.all_ready = !report.services.empty() && report.ready_count == report.services.size();
    return report;
}
} // namespace sovereign::os_runtime
