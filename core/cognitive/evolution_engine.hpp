#pragma once

#include "memory_engine.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace sovereign::cognitive
{

struct EvolutionCandidate
{
    std::string subsystem;
    std::string title;
    std::string summary;
    std::string file_path;
    int priority{0};
    std::uint64_t created_unix_seconds{0};
};

class EvolutionEngine
{
public:
    explicit EvolutionEngine(
        const std::filesystem::path& root = std::filesystem::current_path(),
        sovereign::memory::MemoryEngine* memory = nullptr);

    std::vector<EvolutionCandidate> ScanSystem(
        const std::filesystem::path& root = {},
        std::size_t limit = 10);
    std::vector<EvolutionCandidate> NextEvolutionBatch(std::size_t limit = 10) const;
    void ObserveSystemChange(
        const std::string& subsystem,
        const std::string& title,
        const std::string& summary,
        const std::string& file_path = {},
        int priority = 1);

private:
    static std::string NormalizeSubsystem(const std::string& subsystem);
    static std::string DetectSubsystemFromPath(const std::filesystem::path& path);
    static std::string DetectTitleFromBody(const std::string& body, const std::filesystem::path& path);
    static std::string Trim(const std::string& value);
    static int ScoreCandidate(const std::string& body, const std::filesystem::path& path);
    static bool IsCodeLike(const std::filesystem::path& path);
    static std::uint64_t NowUnixSeconds();

    std::filesystem::path root_;
    sovereign::memory::MemoryEngine* memory_;
    std::vector<EvolutionCandidate> backlog_;
};

enum class SandboxProposalStatus : std::uint8_t
{
    Draft = 0,
    Isolated = 1,
    ValidationPending = 2,
    ValidationPassed = 3,
    AwaitingApproval = 4,
    Approved = 5,
    Applied = 6
};

struct SandboxProposal
{
    std::string id;
    std::filesystem::path source_root;
    std::filesystem::path sandbox_root;
    std::string title;
    std::string summary;
    std::string validation_command;
    std::string validation_log;
    std::string rollback_command;
    std::string rollback_summary;
    std::string telemetry_summary;
    std::vector<EvolutionCandidate> candidates;
    SandboxProposalStatus status{SandboxProposalStatus::Isolated};
    std::uint64_t created_unix_seconds{0};
    std::uint64_t validation_unix_seconds{0};
    bool requires_human_approval{true};
};

class SandboxedEvolutionEngine
{
public:
    explicit SandboxedEvolutionEngine(
        const std::filesystem::path& root = std::filesystem::current_path(),
        const std::filesystem::path& sandbox_root = std::filesystem::temp_directory_path());

    SandboxProposal CreateProposal(std::size_t limit = 5);
    bool ValidateProposal(const SandboxProposal& proposal);
    bool ApproveProposal(const std::string& proposal_id, const std::string& approver = "human");
    bool ApplyApprovedProposal(const std::string& proposal_id);
    std::vector<SandboxProposal> PendingProposals() const;

private:
    static std::string NewProposalId();
    static std::uint64_t NowUnixSeconds();
    static void CopyTree(const std::filesystem::path& source, const std::filesystem::path& target);
    static bool IsExcludedDirectory(const std::filesystem::path& path);

    std::filesystem::path root_;
    std::filesystem::path sandbox_root_;
    std::vector<SandboxProposal> proposals_;
};

} // namespace sovereign::cognitive
