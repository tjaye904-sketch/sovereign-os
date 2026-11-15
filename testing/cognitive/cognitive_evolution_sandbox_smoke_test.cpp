#include "evolution_engine.hpp"
#include "test_require.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    using namespace sovereign::cognitive;

    const auto repo_root = std::filesystem::path("/home/tjaye904/Documents/GitHub/sovereign-os");
    const auto sandbox_root = std::filesystem::temp_directory_path() / "sovereign_evolution_sandbox_test";
    std::error_code ec;
    std::filesystem::remove_all(sandbox_root, ec);

    SandboxedEvolutionEngine sandbox(repo_root, sandbox_root);
    auto proposal = sandbox.CreateProposal(2);
    REQUIRE(!proposal.id.empty());
    REQUIRE(proposal.status == SandboxProposalStatus::Isolated);
    REQUIRE(std::filesystem::exists(proposal.sandbox_root));
    REQUIRE(std::filesystem::exists(proposal.sandbox_root / "CMakeLists.txt"));

    const bool validated = sandbox.ValidateProposal(proposal);
    REQUIRE(validated);
    REQUIRE(!proposal.validation_log.empty());
    REQUIRE(!proposal.rollback_command.empty());
    REQUIRE(proposal.rollback_summary.find("sandbox") != std::string::npos ||
            proposal.rollback_summary.find("source") != std::string::npos);

    std::filesystem::remove_all(sandbox_root, ec);
    std::cout << "cognitive_evolution_sandbox_smoke_test: OK\n";
    return 0;
}
