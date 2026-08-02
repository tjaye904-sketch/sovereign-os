#pragma once

// Phase II — OS Runtime service manager (Sovereign primary orchestration).
// Manages declared services: dependency order, lifecycle, health, restart policy.

#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace sovereign::os_runtime
{

enum class ServiceState : std::uint8_t
{
    Stopped = 0,
    Starting,
    Running,
    Degraded,
    Failed,
    Stopping
};

enum class RestartPolicy : std::uint8_t
{
    No = 0,
    OnFailure,
    Always
};

struct ServiceSpec
{
    std::string name;
    std::vector<std::string> after; // start after these
    RestartPolicy restart{RestartPolicy::OnFailure};
    std::uint32_t max_restarts{5};
    std::function<bool()> start; // returns true if started ok
    std::function<void()> stop;
    std::function<bool()> health; // true = healthy
};

struct ServiceStatus
{
    std::string name;
    ServiceState state{ServiceState::Stopped};
    std::uint32_t restarts{0};
    std::string last_error;
};

class ServiceOrchestrator
{
public:
    void Register(ServiceSpec spec);
    bool StartAll();
    void StopAll() noexcept;
    bool StartOne(const std::string& name);
    void StopOne(const std::string& name) noexcept;
    // Watchdog tick: health-check running services; restart per policy
    void WatchdogTick();
    std::vector<ServiceStatus> List() const;
    ServiceStatus Get(const std::string& name) const;

private:
    std::vector<std::string> TopoOrder() const;
    mutable std::mutex mutex_;
    std::map<std::string, ServiceSpec> specs_;
    std::map<std::string, ServiceStatus> status_;
};

} // namespace sovereign::os_runtime
