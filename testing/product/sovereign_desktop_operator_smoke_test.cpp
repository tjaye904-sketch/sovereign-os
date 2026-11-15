#include "sovereign_host.hpp"
#include "test_require.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>

int main()
{
    // Priority #3: CTest must not open headed browsers that block until closed.
    setenv("SOVEREIGN_SYS_HEADLESS", "1", 1);
    setenv("SOVEREIGN_CU_HEADLESS", "1", 1);
    setenv("SOVEREIGN_DESKTOP_HEADLESS", "1", 1);

    const auto root = std::filesystem::temp_directory_path() / "sov_desktop_operator_test";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);

    sovereign::product::HostConfig cfg;
    cfg.data_root = root;
    sovereign::product::SovereignHost host(cfg);

    REQUIRE(host.Open());
    REQUIRE(host.Identity().BootstrapOwner("owner", "secret-pass").Ok());
    const auto auth = host.Identity().Authenticate("owner", "secret-pass");
    REQUIRE(auth.Ok());

    std::string browser_decision_hex;
    REQUIRE(host.Desktop().RequestAction(
        auth.value.principal_id,
        "browser.open",
        "https://docs.python.org/3/",
        "open allowlisted docs for a purpose-driven research task",
        browser_decision_hex));

    REQUIRE(host.Desktop().Approve(
        auth.value.principal_id,
        browser_decision_hex,
        "approved for host automation verification"));

    std::string browser_report;
    REQUIRE(host.Desktop().ExecuteApprovedAction(browser_decision_hex, browser_report));
    REQUIRE(!browser_report.empty());
    // Under headless env, execution should record safe/headless path rather than hang on GUI.
    REQUIRE(
        browser_report.find("headless") != std::string::npos ||
        browser_report.find("executed") != std::string::npos ||
        browser_report.find("desktop") != std::string::npos);

    std::string desktop_decision_hex;
    REQUIRE(host.Desktop().RequestAction(
        auth.value.principal_id,
        "desktop.snapshot",
        "capture current screen",
        "operator desktop verification",
        desktop_decision_hex));

    REQUIRE(host.Desktop().Approve(
        auth.value.principal_id,
        desktop_decision_hex,
        "approved for operator desktop check"));

    std::string report;
    REQUIRE(host.Desktop().ExecuteApprovedAction(desktop_decision_hex, report));
    REQUIRE(!report.empty());
    REQUIRE(report.find("headless") != std::string::npos);
    REQUIRE(std::filesystem::exists(root / "desktop"));
    REQUIRE(std::filesystem::is_directory(root / "desktop"));

    auto evidence_files = 0;
    for (const auto& entry : std::filesystem::directory_iterator(root / "desktop"))
    {
        if (entry.is_regular_file())
        {
            ++evidence_files;
        }
    }
    REQUIRE(evidence_files > 0);

    auto desktop_audit = host.Audit().ListByCategory("desktop", 20);
    REQUIRE(desktop_audit.Ok());
    REQUIRE(!desktop_audit.value.empty());

    host.Close();
    std::filesystem::remove_all(root, ec);
    std::cout << "sovereign_desktop_operator_smoke_test: OK\n";
    return 0;
}
