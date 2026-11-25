#include "service_orchestrator.hpp"
#include "test_require.hpp"
#include <iostream>

int main()
{
    using namespace sovereign::os_runtime;
    ServiceOrchestrator orch;
    int started = 0;
    int stopped = 0;

    ServiceSpec a;
    a.name = "host";
    a.start = [&] {
        ++started;
        return true;
    };
    a.stop = [&] { ++stopped; };
    a.health = [] { return true; };
    a.restart = RestartPolicy::OnFailure;

    ServiceSpec b;
    b.name = "agent-tick";
    b.after = {"host"};
    b.start = [&] {
        ++started;
        return true;
    };
    b.stop = [&] { ++stopped; };
    b.health = [] { return true; };

    orch.Register(std::move(a));
    orch.Register(std::move(b));
    REQUIRE(orch.StartAll());
    REQUIRE(started == 2);
    auto list = orch.List();
    REQUIRE(list.size() == 2);
    orch.WatchdogTick();
    orch.StopAll();
    REQUIRE(stopped == 2);
    const auto topo = ServiceOrchestrator::DefaultSessionTopology();
    REQUIRE(topo.size() == 5);
    const auto partial = ServiceOrchestrator::EvaluateSessionReadiness([](const SessionServiceEndpoint& ep) { return ep.name == "daemon" || ep.name == "caps"; });
    REQUIRE(!partial.all_ready);
    REQUIRE(partial.ready_count == 2);
    const auto full = ServiceOrchestrator::EvaluateSessionReadiness([](const SessionServiceEndpoint&) { return true; });
    REQUIRE(full.all_ready);
    REQUIRE(full.ready_count == 5);
    std::cout << "service_orchestrator_smoke_test: OK\n";
    return 0;
}
