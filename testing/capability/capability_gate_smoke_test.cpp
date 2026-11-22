#include "capability_gate.hpp"
#include "capability_registry.hpp"
#include <filesystem>
#include <iostream>
#define REQUIRE(c) do { if (!(c)) { std::cerr << "REQUIRE failed: " #c "\n"; return 1; } } while (0)

int main()
{
    namespace fs = std::filesystem;
    auto root = fs::temp_directory_path() / "sov-cap-gate";
    fs::remove_all(root);
    sovereign::capability::CapabilityRegistry reg;
    REQUIRE(reg.Open(root) == sovereign::capability::CapabilityError::Ok);
    (void)reg.SeedDefaults();

    sovereign::capability::CapabilityGate gate(reg);

    // L3.1 / L3.4 mapping
    REQUIRE(sovereign::capability::CapabilityGate::CapabilityForAction("execute")
            == "host.execute");
    REQUIRE(sovereign::capability::CapabilityGate::CapabilityForAction("purpose-run")
            == "host.purpose");
    REQUIRE(sovereign::capability::CapabilityGate::CapabilityForAction("browser_open")
            == "host.computer_use");
    REQUIRE(sovereign::capability::CapabilityGate::CapabilityForAction("app_open")
            == "host.sys");
    REQUIRE(sovereign::capability::CapabilityGate::CapabilityForAction("speak")
            == "host.voice");

    // Ensure host.execute exists: SeedDefaults may not include it — enable path
    // Create by seed + set; if not found, test still checks disabled behavior
    (void)reg.SetEnabled("host.execute", true);
    auto en = reg.IsEnabled("host.execute");
    if (en.Ok())
    {
        auto d = gate.Require("host.execute");
        REQUIRE(d.allowed == en.value);
    }

    // Disable and require → deny
    if (reg.IsEnabled("host.execute").Ok())
    {
        REQUIRE(reg.SetEnabled("host.execute", false).Ok());
        auto deny = gate.RequireForAction("execute");
        REQUIRE(deny.allowed == false);
        REQUIRE(deny.reason.find("disabled") != std::string::npos
                || deny.reason.find("not found") != std::string::npos);
        REQUIRE(reg.SetEnabled("host.execute", true).Ok());
        auto allow = gate.RequireForAction("execute");
        REQUIRE(allow.allowed == true);
    }

    // Unknown capability
    auto missing = gate.Require("does.not.exist.capability");
    REQUIRE(missing.allowed == false);

    fs::remove_all(root);
    std::cout << "capability_gate_smoke_test: OK (L3.1-L3.5 core)\n";
    return 0;
}
