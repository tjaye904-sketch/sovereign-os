# Sovereign-OS Architecture Document

**Version:** 1.0  
**Date:** 2026-08-27  
**Status:** Complete Codebase Review

---

## 1. System Overview

Sovereign-OS is a C++20 cognitive operating system built as a modular, file-backed service architecture. The system provides identity management, goal tracking, semantic search, policy evaluation, agent scheduling, and workspace synchronization — all running locally without external dependencies.

### Design Principles
- **Local-first**: All data persisted to local filesystem under `data_root/`
- **Modular**: Each subsystem is an independent namespace with its own types and service class
- **Error-safe**: Custom `Result<T>` / `Error` enum pattern per module (no exceptions)
- **Thread-safe**: All services use `std::mutex` for internal state protection
- **Zero external deps**: Only C++20 standard library

---

## 2. Module Inventory & Completion Assessment

### 2.1 Core Subsystems (all headers present)

| # | Module | Header Files | Status | Notes |
|---|--------|-------------|--------|-------|
| 1 | **agent** | `agent_scheduler.hpp`, `agent_types.hpp` | Complete | Agent lifecycle, scheduling, roles |
| 2 | **archive** | `archive_service.hpp` | Complete | Document add/remove/search/export |
| 3 | **audit** | `audit_log.hpp`, `audit_types.hpp` | Complete | Append-only log with chain verification |
| 4 | **briefing** | `briefing_service.hpp`, `briefing_types.hpp` | Complete | Session summaries, context packs |
| 5 | **cognitive** | `cognitive_core.hpp`, `cognitive_service.hpp`, `cognitive_types.hpp` | Complete | Core cognitive loop, events, actions |
| 6 | **executor** | `action_executor.hpp`, `executor.hpp` | Complete | Sync/async action execution with handlers |
| 7 | **goals** | `goals_service.hpp`, `goals_types.hpp` | Complete | Goal CRUD with status/horizon/priority |
| 8 | **governance** | `governance_service.hpp`, `governance_types.hpp` | Complete | Principal, role, permission, policy |
| 9 | **identity** | `identity_service.hpp`, `identity_types.hpp` | Complete | Credential management, principal profiles |
| 10 | **memory** | `memory_engine.hpp`, `memory_types.hpp` | Complete | Memory records with type/access/config |
| 11 | **model_router** | `model_router.hpp`, `model_router_types.hpp` | Complete | Model endpoint selection and routing |
| 12 | **policy** | `policy_engine.hpp`, `policy_types.hpp` | Complete | Policy rule evaluation with context |
| 13 | **product** | `product.hpp`, `sovereign_host.hpp` | Complete | Main orchestrator (SovereignHost) |
| 14 | **semantic** | `semantic_store.hpp`, `semantic_types.hpp` | Complete | Embedding-based semantic search |
| 15 | **sync** | `sync_engine.hpp`, `sync_service.hpp` | Complete | Cross-workspace synchronization |
| 16 | **tools** | `tool_registry.hpp`, `tool_types.hpp` | Complete | Tool registration and metadata |
| 17 | **workspace** | `workspace_service.hpp`, `workspace_types.hpp` | Complete | Workspace lifecycle management |

### 2.2 Shared Infrastructure

| Module | Files | Status | Notes |
|--------|-------|--------|-------|
**Types:** `CredentialType` (Password/PublicKey/Token), `PrincipalProfile` (name/email/roles/credentials/timestamps)

### 3.7 Memory (`sovereign::memory`)

```
MemoryEngine: Local memory storage engine
├── Open(directory) / Close()
├── Store(MemoryRecord) → Result<MemoryId>
├── Retrieve(MemoryId) → Result<MemoryRecord>
├── Search(query, MemoryType) → vector<ScoredMemory>
├── Delete(MemoryId) → Result<void>
└── List(MemoryType) → vector<MemoryRecord>
```

**Types:** `MemoryType` (ShortTerm/LongTerm/Ephemeral), `MemoryAccess` (Private/Shared/System), `MemoryConfig`, `MemoryRecord` (id/content/type/access/timestamps)

### 3.8 Model Router (`sovereign::model_router`)

```
ModelRouter: Routes cognitive requests to appropriate models
├── Open(directory) / Close()
├── RegisterEndpoint(ModelEndpoint) → Result<void>
├── SelectEndpoint(ModelCapability, Context) → Result<ModelEndpoint>
├── Invoke(endpoint, request) → Result<ModelResponse>
├── ListEndpoints() → vector<ModelEndpoint>
└── RemoveEndpoint(endpointId) → Result<void>
```

**Types:** `ModelEndpoint` (url/capabilities/priority/health), `ModelCapability`, `ModelSelector`, `ModelError`, `ModelResult<T>`

### 3.9 Policy Engine (`sovereign::policy`)

```
PolicyEngine: Evaluates policies against contexts
├── Open(directory) / Close()
├── AddPolicy(PolicyRule) → Result<void>
├── Evaluate(ruleId, PolicyContext) → Result<PolicyResult>
├── ListPolicies() → vector<PolicyRule>
└── RemovePolicy(ruleId) → Result<void>
```

**Types:** `PolicyRule` (id/condition/action/effect/priority), `PolicyAction` (Allow/Deny/Log/Alert), `PolicyEffect`, `PolicyContext`, `PolicyError`, `PolicyResult`

### 3.10 Product (`sovereign::product`)

```
SovereignHost: Main orchestrator — bootstraps and coordinates all services
├── Initialize(config) → Result<void>
├── Start() → Result<void>
├── Stop() → void
├── GetService<T>() → T* (access to any registered service)
└── HealthCheck() → SystemHealth

Product: Configuration and metadata
├── name, version, description
├── data_root path
└── services list
```

### 3.11 Semantic Store (`sovereign::semantic`)

```
SemanticStore: Embedding-based semantic search
├── Open(directory) / Close()
├── Upsert(text, tags, author) → Result<SemanticRecordId>
├── Query(query, top_k) → vector<ScoredRecord>
└── Size() → std::size_t
```

**Types:** `Embedding` (vector<float>), `SemanticRecord` (id/author/text/embedding/embed_model/tags/timestamps), `ScoredRecord`, `SemanticError`

### 3.12 Sync Engine (`sovereign::sync`)

```
SyncEngine: Cross-workspace synchronization
├── Open(directory) / Close()
├── Sync(workspaceId, target) → Result<SyncResult>
├── GetStatus(workspaceId) → SyncStatus
└── Cancel(workspaceId) → Result<void>

SyncService: Higher-level sync orchestration
├── RegisterWorkspace(workspace) → Result<void>
├── UnregisterWorkspace(workspaceId) → Result<void>
├── ListWorkspaces() → vector<WorkspaceInfo>
└── HealthCheck() → SyncHealth
```

**Types:** `SyncStatus` (Idle/Syncing/Completed/Error), `SyncResult`, `SyncHealth`, `WorkspaceInfo`

### 3.13 Tools (`sovereign::tools`)

```
ToolRegistry: Tool registration and discovery
├── Register(ToolDefinition) → Result<void>
├── Unregister(toolId) → Result<void>
├── Get(toolId) → Result<ToolDefinition>
├── List() → vector<ToolDefinition>
└── Execute(toolId, params) → Result<ToolResult>
```

**Types:** `ToolDefinition` (id/name/description/params/result_schema), `ToolCapability`, `ToolError`, `ToolResult`

### 3.14 Workspace (`sovereign::workspace`)

```
WorkspaceService: Workspace lifecycle management
├── Open(directory) / Close()
├── Create(WorkspaceConfig) → Result<WorkspaceId>
├── Get(WorkspaceId) → Result<Workspace>
├── ListActive() / ListAll() → vector<Workspace>
├── SetStatus(WorkspaceId, WorkspaceStatus)
└── Delete(WorkspaceId) → Result<void>
```

**Types:** `WorkspaceConfig` (name/description/path/bound_services), `WorkspaceStatus` (Active/Inactive/Archived), `Workspace` (id/config/status/timestamps)

### 3.15 Audit Log (`sovereign::audit`)

```
AuditLog: Append-only integrity-preserving audit trail
├── Open(directory) / Close()
├── Append(AuditEntry) → Result<AuditEntryId>
├── List(limit) → vector<AuditEntry>
├── ListByCategory(category, limit) → vector<AuditEntry>
├── VerifyChain() → Result<bool>  ← Phase XII integrity check
└── ChainTip() → Result<std::string>
```

**Types:** `AuditEntry` (id/entry_type/category/payload/timestamp/chain_hash), `AuditEntryType`, `AuditError`

### 3.16 Briefing (`sovereign::briefing`)

```
BriefingService: Session briefing and context management
├── Open(directory) / Close()
├── CreateBriefing(BriefingConfig) → Result<BriefingId>
├── Get(BriefingId) → Result<Briefing>
├── AddEntry(BriefingId, BriefingEntry) → Result<void>
├── ListActive() → vector<Briefing>
└── CloseBriefing(BriefingId) → Result<void>
```

**Types:** `BriefingConfig` (title/description/entries), `BriefingEntry` (type/content/timestamp), `BriefingStatus`, `BriefingError`
**Types:** `BriefingConfig` (title/description/entries), `BriefingEntry` (type/content/timestamp), `BriefingStatus`, `BriefingError`

---

## 4. Data Flow Architecture

```
 SovereignHost (Main Orchestrator)
        │
 ┌──────┴──────┐                    ┌─────────┐
 │ Cognitive    │◄────────────────►│ Agent   │
 │ Service      │   Events/Actions │ Scheduler│
 └──────┬──────┘                    └─────────┘
        │
 ┌──────┴──────┐     ┌──────────────┐     ┌──────────────┐
 │  Executor    │────►│  Policy      │────►│ Governance   │
 │              │     │  Engine      │     │ Service      │
 └──────┬──────┘     └──────────────┘     └──────────────┘
        │
 ┌──────┴──────┐     ┌──────────────┐     ┌──────────────┐
 │  Model       │     │  Memory      │     │  Semantic    │
 │  Router      │     │  Engine      │     │  Store       │
 └──────┬──────┘     └──────────────┘     └──────────────┘
        │
 ┌──────┴──────┐     ┌──────────────┐     ┌──────────────┐
 │  Tools       │     │  Workspace   │     │   Sync       │
 │  Registry    │     │  Service     │     │  Engine      │
 └─────────────┘     └──────────────┘     └──────────────┘
        │
 ┌──────┴──────┐     ┌──────────────┐
 │   Goals      │     │   Audit      │
 │  Service     │     │   Log        │
 └─────────────┘     └──────────────┘
        │
 ┌──────┴──────┐     ┌──────────────┐
 │   Archive    │     │  Briefing    │
 │  Service     │     │  Service     │
 └─────────────┘     └──────────────┘
```

---

## 5. Shared Types

All modules share `PrincipalId` from `common/principal_id.hpp`:
```cpp
struct PrincipalId { std::array<std::uint8_t, 16> bytes{}; };
```

Every module defines its own:
- **Error enum** (`XxxError : std::uint8_t`) with `Ok = 0` as first value
- **Result<T> template** with `error`, `value`, and `Ok()` method
- **ID struct** (`XxxId`) as `std::array<std::uint8_t, 16>`
- **ID struct** (`XxxId`) as `std::array<std::uint8_t, 16>`

---

## 6. Completion Summary

### By Module (Header Files)
| Category | Headers Present | Estimated Implementation | Completion |
|----------|----------------|-------------------------|------------|
| Agent | 2 | Stubs only | ~15% |
| Cognitive | 3 | Stubs only | ~15% |
| Executor | 2 | Stubs only | ~15% |
| Goals | 2 | Stubs only | ~15% |
| Governance | 2 | Stubs only | ~15% |
| Identity | 2 | Stubs only | ~15% |
| Memory | 2 | Stubs only | ~15% |
| Model Router | 2 | Stubs only | ~15% |
| Policy | 2 | Stubs only | ~15% |
| Product | 2 | Partial (SovereignHost) | ~30% |
| Semantic | 2 | Stubs only | ~15% |
| Sync | 2 | Stubs only | ~15% |
| Tools | 2 | Stubs only | ~15% |
| Workspace | 2 | Stubs only | ~15% |
| Audit | 2 | Stubs only | ~15% |
| Briefing | 2 | Stubs only | ~15% |
| Archive | 1 | Stubs only | ~15% |
| **Common** | **1** | **Present** | **~50%** |
| **TOTAL** | **34** | **Headers complete** | **~20%** |

### Overall Assessment
- **API Design:** 100% — All interfaces defined and consistent
- **Type System:** 95% — All types present, shared PrincipalId working
- **Implementation:** ~15-20% — Most services have file-backed skeleton code
- **Integration:** ~10% — SovereignHost wires services together
- **Testing:** ~5% — Minimal test coverage
- **Documentation:** 80% — Architecture + spec docs present

### Overall Project Completion: **~25%**

---

## 7. Remaining Work Execution Plan

### Phase 1: Core Implementation (Priority: Critical)
1. Implement all service method bodies in each module
2. Complete SovereignHost integration wiring
3. Add `common/` utilities (logging, config parsing)

### Phase 2: Testing & Validation (Priority: High)
4. Unit tests for each service module
5. Integration tests for SovereignHost lifecycle
6. Property-based tests for policy engine

### Phase 3: Advanced Features (Priority: Medium)
7. Semantic embedding model integration
8. Cross-workspace sync protocol implementation
9. Audit chain integrity verification (Phase XII)

### Phase 4: Polish & Release (Priority: Low)
10. Performance optimization
11. Documentation and examples
12. Release packaging
| **common** | `principal_id.hpp` | Present | Shared PrincipalId type used across modules |