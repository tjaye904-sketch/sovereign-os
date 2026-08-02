"""Command-line interface for the Sovereign Development Mesh."""

from __future__ import annotations

import argparse
import json
import shlex
import sys
from dataclasses import asdict
from pathlib import Path
from typing import Any

from .adapters import CodexAdapter, HermesAdapter, OpenClawAdapter
from .decisions import record_decision, require_approved_decision
from .errors import MeshError
from .policy import BuildAuthorityPolicy
from .repository import inspect_repository, require_isolated_worktree


PACKAGE_ROOT = Path(__file__).resolve().parent
DEFAULT_POLICY = PACKAGE_ROOT / "policy" / "build-authority.json"


def print_json(value: Any) -> None:
    print(json.dumps(value, indent=2, sort_keys=True, default=str))


def load_policy(path: str | None) -> BuildAuthorityPolicy:
    return BuildAuthorityPolicy.load(
        Path(path).resolve() if path else DEFAULT_POLICY
    )


def command_status(args: argparse.Namespace) -> int:
    policy = load_policy(args.policy)
    repository = inspect_repository(Path(args.repository))

    adapters = [
        CodexAdapter(policy),
        HermesAdapter(policy),
        OpenClawAdapter(policy),
    ]

    adapter_status: dict[str, Any] = {}
    for adapter in adapters:
        try:
            adapter_status[adapter.name] = {
                "available": True,
                "executable": adapter.executable_path(),
                "version": adapter.version(),
            }
        except MeshError as error:
            adapter_status[adapter.name] = {
                "available": False,
                "error": str(error),
            }

    print_json(
        {
            "mesh_version": "0.1.0",
            "mode": "preview",
            "policy": policy.summary(),
            "repository": asdict(repository),
            "adapters": adapter_status,
        }
    )
    return 0


def command_repository(args: argparse.Namespace) -> int:
    identity = inspect_repository(Path(args.repository))
    print_json(asdict(identity))
    return 0


def command_require_worktree(args: argparse.Namespace) -> int:
    identity = require_isolated_worktree(Path(args.repository))
    print_json(asdict(identity))
    return 0


def command_preview(args: argparse.Namespace) -> int:
    policy = load_policy(args.policy)
    worktree = Path(args.worktree).resolve()

    if args.adapter == "codex":
        adapter = CodexAdapter(policy)
        if args.operation == "inspect":
            preview = adapter.inspect(worktree, args.prompt)
        elif args.operation == "implement":
            preview = adapter.implement(worktree, args.prompt)
        elif args.operation == "review":
            preview = adapter.review(
                worktree,
                base_branch=args.base_branch,
                prompt=args.prompt,
            )
        else:
            raise ValueError(
                f"Unsupported Codex operation: {args.operation}"
            )

    elif args.adapter == "hermes":
        adapter = HermesAdapter(policy)
        if args.operation == "inspect":
            preview = adapter.inspect(worktree, args.prompt)
        elif args.operation == "implement":
            preview = adapter.implement(worktree, args.prompt)
        else:
            raise ValueError(
                f"Unsupported Hermes operation: {args.operation}"
            )

    elif args.adapter == "openclaw":
        adapter = OpenClawAdapter(policy)
        if args.operation == "audit":
            preview = adapter.audit(
                worktree,
                agent_id=args.agent_id,
                limit=args.limit,
            )
        else:
            raise ValueError(
                f"Unsupported OpenClaw preview operation: {args.operation}"
            )
    else:
        raise ValueError(f"Unsupported adapter: {args.adapter}")

    output = asdict(preview)
    output["shell_preview"] = shlex.join(preview.argv)
    print_json(output)
    return 0


def command_record_decision(args: argparse.Namespace) -> int:
    output = record_decision(
        proposal_path=Path(args.proposal).resolve(),
        decision_dir=Path(args.decision_dir).resolve(),
        decision=args.decision,
        notes=args.notes,
    )
    print_json({"decision_record": str(output)})
    return 0


def command_verify_decision(args: argparse.Namespace) -> int:
    decision = require_approved_decision(
        proposal_path=Path(args.proposal).resolve(),
        decision_path=Path(args.decision).resolve(),
    )
    print_json({"approved": True, "decision": decision})
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="meshctl",
        description=(
            "Governed orchestration interface for Sovereign development agents."
        ),
    )
    parser.add_argument(
        "--policy",
        help="Path to an alternate build authority policy.",
    )

    subparsers = parser.add_subparsers(dest="command", required=True)

    status = subparsers.add_parser(
        "status",
        help="Inspect mesh, repository, policy, and adapter availability.",
    )
    status.add_argument("--repository", default=".")
    status.set_defaults(handler=command_status)

    repository = subparsers.add_parser(
        "repository",
        help="Inspect Git checkout identity.",
    )
    repository.add_argument("repository", nargs="?", default=".")
    repository.set_defaults(handler=command_repository)

    worktree = subparsers.add_parser(
        "require-worktree",
        help="Fail unless the supplied checkout is an isolated Git worktree.",
    )
    worktree.add_argument("repository", nargs="?", default=".")
    worktree.set_defaults(handler=command_require_worktree)

    preview = subparsers.add_parser(
        "preview",
        help="Construct but do not execute a governed adapter command.",
    )
    preview.add_argument(
        "adapter",
        choices=["codex", "hermes", "openclaw"],
    )
    preview.add_argument("operation")
    preview.add_argument("--worktree", required=True)
    preview.add_argument("--prompt", default="")
    preview.add_argument("--base-branch", default="master")
    preview.add_argument("--agent-id", default="main")
    preview.add_argument("--limit", type=int, default=100)
    preview.set_defaults(handler=command_preview)

    decision = subparsers.add_parser(
        "record-decision",
        help="Record the owner's immutable decision for a proposal.",
    )
    decision.add_argument("--proposal", required=True)
    decision.add_argument(
        "--decision",
        required=True,
        choices=["approved", "rejected", "revision_requested"],
    )
    decision.add_argument("--notes", default="")
    decision.add_argument(
        "--decision-dir",
        default=".sovereign-mesh/decisions",
    )
    decision.set_defaults(handler=command_record_decision)

    verify = subparsers.add_parser(
        "verify-decision",
        help="Verify that an unchanged proposal has an approved owner decision.",
    )
    verify.add_argument("--proposal", required=True)
    verify.add_argument("--decision", required=True)
    verify.set_defaults(handler=command_verify_decision)

    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    try:
        return int(args.handler(args))
    except (MeshError, ValueError, FileExistsError) as error:
        print(f"meshctl: error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
