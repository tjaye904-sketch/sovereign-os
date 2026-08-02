# Sovereign Mesh

The Sovereign Mesh is a policy-controlled orchestration layer for external
agents and development tools.

## Governance invariants

- The primary Sovereign checkout is never an agent mutation target.
- Mutating activity requires an isolated Git worktree.
- Task manifests and evidence are bound by SHA-256 digests.
- Proposal approval is invalidated when its bound proposal changes.
- Human approval is required before integration.
- SM-004 emits an integration command preview but does not execute a merge.

## Controllers

### meshctl

Provides adapter discovery, policy inspection, command preview, and decision
operations.

### workflowctl

Provides managed worktree and governed task-packet operations.

## Example task preparation

Command:

    tools/sovereign_mesh/workflowctl create \
      --task-id SM-004-DEMO \
      --title "Read-only architecture inspection" \
      --adapter codex \
      --prompt "Inspect the assigned worktree and report findings."

Managed worktrees are created under:

    .sovereign-mesh/worktrees/<task-id>

Local task manifests, evidence, proposals, decisions, and worktrees are
deliberately excluded from Git.
