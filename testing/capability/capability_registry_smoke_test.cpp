#include "capability_registry.hpp"

#include <filesystem>
#include <iostream>

#define REQUIRE(cond) \
    do { if (!(cond)) { std::cerr << "REQUIRE failed: " #cond "\n"; return 1; } } while (0)

int main()
{
    namespace fs = std::filesystem;
    auto root = fs::temp_directory_path() / "sov-cap-reg";
    fs::remove_all(root);

    sovereign::capability::CapabilityRegistry reg;
    REQUIRE(reg.Open(root) == sovereign::capability::CapabilityError::Ok);
    auto n = reg.SeedDefaults();
    REQUIRE(n.Ok());
    REQUIRE(n.value >= 10);

    auto list = reg.List();
    REQUIRE(list.Ok());
    REQUIRE(list.value.size() >= n.value);

    REQUIRE(reg.SetEnabled("host.computer_use", false).Ok());
    auto en = reg.IsEnabled("host.computer_use");
    REQUIRE(en.Ok() && !en.value);
    REQUIRE(reg.SetEnabled("host.computer_use", true).Ok());

    sovereign::capability::Capability custom;
    custom.id = "ops.custom_probe";
    custom.description = "test";
    custom.enabled = true;
    custom.source = "operator";
    REQUIRE(reg.Register(custom).Ok());
    REQUIRE(reg.Get("ops.custom_probe").Ok());

    // fail-closed unknown
    auto unk = reg.IsEnabled("totally.unknown");
    REQUIRE(unk.Ok() && !unk.value);

    reg.Close();
    // reopen persistence
    sovereign::capability::CapabilityRegistry reg2;
    REQUIRE(reg2.Open(root) == sovereign::capability::CapabilityError::Ok);
    REQUIRE(reg2.Get("ops.custom_probe").Ok());

    fs::remove_all(root);
    std::cout << "capability_registry_smoke_test: OK\n";
    return 0;
}
