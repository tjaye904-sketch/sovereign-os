#include "sovereign_host.hpp"

#include <cstdlib>

namespace sovereign::product
{

SovereignHost::SovereignHost(HostConfig config)
    : config_(std::move(config))
{
    if (const char* m = std::getenv("SOVEREIGN_OLLAMA_MODEL");
        m != nullptr && m[0] != '\0')
    {
        config_.default_model = m;
    }
    if (const char* o = std::getenv("OLLAMA_HOST");
        o != nullptr && o[0] != '\0')
    {
        config_.ollama_base = o;
    }
}

bool SovereignHost::Open()
{
    if (open_)
    {
        return true;
    }
    std::error_code ec;
    std::filesystem::create_directories(config_.data_root, ec);
    if (ec)
    {
        return false;
    }

    if (semantic_.Open(config_.data_root / "semantic") != semantic::SemanticError::Ok)
    {
        return false;
    }
    if (agent_.Open(config_.data_root / "agent") != agent::AgentError::Ok)
    {
        return false;
    }
    if (goals_.Open(config_.data_root / "goals") != goals::GoalsError::Ok)
    {
        return false;
    }
    if (policy_.Open(config_.data_root / "policy") != policy::PolicyError::Ok)
        return false;
    if (audit_.Open(config_.data_root / "audit") != audit::AuditError::Ok)
    {
        return false;
    }
    if (identity_store_.Open(config_.data_root / "identity") !=
        identity::IdentityError::Ok)
    {
        return false;
    }
    identity::IdentityConfig icfg = identity::DefaultIdentityConfig();
    icfg.data_directory = config_.data_root / "identity";
    identity_ = std::make_unique<identity::IdentityService>(identity_store_, icfg);

    if (governance_.Open(config_.data_root / "governance") !=
        governance::GovernanceError::Ok)
    {
        return false;
    }
    if (memory_.Open(config_.data_root / "memory") != memory::MemoryError::Ok)
    {
        return false;
    }
    if (workspace_.Open(config_.data_root / "workspace") !=
        workspace::WorkspaceError::Ok)
    {
        return false;
    }
    if (archive_.Open(config_.data_root / "archive") != archive::ArchiveError::Ok)
    {
        return false;
    }
    if (sync_.Open(config_.data_root / "sync") != sync::SyncError::Ok)
    {
        return false;
    }

    models_ = std::make_unique<model_router::ModelRouter>(config_.ollama_base);
    cognitive_ =
        std::make_unique<cognitive::CognitiveService>(governance_, &memory_);
    executor_ = std::make_unique<executor::ActionExecutor>(*cognitive_, &policy_);
    briefing_ = std::make_unique<briefing::BriefingService>(
        goals_, governance_, memory_, workspace_, audit_);
    peer_ = std::make_unique<sync::PeerBundleService>(
        goals_, memory_, semantic_, audit_, sync_);

    open_ = true;
    return true;
}

void SovereignHost::Close() noexcept
{
    if (!open_)
    {
        return;
    }
    sync_.Close();
    audit_.Close();
    goals_.Close();
    agent_.Close();
    semantic_.Close();
    archive_.Close();
    workspace_.Close();
    memory_.Close();
    governance_.Close();
    identity_store_.Close();
    open_ = false;
}

} // namespace sovereign::product
