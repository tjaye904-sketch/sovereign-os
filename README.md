# Sovereign-OS

### A Sovereign, Federated Cognitive Operating System

**Sovereign-OS is a modular, governed, federated cognitive operating system designed to coordinate persistent intelligence, identity, memory, governance, distributed execution, and specialized workspaces across multiple devices and environments.**

Built on a NixOS foundation, Sovereign-OS aims to provide a sovereign control plane above the underlying operating system. It brings together AI model orchestration, runtime services, persistent state, policy enforcement, capability management, workspace coordination, and federated infrastructure.

Sovereign-OS is not simply a chatbot, an AI assistant, an agent framework, or a collection of automation scripts. It is an evolving systems architecture for coordinating intelligence, computation, knowledge, and execution under explicit authority and governance boundaries.

> **Core principle: Nodes provide compute, not authority.**

Federated nodes and workspaces may contribute processing, storage, models, tools, and specialized capabilities. Their participation does not automatically grant them sovereign authority. Identity, authorization, policy, and governance remain explicit architectural concerns.

---

## Table of Contents

- [Vision](#vision)
- [Architectural Overview](#architectural-overview)
- [Foundational Subsystems](#foundational-subsystems)
- [Federated Workspace Architecture](#federated-workspace-architecture)
- [Intelligence and Model Orchestration](#intelligence-and-model-orchestration)
- [Memory and Persistent Knowledge](#memory-and-persistent-knowledge)
- [Governance, Identity, and Security](#governance-identity-and-security)
- [Runtime and Lifecycle Infrastructure](#runtime-and-lifecycle-infrastructure)
- [Federation and Distributed Execution](#federation-and-distributed-execution)
- [System Capabilities](#system-capabilities)
- [Engineering Principles](#engineering-principles)
- [Development Status and Verification](#development-status-and-verification)
- [Long-Term Direction](#long-term-direction)

---

## Vision

The objective of Sovereign-OS is to establish a persistent computational environment in which intelligence, knowledge, infrastructure, applications, automation, and distributed resources can operate as coordinated parts of one governed system.

Rather than depending on one AI model, one machine, one cloud provider, or one execution environment, Sovereign-OS is designed to support replaceable intelligence providers, specialized operational domains, and distributed computational resources while retaining a coherent sovereign authority model.

Its architecture is organized around six ideas:

- **Sovereignty:** Authority and policy are explicit and governed.
- **Federation:** Independent environments can participate without erasing their boundaries.
- **Modularity:** Capabilities are separated into subsystems with defined responsibilities.
- **Continuity:** Persistent state and memory support operation beyond individual sessions.
- **Controlled autonomy:** Automated and intelligent actions remain bounded by authorization and policy.
- **Verifiability:** Runtime behavior and subsystem integration should be testable and traceable.

## Architectural Overview

Sovereign-OS is conceived as a layered architecture.

### 1. NixOS Infrastructure Foundation

NixOS provides the declarative and reproducible operating-system foundation.

Responsibilities include:

- System configuration and provisioning.
- Reproducible environments.
- Package and dependency management.
- Service lifecycle management.
- Hardware and resource integration.
- Infrastructure-level isolation.
- Deployment and recovery foundations.

### 2. Sovereign Control Plane

The control plane coordinates authority, identity, governance, and system-wide operations.

Responsibilities include:

- Sovereign identity.
- Authorization and capability control.
- Policy enforcement.
- Workspace ownership and coordination.
- Federation membership and trust boundaries.
- Runtime orchestration.
- Execution authorization.
- Operational oversight.

### 3. Cognitive and Coordination Runtime

The cognitive runtime coordinates reasoning services and operational planning.

Responsibilities include:

- AI model routing.
- Context management.
- Planning and task decomposition.
- Scheduling and coordination.
- Agent and tool orchestration.
- Workspace-aware operations.
- Execution planning and tracking.

### 4. Execution and Integration Layer

This layer connects authorized plans and requests to operational capabilities.

Responsibilities include:

- Local and distributed execution.
- Tool and capability invocation.
- Service orchestration.
- Infrastructure management.
- Automation and background operations.
- Resource coordination.
- Execution tracking and recovery.

### 5. Persistence and Observability Layer

This layer supports continuity, traceability, and operational accountability.

Responsibilities include:

- Durable runtime state.
- Event processing.
- Subscription lifecycle management.
- State transitions and migrations.
- Audit records.
- Capability traces.
- Execution explanations.
- Recovery and verification.

These layers describe the intended architecture. Individual capabilities become operational as their respective components are implemented, integrated, and verified.

## Foundational Subsystems

The Federation Core brings together foundational capabilities that support coordinated operation across the system.

| Subsystem | Primary responsibility |
|---|---|
| **Identity** | Identifies sovereign entities, nodes, services, and participants. |
| **Governance** | Defines authority boundaries and governance decisions. |
| **Policy Engine** | Evaluates and enforces applicable policies. |
| **Capability Management** | Controls which actors may invoke which capabilities. |
| **Memory** | Provides persistent knowledge and contextual continuity. |
| **Planner** | Decomposes goals and coordinates execution plans. |
| **Scheduler** | Coordinates work, timing, and resource availability. |
| **Workspace Controller** | Manages workspace boundaries and lifecycle. |
| **Model Router** | Selects and routes requests to appropriate intelligence providers. |
| **Audit and Trace** | Records operational activity and supports investigation. |
| **Secret Management** | Provides protected handling of credentials and secrets. |
| **Runtime Lifecycle** | Coordinates runtime state, subscriptions, and lifecycle transitions. |
| **Federation Manager** | Coordinates membership and participation across federated environments. |

The exact implementation boundaries and maturity of each subsystem are tracked through the project's engineering work and runtime milestones.

## Federated Workspace Architecture

Sovereign-OS is designed to support specialized workspaces as distinct operational domains within a unified sovereign environment.

A workspace is more than a folder or prompt collection. It is an organizational and operational boundary for responsibilities, context, capabilities, and workflows.

The architecture defines sixteen primary workspace domains:

| Workspace | Intended domain |
|---|---|
| **Development** | Software engineering, repositories, builds, testing, debugging, and code analysis. |
| **Knowledge** | Knowledge acquisition, organization, retrieval, and contextual relationships. |
| **Business** | Business operations, project coordination, planning, and operational workflows. |
| **Finance** | Financial information management, analysis, budgeting, and related workflows. |
| **Life Management** | Personal organization, scheduling, task management, and planning. |
| **Health** | Health information organization and health-related coordination workflows. |
| **Research** | Investigation, experimentation, research coordination, and synthesis. |
| **Security** | Security policy, access control, threat analysis, and monitoring. |
| **Infrastructure** | Provisioning, configuration, deployment, services, and resource monitoring. |
| **Automation** | Event-driven workflows, scheduled operations, and task orchestration. |
| **Robotics** | Robotics integration, hardware coordination, and physical execution interfaces. |
| **Communications** | Messaging, information exchange, and participant coordination. |
| **Creative** | Creative development, content production, and multimedia workflows. |
| **Intelligence** | Model orchestration, reasoning, analysis, and intelligent task execution. |
| **Governance** | Policy administration, authority management, and oversight. |
| **Federation** | Federation membership, node coordination, trust relationships, and distributed capabilities. |

### Workspace Design Principles

Workspaces are intended to provide:

- Defined operational responsibilities.
- Explicit context and data boundaries.
- Governed capability access.
- Controlled inter-workspace communication.
- Independent operation where appropriate.
- Authorized collaboration where needed.
- Persistent operational context.
- Lifecycle and federation awareness.

The goal is specialization without fragmentation: workspaces can operate within their own scopes while participating in coordinated system-wide workflows when authorized.

## Intelligence and Model Orchestration

Sovereign-OS is designed to coordinate multiple intelligence providers and computational models through a model-routing and orchestration architecture.

Intended capabilities include:

- Model selection and routing.
- Task-oriented model assignment.
- Specialized model utilization.
- Context-aware reasoning coordination.
- Multi-model workflows.
- Planning and task decomposition.
- Tool and capability coordination.
- Separation of intelligence providers from sovereign authority.

Models provide intelligence capabilities. They do not become the sovereign authority of the operating system. This separation is intended to let the system evolve independently of any one model, provider, or inference environment.

## Memory and Persistent Knowledge

Persistent memory is a foundational architectural concern.

The system is intended to maintain continuity across workspaces, runtime operations, and distributed participants through structured and durable state.

Intended responsibilities include:

- Persistent contextual information.
- Structured knowledge management.
- Runtime state persistence.
- Historical operational records.
- Event-driven state changes.
- Contextual retrieval.
- Workspace-associated knowledge.
- State migration and recovery.
- Traceable state transitions.

Memory is treated as a system capability, not merely a conversational feature.

## Governance, Identity, and Security

Sovereign authority requires explicit boundaries between identity, authorization, capabilities, execution, and infrastructure.

The governance and security architecture is intended to encompass:

- Identity-based authorization.
- Capability-oriented access control.
- Policy evaluation and enforcement.
- Execution authorization.
- Workspace isolation and boundaries.
- Federation trust boundaries.
- Secret management.
- Auditability and operational traceability.
- Governed coordination between subsystems.

Federated participation does not imply unrestricted access or authority. A node or workspace should receive only the permissions and capabilities granted within the applicable governance model.

## Runtime and Lifecycle Infrastructure

The runtime is responsible for the internal operational behavior and continuity of Sovereign-OS.

Runtime engineering areas include:

- Event processing.
- Subscription management and lifecycle.
- Runtime identity.
- Persistent state.
- Explicit state transitions.
- Capability tracing.
- Trace querying and pagination.
- Migration handling.
- Recovery operations.
- Execution explanations.
- Integration verification.
- Regression testing.
- Lifecycle coordination.

The runtime is developed through incremental engineering milestones. Each milestone should build on verified foundations rather than assuming that architectural intent alone establishes operational behavior.

## Federation and Distributed Execution

The federation architecture is intended to extend Sovereign-OS beyond a single machine.

Potential participating environments include:

- Personal computers.
- Dedicated servers.
- Virtual machines.
- Cloud infrastructure.
- Edge devices.
- Specialized compute systems.
- Robotics platforms.
- Additional Sovereign-OS installations.

Federation is more than network connectivity. It requires explicit membership, identity, trust, policy, capability boundaries, and coordination.

The architecture separates **computational participation** from **sovereign authority**. Distributed resources may contribute processing, storage, models, tools, and execution environments while remaining within their defined authorization and governance boundaries.

## System Capabilities

The following is a consolidated view of the capabilities the architecture is designed to support.

### Operating Infrastructure

- Declarative operating-system foundation.
- Reproducible infrastructure.
- Service and runtime lifecycle management.
- Resource and environment coordination.
- Deployment and recovery foundations.

### Artificial Intelligence

- Multi-model orchestration.
- Model routing.
- Cognitive task coordination.
- Planning and reasoning workflows.
- Tool-enabled intelligent operations.
- Model-provider independence.

### Federation and Workspaces

- Multi-node architecture.
- Federated workspace domains.
- Distributed capability coordination.
- Workspace isolation.
- Cross-workspace collaboration.
- Governed resource participation.

### Governance and Security

- Sovereign identity.
- Authority management.
- Policy enforcement.
- Capability authorization.
- Trust boundaries.
- Secret handling.
- Auditability.

### Knowledge and Memory

- Persistent memory architecture.
- Contextual continuity.
- Knowledge organization.
- Runtime persistence.
- Historical traceability.
- State migration and recovery.

### Execution and Automation

- Task orchestration.
- Scheduling.
- Event-driven automation.
- Service coordination.
- Authorized tool execution.
- Distributed execution workflows.
- Lifecycle management.

### Reliability and Observability

- Explicit runtime state transitions.
- Event and subscription management.
- Capability tracing.
- Audit records.
- Migration and recovery.
- Regression testing.
- Integration verification.
- Operational explanations.

## Engineering Principles

Sovereign-OS follows several engineering principles:

1. **Explicit contracts:** Subsystems should expose clear responsibilities, interfaces, and invariants.
2. **Bounded authority:** Identity, policy, and authorization should determine what an actor can do.
3. **Separation of concerns:** Models, workspaces, nodes, runtime services, and governance should not be conflated.
4. **Persistent state:** Important runtime state should have defined durability and recovery behavior.
5. **Incremental delivery:** Capabilities should be implemented in bounded, verifiable milestones.
6. **Regression discipline:** Changes should preserve previously established behavior.
7. **Traceability:** Significant operations and capability use should be explainable and inspectable.
8. **Replaceable components:** Providers and execution environments should be replaceable where the architecture permits.
9. **Controlled autonomy:** Automated actions should remain within established authority and policy boundaries.
10. **Federation without authority leakage:** Participation in the federation should not silently expand a node's or workspace's privileges.

## Development Status and Verification

Sovereign-OS is a long-term systems engineering project with an evolving implementation.

The architecture described in this README represents the project's system design and intended capabilities. It should not be interpreted as a claim that every listed subsystem, workspace, integration, or distributed capability is already fully implemented or production-ready.

Implementation status should be established from the source tree, milestone records, tests, and current verification evidence.

Development emphasizes:

- Incremental runtime milestones.
- Explicit architectural contracts.
- Independently testable components.
- Persistent and recoverable state.
- Regression verification.
- Capability traceability.
- Migration correctness.
- Integration stability.
- Clear subsystem responsibilities.

When documenting an individual capability, distinguish among:

- **Designed:** Defined in the architecture or engineering plan.
- **Implemented:** Present in the codebase.
- **Integrated:** Connected to its required system components.
- **Verified:** Supported by current tests or other reproducible evidence.
- **Production-ready:** Demonstrated to meet the project's operational requirements.

This distinction keeps the repository's claims aligned with the actual implementation.

## Long-Term Direction

The long-term direction is a persistent, sovereign computational environment capable of coordinating intelligence, knowledge, infrastructure, applications, automation, and distributed resources through one governed architecture.

Its workspaces provide specialization.

Its subsystems provide capabilities.

Its federation enables distributed participation.

Its runtime provides continuity.

Its governance defines authority.

Its NixOS foundation provides reproducible infrastructure.

**Sovereign-OS aims to make intelligence and computation federated, persistent, extensible, and governed—without surrendering sovereign control to any single model, machine, or provider.**

---

## Project Identity

| Attribute | Description |
|---|---|
| **Project** | Sovereign-OS |
| **Architecture** | Modular, federated, governed cognitive operating system |
| **Infrastructure foundation** | NixOS |
| **Control model** | Sovereign control plane with federated execution environments |
| **Core domains** | Identity, governance, policy, memory, planning, scheduling, orchestration, audit, and federation |
| **Workspace architecture** | Sixteen specialized operational domains |
| **Development approach** | Incremental implementation, explicit contracts, persistent state, and verifiable runtime behavior |

**Sovereign intelligence. Federated capabilities. Unified coordination. Governed execution.**

---

## Implementation Maturity Matrix (`main` @ `L36` — `69/69` CTest Verified — Bare-Metal KDE Plasma 6 + Two-Way Voice Verified)

The project includes verified subsystems across infrastructure, identity, governance, capabilities, memory, planning, workspaces, model routing, audit logs, federation, and mission control. For the complete table detailing subsystem maturity levels and verification evidence, refer to the repository's `README.md`.

## Quickstart & Release Artifacts

### Build & Verify from Source
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j\$(nproc)
ctest --test-dir build --output-on-failure
```

### Start Host Control Plane & Mission Control UI
```bash
./scripts/sovereign-session-ensure.sh
./build/apps/cli/sovereign-cli doctor --fix
./build/apps/cli/sovereign-cli workspace-init-domains
# Open http://localhost:8769/ui
```

### Bootable NixOS ISO & Bare-Metal / SD / USB Deployment
- Download `sovereign-os-2.0-plasma6-x86_64.iso` and `SHA256SUMS` from [GitHub Releases (`v2.0.0`)](https://github.com), or build locally with `nix build .#iso`.
- Write safely to a removable USB drive or SD card using the provided flashing script.
  ```bash
  ./scripts/sovereign-flash-usb.sh --list
  ./scripts/sovereign-flash-usb.sh --yes-flash /dev/sdX   # or /dev/mmcblk0
  ```
