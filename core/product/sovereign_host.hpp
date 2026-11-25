#pragma once

// Local product host: opens data roots and wires core product services.
// Not a network API gateway — in-process composition root for v1.

#include "action_executor.hpp"
#include "agent_scheduler.hpp"
#include "api_gateway.hpp"
#include "archive_service.hpp"
#include "audit_log.hpp"
#include "briefing_service.hpp"
#include "../capability/capability_gate.hpp"
#include "../capability/capability_registry.hpp"
#include "../node/node_registry.hpp"
#include "cognitive_service.hpp"
#include "desktop_operator.hpp"
#include "evolution_engine.hpp"
#include "external_gateway.hpp"
#include "domain_profile.hpp"
#include "goals_service.hpp"
#include "governance_service.hpp"
#include "identity_service.hpp"
#include "identity_store_file.hpp"
#include "memory_engine.hpp"
#include "model_router.hpp"
#include "policy_engine.hpp"
#include "peer_bundle.hpp"
#include "purpose_runtime.hpp"
#include "sync_service.hpp"
#include "workspace_service.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace sovereign::product
{

struct HostConfig
{
    std::filesystem::path data_root;
    std::string ollama_base{"http://127.0.0.1:11434"};
    std::string default_model{"openclaw"};
};

class SovereignHost
{
public:
    explicit SovereignHost(HostConfig config);

    // Opens stores under data_root/{identity,governance,memory,workspace,archive,sync}
    bool Open();
    void Close() noexcept;

    audit::AuditLog& Audit() noexcept { return audit_; }
    goals::GoalsService& Goals() noexcept { return goals_; }
    briefing::BriefingService& Briefing() noexcept { return *briefing_; }
    agent::AgentScheduler& Agent() noexcept { return agent_; }
    sync::PeerBundleService& Peer() noexcept { return *peer_; }
    semantic::SemanticStore& Semantic() noexcept { return semantic_; }
    identity::IdentityService& Identity() noexcept { return *identity_; }
    governance::GovernanceService& Governance() noexcept { return governance_; }
    policy::PolicyEngine& Policy() noexcept { return policy_; }
    memory::MemoryEngine& Memory() noexcept { return memory_; }
    workspace::WorkspaceService& Workspace() noexcept { return workspace_; }
    archive::ArchiveService& Archive() noexcept { return archive_; }
    sync::SyncService& Sync() noexcept { return sync_; }
    model_router::ModelRouter& Models() noexcept { return *models_; }
    cognitive::CuriosityEngine& Curiosity() noexcept { return *curiosity_; }
    cognitive::EvolutionEngine& Evolution() noexcept { return *evolution_; }
    cognitive::SandboxedEvolutionEngine& SandboxEvolution() noexcept { return *sandbox_evolution_; }
    DesktopOperatorGateway& Desktop() noexcept { return *desktop_; }
    ExternalInterfaceGateway& External() noexcept { return *external_; }
    ApiGatewayService& ApiGateway() noexcept { return *api_gateway_; }
    capability::CapabilityRegistry& Capabilities() noexcept { return capabilities_; }
    capability::CapabilityGate& Gate() noexcept { return *capability_gate_; }
    node::NodeRegistry& Nodes() noexcept { return nodes_; }
    PurposeRuntime& Purpose() noexcept { return purpose_; }
    cognitive::CognitiveService& Cognitive() noexcept { return *cognitive_; }
    executor::ActionExecutor& Executor() noexcept { return *executor_; }

    const std::filesystem::path& DataRoot() const noexcept { return config_.data_root; }
    const std::string& DefaultModel() const noexcept { return config_.default_model; }
    const std::string& OllamaBase() const noexcept { return config_.ollama_base; }

private:
    HostConfig config_;
    audit::AuditLog audit_;
    goals::GoalsService goals_;
    agent::AgentScheduler agent_;
    semantic::SemanticStore semantic_;
    std::unique_ptr<briefing::BriefingService> briefing_;
    std::unique_ptr<sync::PeerBundleService> peer_;
    identity::FileIdentityStore identity_store_;
    std::unique_ptr<identity::IdentityService> identity_;
    governance::GovernanceService governance_;
    policy::PolicyEngine policy_;
    memory::MemoryEngine memory_;
    workspace::WorkspaceService workspace_;
    archive::ArchiveService archive_;
    sync::SyncService sync_;
    std::unique_ptr<model_router::ModelRouter> models_;
    std::unique_ptr<cognitive::CuriosityEngine> curiosity_;
    std::unique_ptr<cognitive::EvolutionEngine> evolution_;
    std::unique_ptr<cognitive::SandboxedEvolutionEngine> sandbox_evolution_;
    std::unique_ptr<DesktopOperatorGateway> desktop_;
    std::unique_ptr<ExternalInterfaceGateway> external_;
    std::unique_ptr<ApiGatewayService> api_gateway_;
    capability::CapabilityRegistry capabilities_{};
    std::unique_ptr<capability::CapabilityGate> capability_gate_;
    node::NodeRegistry nodes_{};
    PurposeRuntime purpose_{};
    std::unique_ptr<cognitive::CognitiveService> cognitive_;
    std::unique_ptr<executor::ActionExecutor> executor_;
    bool open_{false};
};

} // namespace sovereign::product
