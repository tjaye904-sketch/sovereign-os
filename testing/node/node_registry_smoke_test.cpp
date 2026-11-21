#include "node_registry.hpp"
#include <filesystem>
#include <iostream>
#define REQUIRE(c) do { if (!(c)) { std::cerr << "REQUIRE failed: " #c "\n"; return 1; } } while (0)

int main()
{
    namespace fs = std::filesystem;
    auto root = fs::temp_directory_path() / "sov-node-reg";
    fs::remove_all(root);
    sovereign::node::NodeRegistry reg;
    REQUIRE(reg.Open(root) == sovereign::node::NodeError::Ok);

    // L2.1
    auto local = reg.RegisterLocal("laptop");
    REQUIRE(local.Ok());
    auto peer = reg.RegisterPeer("edge-1", "10.0.0.2:9000");
    REQUIRE(peer.Ok());
    REQUIRE(reg.List().value.size() == 2);

    // L2.2
    REQUIRE(reg.Heartbeat(local.value, sovereign::node::NodeHealth::Healthy).Ok());
    REQUIRE(reg.Get(local.value).value.health == sovereign::node::NodeHealth::Healthy);

    // L2.3
    REQUIRE(reg.BindWorkspace(local.value, "development").Ok());
    REQUIRE(reg.ListByWorkspace("development").value.size() == 1);

    // L2.4
    REQUIRE(reg.SetCapabilities(local.value, {"host.cli", "model.chat"}).Ok());
    REQUIRE(reg.NodeHasCapability(local.value, "host.cli").value == true);
    REQUIRE(reg.NodeHasCapability(local.value, "voice.speak").value == false);

    // L2.5
    REQUIRE(reg.AppendEvent("test", "ok").Ok());
    REQUIRE(reg.RecentEvents(5).value.size() >= 1);

    reg.Close();
    sovereign::node::NodeRegistry reg2;
    REQUIRE(reg2.Open(root) == sovereign::node::NodeError::Ok);
    REQUIRE(reg2.FindByName("laptop").Ok());

    fs::remove_all(root);
    std::cout << "node_registry_smoke_test: OK (L2.1-L2.5)\n";
    return 0;
}
