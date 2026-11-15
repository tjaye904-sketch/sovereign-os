#include "sovereign_host.hpp"
#include "test_require.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    const auto root = std::filesystem::temp_directory_path() / "sov_external_gateway_test";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);

    sovereign::product::HostConfig cfg;
    cfg.data_root = root;
    sovereign::product::SovereignHost host(cfg);
    REQUIRE(host.Open());
    REQUIRE(host.Identity().BootstrapOwner("owner", "secret-pass").Ok());
    const auto auth = host.Identity().Authenticate("owner", "secret-pass");
    REQUIRE(auth.Ok());

    std::string decision_hex;
    REQUIRE(host.External().Request(
        auth.value.principal_id,
        "external:status",
        "Status poll from external caller",
        "evidence:gateway",
        decision_hex));
    REQUIRE(!decision_hex.empty());
    REQUIRE(host.External().Approve(
        auth.value.principal_id,
        decision_hex,
        "owner authorized external status poll"));

    host.Close();
    std::filesystem::remove_all(root, ec);
    std::cout << "sovereign_external_gateway_smoke_test: OK\n";
    return 0;
}
