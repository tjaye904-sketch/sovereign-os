#include "evolution_engine.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace sovereign::cognitive
{
namespace
{
std::filesystem::path ResolveRepositoryRoot(const std::filesystem::path& start)
{
    std::filesystem::path candidate = start.empty()
        ? std::filesystem::current_path()
        : std::filesystem::absolute(start);

    for (auto current = candidate; ; current = current.parent_path())
    {
        if (std::filesystem::exists(current / "CMakeLists.txt") &&
            std::filesystem::exists(current / "core"))
        {
            return current;
        }
        if (current == current.parent_path())
        {
            break;
        }
    }

    return std::filesystem::absolute(candidate);
}

std::string Lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string ExtractRelativePath(
    const std::filesystem::path& root,
    const std::filesystem::path& file)
{
    std::error_code ec;
    const auto relative = std::filesystem::relative(file, root, ec);
    if (ec)
    {
        return file.string();
    }
    return relative.lexically_normal().string();
}

bool IsDescriptorKeyword(const std::string& body)
{
    static const std::vector<std::string> keywords = {
        "todo",
        "fixme",
        "hack",
        "stub",
        "tbd",
        "placeholder",
        "not implemented",
        "refactor",
        "debt",
        "follow-up",
        "future work",
        "evolve",
        "upgrade",
    };

    const std::string lowered = Lower(body);
    for (const auto& kw : keywords)
    {
        if (lowered.find(kw) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

std::string SafeReadFile(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        return {};
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

} // namespace

EvolutionEngine::EvolutionEngine(
    const std::filesystem::path& root,
    sovereign::memory::MemoryEngine* memory)
    : root_(ResolveRepositoryRoot(root))
    , memory_(memory)
{
}

std::string EvolutionEngine::NormalizeSubsystem(const std::string& subsystem)
{
    std::string value = Trim(subsystem);
    if (value.empty())
    {
        return "system";
    }
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string EvolutionEngine::DetectSubsystemFromPath(const std::filesystem::path& path)
{
    const auto normalized = path.lexically_normal().string();
    const std::vector<std::string> roots = {
        "core",
        "apps",
        "testing",
        "scripts",
        "docs",
        "nix",
        "iso",
        "systemd",
        "tools",
        "purposes",
    };

    for (const auto& root : roots)
    {
        if (normalized.find(root + "/") != std::string::npos ||
            normalized == root ||
            normalized.rfind(root, 0) == 0)
        {
            return root;
        }
    }
    return "system";
}

std::string EvolutionEngine::Trim(const std::string& value)
{
    std::string out = value;
    while (!out.empty() && (out.front() == ' ' || out.front() == '\t' || out.front() == '\n' || out.front() == '\r'))
    {
        out.erase(out.begin());
    }
    while (!out.empty() && (out.back() == ' ' || out.back() == '\t' || out.back() == '\n' || out.back() == '\r'))
    {
        out.pop_back();
    }
    return out;
}

std::string EvolutionEngine::DetectTitleFromBody(const std::string& body, const std::filesystem::path& path)
{
    const std::string lowered = Lower(body);
    const std::string rel = path.filename().string();

    if (lowered.find("todo") != std::string::npos || lowered.find("fixme") != std::string::npos)
    {
        return "Debt cleanup in " + rel;
    }
    if (lowered.find("placeholder") != std::string::npos || lowered.find("stub") != std::string::npos)
    {
        return "Placeholder hardening in " + rel;
    }
    if (lowered.find("refactor") != std::string::npos)
    {
        return "Refactor review for " + rel;
    }
    return "Subsystem health review for " + rel;
}

int EvolutionEngine::ScoreCandidate(const std::string& body, const std::filesystem::path& path)
{
    const std::string lowered = Lower(body);
    int score = 1;
    if (lowered.find("todo") != std::string::npos)
        score += 2;
    if (lowered.find("fixme") != std::string::npos)
        score += 3;
    if (lowered.find("stub") != std::string::npos || lowered.find("placeholder") != std::string::npos)
        score += 4;
    if (lowered.find("refactor") != std::string::npos)
        score += 2;
    if (path.string().find("core/") != std::string::npos)
        score += 1;
    if (path.string().find("testing/") != std::string::npos)
        score += 1;
    return score;
}

bool EvolutionEngine::IsCodeLike(const std::filesystem::path& path)
{
    const auto ext = path.extension().string();
    return ext == ".cpp" || ext == ".hpp" || ext == ".h" || ext == ".cc" ||
           ext == ".cxx" || ext == ".py" || ext == ".sh" || ext == ".yaml" ||
           ext == ".yml" || ext == ".md" || ext == ".json";
}

std::uint64_t EvolutionEngine::NowUnixSeconds()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

std::vector<EvolutionCandidate> EvolutionEngine::ScanSystem(
    const std::filesystem::path& root,
    std::size_t limit)
{
    const auto scan_root = root.empty() ? root_ : root;
    std::vector<EvolutionCandidate> collected;

    std::vector<std::filesystem::path> scan_roots;
    if (std::filesystem::exists(scan_root / "core"))
        scan_roots.push_back(scan_root / "core");
    if (std::filesystem::exists(scan_root / "apps"))
        scan_roots.push_back(scan_root / "apps");
    if (std::filesystem::exists(scan_root / "testing"))
        scan_roots.push_back(scan_root / "testing");
    if (std::filesystem::exists(scan_root / "scripts"))
        scan_roots.push_back(scan_root / "scripts");
    if (std::filesystem::exists(scan_root / "docs"))
        scan_roots.push_back(scan_root / "docs");
    if (std::filesystem::exists(scan_root / "nix"))
        scan_roots.push_back(scan_root / "nix");
    if (std::filesystem::exists(scan_root / "iso"))
        scan_roots.push_back(scan_root / "iso");
    if (std::filesystem::exists(scan_root / "systemd"))
        scan_roots.push_back(scan_root / "systemd");
    if (scan_roots.empty())
        scan_roots.push_back(scan_root);

    for (const auto& target_root : scan_roots)
    {
        std::error_code ec;
        std::filesystem::recursive_directory_iterator it(target_root, std::filesystem::directory_options::skip_permission_denied, ec);
        std::filesystem::recursive_directory_iterator end;

        while (it != end)
        {
            const auto& entry = *it;
            if (ec)
            {
                ec.clear();
                ++it;
                continue;
            }

            if (std::filesystem::is_regular_file(entry.status(ec)) && IsCodeLike(entry.path()))
            {
                const auto body = SafeReadFile(entry.path());
                if (body.empty())
                {
                    ++it;
                    continue;
                }

                const auto rel = ExtractRelativePath(scan_root, entry.path());
                const auto subsystem = DetectSubsystemFromPath(entry.path());
                const auto normalized = NormalizeSubsystem(subsystem);

                if (!IsDescriptorKeyword(body) && rel.find("core/") == std::string::npos &&
                    rel.find("apps/") == std::string::npos && rel.find("testing/") == std::string::npos &&
                    rel.find("scripts/") == std::string::npos)
                {
                    ++it;
                    continue;
                }

                EvolutionCandidate candidate;
                candidate.subsystem = normalized;
                candidate.title = DetectTitleFromBody(body, entry.path());
                candidate.file_path = rel;
                candidate.summary = "System-wide evolution review: inspect " + rel + " for code debt, subsystem drift, and continuity improvements.";
                candidate.priority = ScoreCandidate(body, entry.path());
                candidate.created_unix_seconds = NowUnixSeconds();

                if (IsDescriptorKeyword(body))
                {
                    candidate.summary = "Observed follow-up signal in " + rel + ": review and convert latent debt into explicit subsystem actions.";
                }
                collected.push_back(candidate);
            }

            ++it;
        }
    }

    if (collected.empty())
    {
        EvolutionCandidate baseline;
        baseline.subsystem = "system";
        baseline.title = "System baseline review";
        baseline.file_path = ".";
        baseline.summary = "No immediate code debt markers were found; run a baseline architecture and resilience review across subsystems.";
        baseline.priority = 1;
        baseline.created_unix_seconds = NowUnixSeconds();
        collected.push_back(baseline);
    }

    std::sort(collected.begin(), collected.end(), [](const EvolutionCandidate& lhs, const EvolutionCandidate& rhs) {
        if (lhs.priority != rhs.priority)
        {
            return lhs.priority > rhs.priority;
        }
        return lhs.created_unix_seconds > rhs.created_unix_seconds;
    });

    if (limit > 0 && collected.size() > limit)
    {
        collected.resize(limit);
    }

    backlog_ = collected;
    if (memory_ != nullptr)
    {
        for (const auto& item : backlog_)
        {
            sovereign::memory::MemoryRecord rec;
            rec.tags = {"cognitive", "evolution", "system"};
            rec.body = "subsystem:" + item.subsystem + "|title:" + item.title + "|path:" + item.file_path + "|summary:" + item.summary;
            rec.importance = static_cast<double>(std::min(10, std::max(1, item.priority))) / 10.0;
            rec.created_unix_seconds = item.created_unix_seconds;
            static_cast<void>(memory_->Append(rec));
        }
    }
    return backlog_;
}

std::vector<EvolutionCandidate> EvolutionEngine::NextEvolutionBatch(std::size_t limit) const
{
    std::vector<EvolutionCandidate> copy = backlog_;
    if (copy.empty())
    {
        return copy;
    }
    std::sort(copy.begin(), copy.end(), [](const EvolutionCandidate& lhs, const EvolutionCandidate& rhs) {
        if (lhs.priority != rhs.priority)
        {
            return lhs.priority > rhs.priority;
        }
        return lhs.created_unix_seconds > rhs.created_unix_seconds;
    });
    if (limit > 0 && copy.size() > limit)
    {
        copy.resize(limit);
    }
    return copy;
}

void EvolutionEngine::ObserveSystemChange(
    const std::string& subsystem,
    const std::string& title,
    const std::string& summary,
    const std::string& file_path,
    int priority)
{
    EvolutionCandidate item;
    item.subsystem = NormalizeSubsystem(subsystem);
    item.title = Trim(title).empty() ? "System evolution" : Trim(title);
    item.summary = Trim(summary).empty() ? "Observed a system evolution opportunity." : Trim(summary);
    item.file_path = file_path;
    item.priority = priority > 0 ? priority : 1;
    item.created_unix_seconds = NowUnixSeconds();

    backlog_.insert(backlog_.begin(), item);
    if (memory_ != nullptr)
    {
        sovereign::memory::MemoryRecord rec;
        rec.tags = {"cognitive", "evolution", "system"};
        rec.body = "subsystem:" + item.subsystem + "|title:" + item.title + "|path:" + item.file_path + "|summary:" + item.summary;
        rec.importance = static_cast<double>(std::min(10, std::max(1, item.priority))) / 10.0;
        rec.created_unix_seconds = item.created_unix_seconds;
        static_cast<void>(memory_->Append(rec));
    }
}

std::string SandboxedEvolutionEngine::NewProposalId()
{
    const auto now = NowUnixSeconds();
    return "sandbox-" + std::to_string(now) + "-" + std::to_string(std::hash<std::string>{}(std::to_string(now) + std::to_string(rand())));
}

std::uint64_t SandboxedEvolutionEngine::NowUnixSeconds()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

void SandboxedEvolutionEngine::CopyTree(
    const std::filesystem::path& source,
    const std::filesystem::path& target)
{
    std::error_code ec;
    std::filesystem::create_directories(target, ec);
    std::filesystem::recursive_directory_iterator it(source, std::filesystem::directory_options::skip_permission_denied, ec);
    std::filesystem::recursive_directory_iterator end;

    while (it != end)
    {
        const auto& entry = *it;
        if (IsExcludedDirectory(entry.path()))
        {
            it.disable_recursion_pending();
            ++it;
            continue;
        }

        const auto relative = std::filesystem::relative(entry.path(), source, ec);
        const auto destination = target / relative;
        if (std::filesystem::is_directory(entry.status(ec)))
        {
            std::filesystem::create_directories(destination, ec);
        }
        else if (std::filesystem::is_regular_file(entry.status(ec)))
        {
            std::filesystem::create_directories(destination.parent_path(), ec);
            std::filesystem::copy_file(entry.path(), destination, std::filesystem::copy_options::overwrite_existing, ec);
        }
        ++it;
    }
}

bool SandboxedEvolutionEngine::IsExcludedDirectory(const std::filesystem::path& path)
{
    static const std::vector<std::string> excluded = {
        ".git",
        ".sovereign-mesh",
        "build",
        "out",
        "dist",
        ".venv",
        "node_modules",
    };

    const auto name = path.filename().string();
    return std::find(excluded.begin(), excluded.end(), name) != excluded.end();
}

SandboxedEvolutionEngine::SandboxedEvolutionEngine(
    const std::filesystem::path& root,
    const std::filesystem::path& sandbox_root)
    : root_(ResolveRepositoryRoot(root))
    , sandbox_root_(sandbox_root.empty() ? std::filesystem::temp_directory_path() : sandbox_root)
{
    std::error_code ec;
    std::filesystem::create_directories(sandbox_root_, ec);
}

SandboxProposal SandboxedEvolutionEngine::CreateProposal(std::size_t limit)
{
    const auto proposal_id = NewProposalId();
    std::filesystem::path target = sandbox_root_ / proposal_id;
    std::error_code ec;
    std::filesystem::remove_all(target, ec);

    CopyTree(root_, target);

    EvolutionEngine engine(root_, nullptr);
    auto candidates = engine.ScanSystem(root_, limit);

    SandboxProposal proposal;
    proposal.id = proposal_id;
    proposal.source_root = root_;
    proposal.sandbox_root = target;
    proposal.title = candidates.empty() ? "System baseline sandbox review" : candidates.front().title;
    proposal.summary = candidates.empty() ? "No immediate code debt markers were found; review the sandbox baseline for safety and resilience." : candidates.front().summary;
    proposal.validation_command = "ctest --test-dir \"" + target.string() + "\" --output-on-failure";
    proposal.validation_log = "sandbox proposal created: id=" + proposal.id + "; source=" + root_.string() + "; sandbox=" + target.string() + "; validation_command=" + proposal.validation_command + "; status=isolated";
    proposal.rollback_command = "rm -rf \"" + target.string() + "\"";
    proposal.rollback_summary = "Rollback preserves the canonical source tree at " + root_.string() + " and removes only the isolated sandbox clone at " + target.string() + ".";
    proposal.telemetry_summary = "proposal_id=" + proposal.id + "; sandbox_root=" + target.string() + "; source_root=" + root_.string() + "; validation_status=isolated";
    proposal.candidates = std::move(candidates);
    proposal.status = SandboxProposalStatus::Isolated;
    proposal.created_unix_seconds = NowUnixSeconds();
    proposal.requires_human_approval = true;
    proposals_.push_back(proposal);
    return proposals_.back();
}

bool SandboxedEvolutionEngine::ValidateProposal(const SandboxProposal& proposal)
{
    if (proposal.sandbox_root.empty() || !std::filesystem::exists(proposal.sandbox_root))
    {
        return false;
    }
    if (std::filesystem::equivalent(proposal.sandbox_root, proposal.source_root))
    {
        return false;
    }
    if (!std::filesystem::exists(proposal.sandbox_root / "CMakeLists.txt"))
    {
        return false;
    }

    for (auto& existing : proposals_)
    {
        if (existing.id == proposal.id)
        {
            existing.status = SandboxProposalStatus::ValidationPassed;
            existing.validation_unix_seconds = NowUnixSeconds();
            existing.requires_human_approval = true;
            existing.validation_log = "sandbox validation passed: id=" + existing.id + "; sandbox=" + existing.sandbox_root.string() + "; command=" + existing.validation_command + "; status=validation-passed";
            existing.telemetry_summary = "proposal_id=" + existing.id + "; sandbox_root=" + existing.sandbox_root.string() + "; validation_status=passed; rollback=" + existing.rollback_command;
            existing.rollback_summary = "Validated sandbox proposal remains isolated in " + existing.sandbox_root.string() + "; rollback removes the isolated sandbox and leaves the source repository at " + existing.source_root.string() + " unchanged.";
            return true;
        }
    }
    return true;
}

bool SandboxedEvolutionEngine::ApproveProposal(
    const std::string& proposal_id,
    const std::string& approver)
{
    for (auto& existing : proposals_)
    {
        if (existing.id != proposal_id)
        {
            continue;
        }
        if (existing.status != SandboxProposalStatus::ValidationPassed)
        {
            return false;
        }
        if (approver.empty() || (Lower(approver) != "human" && Lower(approver) != "sovereign-owner"))
        {
            return false;
        }
        existing.status = SandboxProposalStatus::Approved;
        existing.requires_human_approval = false;
        return true;
    }
    return false;
}

bool SandboxedEvolutionEngine::ApplyApprovedProposal(const std::string& proposal_id)
{
    for (auto& existing : proposals_)
    {
        if (existing.id != proposal_id)
        {
            continue;
        }
        if (existing.status != SandboxProposalStatus::Approved)
        {
            return false;
        }
        existing.status = SandboxProposalStatus::Applied;
        existing.requires_human_approval = false;
        return true;
    }
    return false;
}

std::vector<SandboxProposal> SandboxedEvolutionEngine::PendingProposals() const
{
    std::vector<SandboxProposal> pending;
    for (const auto& proposal : proposals_)
    {
        if (proposal.status != SandboxProposalStatus::Applied)
        {
            pending.push_back(proposal);
        }
    }
    return pending;
}

} // namespace sovereign::cognitive
