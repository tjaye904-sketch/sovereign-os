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

    // --- L8: Governed purpose runner (CapabilityGate + GovernanceService + AuditLog) ---
    const auto audit_before = host.Audit().ListByCategory("governance", 200);
    REQUIRE(audit_before.Ok());
    const std::size_t initial_gov_audits = audit_before.value.size();

    // 1. Baseline Run() allowed when host.purpose + action capabilities are enabled
    auto run_ok = host.Purpose().Run("research.web_collect", "https://docs.python.org/3/", "l8 allow check");
    REQUIRE(run_ok.allowed);
    REQUIRE(run_ok.executed);

    // 2. CapabilityGate blocks Run() when host.purpose is disabled
    REQUIRE(host.Capabilities().SetEnabled("host.purpose", false).Ok());
    auto run_cap_denied = host.Purpose().Run("research.web_collect", "https://docs.python.org/3/", "l8 cap deny");
    REQUIRE(!run_cap_denied.allowed);
    REQUIRE(!run_cap_denied.executed);
    REQUIRE(!run_cap_denied.reason.empty());
    REQUIRE(host.Capabilities().SetEnabled("host.purpose", true).Ok());

    // 3. High-risk pack requires GovernanceService approval
    sovereign::product::PurposePack high_pack;
    high_pack.id = "ops.high_risk_probe";
    high_pack.purpose_id = "ops.high_risk_probe";
    high_pack.description = "High risk purpose pack for L8 governance gate test";
    high_pack.risk = sovereign::product::PurposeRisk::High;
    high_pack.allowed_tools = {"browser_open"};
    high_pack.allowed_hosts = {"docs.python.org"};
    high_pack.steps.push_back(sovereign::product::PurposeStep{
        .action = "browser_open",
        .target = "https://docs.python.org/3/",
        .detail = "open docs under high risk"});
    REQUIRE(host.Purpose().RegisterPack(high_pack));

    auto high_denied = host.Purpose().Run("ops.high_risk_probe", "https://docs.python.org/3/", "high risk unapproved");
    REQUIRE(!high_denied.allowed);
    REQUIRE(!high_denied.executed);
    REQUIRE(high_denied.reason == "governance_approval_required");

    // Configure governance auto-approval rule for ops.high_risk_probe and retry
    sovereign::governance::GovernanceRule allow_rule;
    allow_rule.action = "purpose.run.ops.high_risk_probe";
    allow_rule.allow_auto_approval = true;
    allow_rule.minimum_auto_approve_role = sovereign::governance::GovernanceRole::Operator;
    allow_rule.minimum_approval_authority_role = sovereign::governance::GovernanceRole::Owner;
    host.Governance().SetRule(allow_rule);

    auto high_allowed = host.Purpose().Run("ops.high_risk_probe", "https://docs.python.org/3/", "high risk approved");
    REQUIRE(high_allowed.allowed);
    REQUIRE(high_allowed.executed);

    // 4. AuditLog captured allow and deny events
    const auto audit_after = host.Audit().ListByCategory("governance", 200);
    REQUIRE(audit_after.Ok());
    REQUIRE(audit_after.value.size() > initial_gov_audits);

    host.Close();
    std::filesystem::remove_all(root, ec);
    std::cout << "sovereign_purpose_runtime_smoke_test: OK\n";
    return 0;
}
