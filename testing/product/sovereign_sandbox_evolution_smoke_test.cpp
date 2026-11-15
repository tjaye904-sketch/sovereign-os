#include "sovereign_host.hpp"
#include "test_require.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    const auto root = std::filesystem::temp_directory_path() / "sov_sandbox_host_test";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);

    sovereign::product::HostConfig cfg;
    cfg.data_root = root;
    sovereign::product::SovereignHost host(cfg);
    REQUIRE(host.Open());

    auto proposal = host.SandboxEvolution().CreateProposal(2);
    REQUIRE(!proposal.id.empty());
    REQUIRE(proposal.status == sovereign::cognitive::SandboxProposalStatus::Isolated);
    REQUIRE(std::filesystem::exists(proposal.sandbox_root));
    REQUIRE(host.SandboxEvolution().ValidateProposal(proposal));
    REQUIRE(host.SandboxEvolution().ApproveProposal(proposal.id, "human"));

    host.Close();
    std::filesystem::remove_all(root, ec);
    std::cout << "sovereign_sandbox_evolution_smoke_test: OK\n";
    return 0;
}
