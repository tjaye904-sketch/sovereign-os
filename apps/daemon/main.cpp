// sovereign-daemon — Phase II OS runtime: ServiceOrchestrator + agent loop.

#include "service_orchestrator.hpp"
#include "sovereign_host.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

namespace
{
std::atomic<bool> g_run{true};

void OnSignal(int)
{
    g_run = false;
}

std::filesystem::path DefaultDataRoot()
{
    if (const char* root = std::getenv("SOVEREIGN_DATA_ROOT");
        root && root[0])
        return root;
    if (const char* home = std::getenv("HOME"); home && home[0])
        return std::filesystem::path(home) / ".local" / "share" / "sovereign";
    return std::filesystem::temp_directory_path() / "sovereign";
}

const char* StateName(sovereign::os_runtime::ServiceState s)
{
    using sovereign::os_runtime::ServiceState;
    switch (s)
    {
    case ServiceState::Stopped:
        return "stopped";
    case ServiceState::Starting:
        return "starting";
    case ServiceState::Running:
        return "running";
    case ServiceState::Degraded:
        return "degraded";
    case ServiceState::Failed:
        return "failed";
    case ServiceState::Stopping:
        return "stopping";
    }
    return "unknown";
}
} // namespace

int main(int argc, char** argv)
{
    if (argc >= 2 &&
        (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h"))
    {
        std::cout
            << "sovereign-daemon [tick_seconds]\n"
            << "  Phase II orchestrator + agent loop. SIGINT/SIGTERM to stop.\n"
            << "  Env: SOVEREIGN_DATA_ROOT, OLLAMA_HOST, SOVEREIGN_OLLAMA_MODEL\n";
        return 0;
    }
    std::uint64_t tick_seconds = 60;
    if (argc >= 2)
        tick_seconds = std::stoull(argv[1]);
    if (tick_seconds < 1)
        tick_seconds = 1;

    std::signal(SIGINT, OnSignal);
    std::signal(SIGTERM, OnSignal);

    sovereign::product::HostConfig cfg;
    cfg.data_root = DefaultDataRoot();
    if (const char* o = std::getenv("OLLAMA_HOST"); o && o[0])
        cfg.ollama_base = o;
    if (const char* m = std::getenv("SOVEREIGN_OLLAMA_MODEL"); m && m[0])
        cfg.default_model = m;

    sovereign::product::SovereignHost host(cfg);
    sovereign::os_runtime::ServiceOrchestrator orch;

    {
        sovereign::os_runtime::ServiceSpec s;
        s.name = "host";
        s.restart = sovereign::os_runtime::RestartPolicy::No;
        s.start = [&] {
            if (!host.Open())
            {
                std::cerr << "daemon: host open failed at " << cfg.data_root
                          << '\n';
                return false;
            }
            return true;
        };
        s.stop = [&] { host.Close(); };
        s.health = [&] { return true; };
        orch.Register(std::move(s));
    }
    {
        sovereign::os_runtime::ServiceSpec s;
        s.name = "agent-tick";
        s.after = {"host"};
        s.restart = sovereign::os_runtime::RestartPolicy::OnFailure;
        s.max_restarts = 10;
        s.start = [] { return true; };
        s.stop = [] {};
        s.health = [] { return true; };
        orch.Register(std::move(s));
    }
    {
        sovereign::os_runtime::ServiceSpec s;
        s.name = "audit-heartbeat";
        s.after = {"host"};
        s.restart = sovereign::os_runtime::RestartPolicy::OnFailure;
        s.start = [&] {
            sovereign::audit::AuditEntry e;
            e.category = "daemon";
            e.action = "start";
            e.detail = "orchestrator online";
            static_cast<void>(host.Audit().Append(e));
            return true;
        };
        s.stop = [&] {
            sovereign::audit::AuditEntry e;
            e.category = "daemon";
            e.action = "stop";
            e.detail = "orchestrator offline";
            static_cast<void>(host.Audit().Append(e));
        };
        s.health = [] { return true; };
        orch.Register(std::move(s));
    }

    if (!orch.StartAll())
    {
        std::cerr << "daemon: orchestrator StartAll failed\n";
        for (const auto& st : orch.List())
            std::cerr << "  " << st.name << " " << StateName(st.state) << " "
                      << st.last_error << '\n';
        return 1;
    }

    std::cout << "sovereign-daemon started data_root=" << cfg.data_root
              << " tick_s=" << tick_seconds << " orchestrator=on" << std::endl;
    for (const auto& st : orch.List())
        std::cout << "  service " << st.name << "=" << StateName(st.state)
                  << std::endl;

    while (g_run)
    {
        using namespace std::chrono;
        const auto now = static_cast<std::uint64_t>(
            duration_cast<seconds>(system_clock::now().time_since_epoch())
                .count());
        orch.WatchdogTick();
        auto due = host.Agent().Due(now);
        if (due.Ok())
        {
            for (const auto& j : due.value)
            {
                if (j.kind == sovereign::agent::JobKind::BriefSnapshot ||
                    j.kind == sovereign::agent::JobKind::MemoryDigest)
                {
                    auto brief = host.Briefing().Build();
                    sovereign::memory::MemoryRecord rec;
                    rec.tags = {"agent", "brief", j.name, "daemon"};
                    rec.body = brief.rendered_text;
                    auto app = host.Memory().Append(rec);
                    if (app.Ok())
                    {
                        static_cast<void>(host.Semantic().Upsert(
                            brief.rendered_text, {"agent", "brief", j.name}));
                        std::cout << "daemon: ran " << j.name << " at " << now
                                  << std::endl;
                    }
                }
                static_cast<void>(host.Agent().MarkRun(j.id, now));
            }
        }
        for (std::uint64_t i = 0; i < tick_seconds && g_run; ++i)
            std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    orch.StopAll();
    std::cout << "sovereign-daemon stopped" << std::endl;
    return 0;
}
