#include "sovereign_host.hpp"
#include "test_require.hpp"
#include <filesystem>
#include <iostream>
using namespace sovereign::product;
int main()
{
    const auto root = std::filesystem::temp_directory_path() / "sov_host_test";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    HostConfig cfg;
    cfg.data_root = root;
    SovereignHost host(cfg);
    REQUIRE(host.Open());
    REQUIRE(host.Identity().BootstrapOwner("owner", "secret-pass").Ok());
    REQUIRE(host.Identity().Authenticate("owner", "secret-pass").Ok());
    REQUIRE(host.Workspace().Create("main", "development").Ok());
    host.Close();
    SovereignHost host2(cfg);
    REQUIRE(host2.Open());
    REQUIRE(host2.Workspace().List().value.size() == 1);
    host2.Close();
    std::filesystem::remove_all(root, ec);
    std::cout << "sovereign_host_smoke_test: OK\n";
    return 0;
}
