#include "sovereign_host.hpp"
#include "test_require.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    const auto root = std::filesystem::temp_directory_path() /
        "sov_purpose_runtime_test";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);

    sovereign::product::HostConfig cfg;
    cfg.data_root = root;
    sovereign::product::SovereignHost host(cfg);

    REQUIRE(host.Open());

    const auto& packs = host.Purpose().BuiltinPacks();
    REQUIRE(!packs.empty());

    const auto research = host.Purpose().Find("research.web_collect");
    REQUIRE(research.has_value());
    REQUIRE(research->allowed_hosts.size() >= 1);
    REQUIRE(research->allowed_tools.size() >= 1);

    auto allow = host.Purpose().CanRun(
        *research,
        "browser_open",
        "https://docs.python.org/3/",
        "open docs for reference");
    REQUIRE(allow.allowed);
    REQUIRE(allow.reason.empty());

    auto canonical = host.Purpose().CanRun(
        *research,
        "browser.open",
        "https://docs.python.org/3/",
        "open docs via canonical host automation name");
    REQUIRE(canonical.allowed);
    REQUIRE(canonical.reason.empty());

    auto app_launch = host.Purpose().CanRun(
        *research,
        "app.launch",
        "https://docs.python.org/3/",
        "launch docs via purpose-approved app path");
    REQUIRE(app_launch.allowed);
    REQUIRE(app_launch.reason.empty());

    auto deny = host.Purpose().CanRun(
        *research,
        "browser_open",
        "https://example.com",
        "unexpected host");
    REQUIRE(!deny.allowed);
    REQUIRE(!deny.reason.empty());

    auto critical = host.Purpose().CanRun(
        *research,
        "download_file",
        "https://docs.python.org/3/",
        "download arbitrary artifact");
    REQUIRE(!critical.allowed);
    REQUIRE(!critical.reason.empty());

    host.Purpose().StartPurpose(research->id, "operator verification");
    host.Purpose().LogAction(research->id, "browser_open", "https://docs.python.org/3/", "open docs page");
    REQUIRE(host.Purpose().CurrentPurposeId() == research->id);
    auto history = host.Purpose().ActionLog();
    REQUIRE(!history.empty());
    REQUIRE(history.back().purpose_id == research->id);

    const auto session_path = root / "purpose-session-state.json";
    REQUIRE(host.Purpose().SaveSession(session_path));

    sovereign::product::PurposeRuntime reloaded;
    REQUIRE(reloaded.LoadSession(session_path));
    REQUIRE(reloaded.CurrentPurposeId() == research->id);
    REQUIRE(!reloaded.ActionLog().empty());
    REQUIRE(reloaded.ActionLog().back().purpose_id == research->id);

    host.Close();
    std::filesystem::remove_all(root, ec);
    std::cout << "sovereign_purpose_runtime_smoke_test: OK\n";
    return 0;
}
