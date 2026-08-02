#pragma once

// Local-first peer sync via durable bundles (folder exchange).
// Spec posture: human authority preserved — import never auto-executes
// or auto-approves; secrets/credentials never included in bundles.

#include "audit_log.hpp"
#include "goals_service.hpp"
#include "memory_engine.hpp"
#include "semantic_store.hpp"
#include "sync_service.hpp"

#include <filesystem>
#include <string>

namespace sovereign::sync
{

enum class PeerError : std::uint8_t
{
    Ok = 0,
    InvalidArgument,
    IOError,
    SchemaError
};

template <typename T>
struct PeerResult
{
    PeerError error{PeerError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == PeerError::Ok;
    }
};

struct PeerBundleSummary
{
    std::size_t goals{0};
    std::size_t memory{0};
    std::size_t semantic{0};
    std::size_t audit{0};
    std::string path;
};

class PeerBundleService
{
public:
    PeerBundleService(
        goals::GoalsService& goals,
        memory::MemoryEngine& memory,
        semantic::SemanticStore& semantic,
        audit::AuditLog& audit,
        SyncService& journal);

    // Write a portable directory bundle under dest_dir/bundle-<ts>/
    PeerResult<PeerBundleSummary> ExportTo(
        const std::filesystem::path& dest_dir,
        const PrincipalId& actor);

    // Read bundle_dir and merge: memory/semantic/audit append; goals by id upsert
    PeerResult<PeerBundleSummary> ImportFrom(
        const std::filesystem::path& bundle_dir,
        const PrincipalId& actor);

private:
    goals::GoalsService& goals_;
    memory::MemoryEngine& memory_;
    semantic::SemanticStore& semantic_;
    audit::AuditLog& audit_;
    SyncService& journal_;
};

} // namespace sovereign::sync
