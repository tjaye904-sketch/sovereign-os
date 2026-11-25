#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace sovereign::os_runtime
{

enum class ServiceState : std::uint8_t { Stopped = 0, Starting, Running, Degraded, Failed, Stopping };
enum class RestartPolicy : std::uint8_t { No = 0, OnFailure, Always };

struct ServiceSpec
{
    std::string name;
    std::vector<std::string> after;
    RestartPolicy restart{RestartPolicy::OnFailure};
    std::uint32_t max_restarts{5};
    std::function<bool()> start;
    std::function<void()> stop;
    std::function<bool()> health;
};

struct ServiceStatus
{
    std::string name;
    ServiceState state{ServiceState::Stopped};
    std::uint32_t restarts{0};
    std::string last_error;
};

struct SessionServiceEndpoint
{
    std::string name;
    std::string unit_or_bin;
    std::string health_url;
    std::uint16_t port{0};
    bool ready{false};
};

struct SessionReadinessReport
{
    bool all_ready{false};
    std::size_t ready_count{0};
    std::vector<SessionServiceEndpoint> services;
};

class ServiceOrchestrator
{
public:
    void Register(ServiceSpec spec);
    bool StartAll();
    void StopAll() noexcept;
    bool StartOne(const std::string& name);
    void StopOne(const std::string& name) noexcept;
    void WatchdogTick();
    std::vector<ServiceStatus> List() const;
    ServiceStatus Get(const std::string& name) const;
    static std::vector<SessionServiceEndpoint> DefaultSessionTopology();
    static SessionReadinessReport EvaluateSessionReadiness(
        const std::function<bool(const SessionServiceEndpoint&)>& probe);

private:
    std::vector<std::string> TopoOrder() const;
    mutable std::mutex mutex_;
    std::map<std::string, ServiceSpec> specs_;
    std::map<std::string, ServiceStatus> status_;
};

} // namespace sovereign::os_runtime
