#include "sovereign_host.hpp"
#include "test_require.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>

int main()
{
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

    std::string decision_hex;
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

    setenv("SOVEREIGN_SYS_HEADLESS", "1", 1);
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

    // --- L9: DesktopOperatorStatus & ApiGatewayService::GetDesktopStatus() ---
    const auto status_headless = host.ApiGateway().GetDesktopStatus(10);
    REQUIRE(status_headless.mode == "headless");
    REQUIRE(status_headless.computer_use_enabled);
    REQUIRE(status_headless.evidence_files > 0);
    REQUIRE(!status_headless.recent_audit_events.empty());

    REQUIRE(host.Capabilities().SetEnabled("host.computer_use", false).Ok());
    const auto status_disabled = host.Desktop().Status(10);
    REQUIRE(!status_disabled.computer_use_enabled);
    REQUIRE(host.Capabilities().SetEnabled("host.computer_use", true).Ok());

    unsetenv("SOVEREIGN_SYS_HEADLESS");
    host.Close();
    std::filesystem::remove_all(root, ec);
    std::cout << "sovereign_desktop_operator_smoke_test: OK\n";
    return 0;
}
