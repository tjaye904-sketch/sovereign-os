# Identity Service — Implementation Specification

**Document ID:** SPEC-IDENTITY-001  
**Status:** Draft for implementation (open questions resolved)  
**Date:** 2026-07-29  
**Decisions locked:** 2026-07-29  
**Milestone:** 2.1 — Identity Service  
**Depends on:** Runtime Platform v1 (complete)  
**Blocks:** Governance Service, Policy Engine, session-scoped product work

---

## 1. Purpose

The Identity Service establishes **who** is interacting with Sovereign and
provides the durable identity substrate required by governance, policy, memory,
and workspaces.

It answers:

- Who is the principal making a request?
- What identity attributes and roles are bound to that principal?
- Is the session valid?
- What audit record must be written for identity-related events?

It does **not** decide whether an action is allowed. Authorization and approval
belong to Governance and Policy. Authentication establishes identity;
authorization consumes it.

---

## 2. Non-Goals (v1)

The first Identity Service milestone deliberately excludes:

- Full multi-factor authentication hardware integration
- Biometric capture pipelines
- Federated external IdP protocols (OIDC/SAML) as primary path
- Multi-tenant organizational directories
- End-user profile UI
- Cryptographic key hierarchy / hardware-backed credentials (deferred to
  Security architecture packages)
- Replacing runtime `identity_types.hpp` primitives wholesale

Those may be added in later Identity revisions after the core lifecycle,
session, and audit contracts are stable.

---

## 3. Relationship to Runtime

Runtime already provides typed identifiers (`AgentId`, `NodeId`, `RuntimeId`,
`WorkspaceId`, `SubscriptionId`, etc.) in `identity_types.hpp`. Those remain
**runtime infrastructure IDs**.

The Identity Service introduces **product principals**:

| Concept | Layer | Role |
|---------|-------|------|
| `AgentId`, `NodeId`, … | Runtime | Infrastructure correlation |
| `PrincipalId` | Identity Service | Product identity of a person, service, or device principal |
| `SessionId` | Identity Service | Authenticated interaction context |
| `RoleId` / role names | Identity Service | Coarse authorization attributes (consumed by Policy later) |

Runtime IDs may appear in Identity audit records for correlation. Identity does
not own runtime subscription or event routing.

Reusable runtime facilities the Identity Service **may** use:

- Service host / service registry registration
- Event bus for identity lifecycle events
- Configuration manager for local identity configuration
- Logging / diagnostics sinks
- Capability invocation only if Identity is exposed as a runtime capability
  (optional in v1; direct service API is sufficient)

---

## 4. Core Concepts

### 4.1 Principal

A principal is an entity that can act in Sovereign.

**Kinds (v1):**

- `Owner` — the human owner of the Sovereign installation
- `Operator` — additional human users explicitly enrolled by the owner
- `Service` — reserved in the data model only (Decision IQ-4B); no public
  create/authenticate API in v1
- `Device` — enrolled local device identity (minimal / deferred if unused)

**Operator enrollment (Decision IQ-2A):** Only an authenticated Owner session
may create Operator principals in v1. One-time enrollment tokens are deferred.

Each principal has:

- `PrincipalId` (UUID)
- Kind
- Display name
- Status: `Active`, `Suspended`, `Revoked`
- Creation and update timestamps
- Optional metadata map (string keys/values; no free-form secrets)

### 4.2 Credential (v1 minimal)

v1 supports a local credential binding sufficient for owner bootstrap and
session establishment:

- Credential type: `LocalSecret` (password/passphrase hash) or `LocalToken`
  (opaque session-capable token material)
- Credentials are stored only as irreversible verifiers or sealed local secrets
- Raw secrets never appear in logs or events

Exact KDF and storage format are implementation details but must be documented
in the security notes for the milestone.

### 4.3 Session

A session binds a principal to an interaction period.

Fields:

- `SessionId` (UUID)
- `PrincipalId`
- Status: `Active`, `Expired`, `Revoked`
- Issued at, last activity, expires at
- Origin attributes (optional): node, client label
- Authentication method used

Sessions are local to the installation in v1 (no cross-device federation yet).

**Resolved policy (2026-07-29):**

- Sessions **persist across process restart** until natural expiry or explicit
  revoke (Decision IQ-1A).
- Default lifetime is **12 hours**, configurable via Identity configuration
  (Decision IQ-3D). Sliding refresh on activity is deferred to v1.1 unless
  trivial to add during implementation.
- Changing a principal's credential **revokes all of that principal's
  active sessions**.

### 4.4 Role

Roles are coarse labels attached to principals for later policy consumption.

v1 built-in roles:

- `owner`
- `operator`
- `readonly`
- `service`

Custom roles are out of scope for v1.

---

## 5. Responsibilities

The Identity Service shall:

1. Create, read, update, and suspend/revoke principals (within policy limits).
2. Bootstrap the single **Owner** principal on first run if none exists.
3. Establish and validate sessions.
4. Revoke sessions (logout, admin revoke, expiry).
5. Attach and query roles for a principal.
6. Emit identity lifecycle events on the runtime event infrastructure.
7. Persist principal, credential verifier, session, and role bindings durably.
8. Produce audit records for identity-significant operations.
9. Expose a clear service API usable by Governance and future services.

The Identity Service shall **not**:

- Grant or deny domain actions (deploy, transfer funds, delete data, etc.)
- Execute governance approvals
- Own workspace or memory content
- Call external identity providers as a required dependency in v1

---

## 6. Public Interface (logical)

Names are logical; C++ concrete types follow project conventions.

```text
IdentityService
  BootstrapOwner(display_name, credential) -> Result<PrincipalId>
  CreatePrincipal(spec) -> Result<PrincipalId>
  GetPrincipal(PrincipalId) -> Result<Principal>
  ListPrincipals(filter) -> Result<PrincipalList>
  SetPrincipalStatus(PrincipalId, status) -> Result<void>
  AssignRole(PrincipalId, role) -> Result<void>
  RevokeRole(PrincipalId, role) -> Result<void>
  GetRoles(PrincipalId) -> Result<RoleSet>

  Authenticate(principal_ref, credential) -> Result<Session>
  ValidateSession(SessionId) -> Result<Session>
  RefreshSession(SessionId) -> Result<Session>
  RevokeSession(SessionId) -> Result<void>
  ListSessions(PrincipalId) -> Result<SessionList>
```

All mutating operations return structured errors (not silent bool failures).

Suggested error categories:

- `NotFound`
- `AlreadyExists`
- `InvalidCredential`
- `SessionExpired`
- `SessionRevoked`
- `PrincipalSuspended`
- `PrincipalRevoked`
- `NotBootstrapped`
- `AlreadyBootstrapped`
- `InvalidArgument`
- `StorageFailure`

---

## 7. Events

Identity publishes events such as:

| Event topic (logical) | When |
|-----------------------|------|
| `identity.owner.bootstrapped` | First owner created |
| `identity.principal.created` | Principal enrolled |
| `identity.principal.status_changed` | Suspend / revoke / reactivate |
| `identity.role.assigned` | Role attached |
| `identity.role.revoked` | Role removed |
| `identity.session.established` | Successful authentication |
| `identity.session.revoked` | Logout or forced revoke |
| `identity.session.expired` | Expiry observed |

Event payloads include principal/session IDs, timestamps, and correlation IDs.
They must not include raw credentials or secret material.

---

## 8. Persistence Requirements

v1 requires durable local storage for:

- Principals
- Credential verifiers
- Sessions
- Role bindings
- Identity audit entries (or a clear handoff to a shared audit store if one
  exists by implementation time)

Storage technology is an implementation choice (e.g. SQLite) but must:

- Survive process restart
- Support transactional updates for authenticate + session create
- Avoid storing reversible passwords

Migration strategy for schema changes must be documented with the first
implementation commit that introduces the schema.

**Storage location (Decision IQ-5D):**

- Prefer `$SOVEREIGN_DATA_ROOT/identity/` when `SOVEREIGN_DATA_ROOT` is set.
- Otherwise default to the XDG data path:
  `~/.local/share/sovereign/identity/` (Linux).
- Never store credential verifiers under the source repository tree.

---

## 9. Security Notes (v1)

- Single-installation trust boundary; no multi-tenant isolation claim in v1
- Owner bootstrap occurs only when no owner exists
- Suspended or revoked principals cannot establish new sessions
- Session validation is mandatory for product APIs that declare “authenticated”
- Identity audit log is append-oriented for significant events
- Threats in scope for v1 acceptance: local credential guessing rate limits
  (basic), session fixation via revoke-on-credential-change, no secret leakage
  in logs/events

Full threat model document is a required milestone output (can be a section of
this file or `docs/project/IDENTITY_THREAT_MODEL.md`).

---

## 10. Acceptance Criteria

Identity Service v1 is **not complete** until all of the following hold:

### Functional

1. Fresh installation can bootstrap exactly one Owner.
2. Owner can authenticate and receive an Active session.
3. Session validates successfully until expiry or revoke.
4. Revoked or expired sessions fail validation.
5. Additional Operator principal can be created by an authorized path
   (owner session) and authenticated.
6. Roles can be assigned and queried.
7. Suspended principal cannot authenticate.
8. Identity events are observable on the event path used by the runtime.
9. Process restart preserves principals and valid unexpired sessions
   (sessions persist until expiry or revoke — IQ-1A).

### Quality

10. Automated unit tests cover bootstrap, authenticate success/failure,
    session validate/revoke/expiry, role assign/query, and status changes.
11. At least one integration test exercises service registration + bootstrap +
    authenticate against a real (test) storage backend.
12. No raw secrets appear in logs or event payloads under test.

### Documentation and process (GD-001)

13. This specification remains consistent with implementation.
14. Public interfaces documented.
15. Threat model notes recorded.
16. `PROJECT_STATUS.md`, `IMPLEMENTATION_MATRIX.md`, `IMPLEMENTATION_ROADMAP.md`,
    and `ENGINEERING_JOURNAL.md` updated.
17. Runtime Platform boundary respected — no drive-by rewrite of completed
    runtime modules unrelated to Identity integration.

---

## 11. Implementation Plan (suggested order)

1. Data model + storage schema
2. Principal CRUD + owner bootstrap
3. Credential verifier + authenticate
4. Session lifecycle
5. Roles
6. Event emission
7. Service registration with runtime service host
8. Tests and threat notes
9. Project-state document updates

Each step should remain reviewable; avoid a single monolithic commit that
mixes storage, API, and unrelated runtime changes.

---

## 12. Resolved Design Decisions (formerly open questions)

Locked 2026-07-29 by owner acceptance of engineering recommendations.

| ID | Topic | Decision |
|----|-------|----------|
| IQ-1A | Session survival across restart | Persist sessions until expiry or explicit revoke. |
| IQ-2A | Operator enrollment | Owner-session only; no enrollment tokens in v1. |
| IQ-3D | Default session lifetime | Configurable; **default 12 hours**. Sliding refresh deferred. |
| IQ-4B | Service principals | Kind reserved in schema only; no public service auth API in v1. |
| IQ-5D | Storage location | `$SOVEREIGN_DATA_ROOT/identity/` if set; else `~/.local/share/sovereign/identity/`. Never repo-relative. |

Additional binding rules from the same decisions:

- Credential change for a principal revokes all of that principal's active sessions.
- Exactly one Owner may be bootstrapped; further Owner creation is rejected.
- Suspended or revoked principals cannot establish sessions.

---

## 13. Definition of Done Pointer

Completion is governed by GD-001 Definition of Done plus Section 10 of this
specification. No status document may list Identity Service as Implemented or
Complete until those criteria are met.

---

## Related Documents

- `VISION.md`
- `SOVEREIGN_BOOTSTRAP.md`
- `IMPLEMENTATION_ROADMAP.md` (Milestone 2.1)
- `IMPLEMENTATION_MATRIX.md`
- `governance/GD-001-Engineering-Governance.md`
- `AUDITS/Audit-001-Runtime-v1.md`
- Runtime: `core/runtime/identity_types.hpp`, service host, event bus
