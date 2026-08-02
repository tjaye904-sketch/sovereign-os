"""Command-line interface for managed Sovereign Mesh workflows."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

from sovereign_mesh.executor import GovernedExecutor
from sovereign_mesh.workflow import ManagedWorkflow, WorkflowError


def emit(payload: Any) -> None:
    print(json.dumps(payload, indent=2, sort_keys=True))


def parser() -> argparse.ArgumentParser:
    root = argparse.ArgumentParser(
        prog="workflowctl",
        description="Managed Sovereign Mesh task-worktree controller.",
    )
    root.add_argument(
        "--repository",
        default=".",
        help="Primary Sovereign repository checkout.",
    )

    commands = root.add_subparsers(dest="command", required=True)

    create = commands.add_parser("create", help="Create a managed task worktree.")
    create.add_argument("--task-id", required=True)
    create.add_argument("--title", required=True)
    create.add_argument("--prompt", required=True)
    create.add_argument("--adapter", required=True)
    create.add_argument("--base-ref", default="HEAD")

    inspect = commands.add_parser("inspect", help="Inspect a managed task.")
    inspect.add_argument("--task-id", required=True)

    execute = commands.add_parser(
        "execute",
        help="Execute explicit argv inside a managed task worktree.",
    )
    execute.add_argument("--task-id", required=True)
    execute.add_argument("--timeout", type=int, default=900)
    execute.add_argument("--cwd-relative", default=".")
    execute.add_argument(
        "argv",
        nargs=argparse.REMAINDER,
        help="Command argv, normally supplied after --.",
    )

    evidence = commands.add_parser(
        "record-evidence",
        help="Capture an agent or operator evidence file.",
    )
    evidence.add_argument("--task-id", required=True)
    evidence.add_argument("--source-file", required=True)
    evidence.add_argument("--producer", required=True)
    evidence.add_argument("--command-preview", required=True)

    proposal = commands.add_parser(
        "assemble-proposal",
        help="Assemble a proposal from validated evidence.",
    )
    proposal.add_argument("--task-id", required=True)
    proposal.add_argument("--summary", required=True)
    proposal.add_argument("--recommendation", required=True)

    verify = commands.add_parser(
        "verify-integration",
        help="Verify whether human approval authorizes integration.",
    )
    verify.add_argument("--task-id", required=True)

    remove = commands.add_parser(
        "remove-worktree",
        help="Remove a clean managed worktree.",
    )
    remove.add_argument("--task-id", required=True)
    remove.add_argument("--delete-branch", action="store_true")

    return root


def main() -> int:
    arguments = parser().parse_args()

    try:
        workflow = ManagedWorkflow(Path(arguments.repository))

        if arguments.command == "create":
            emit(
                workflow.create_task(
                    task_id=arguments.task_id,
                    title=arguments.title,
                    prompt=arguments.prompt,
                    adapter=arguments.adapter,
                    base_ref=arguments.base_ref,
                )
            )
        elif arguments.command == "inspect":
            emit(workflow.inspect_task(arguments.task_id))
        elif arguments.command == "execute":
            argv = list(arguments.argv)
            if argv and argv[0] == "--":
                argv = argv[1:]

            result = GovernedExecutor(workflow).execute(
                task_id=arguments.task_id,
                argv=argv,
                timeout_seconds=arguments.timeout,
                cwd_relative=arguments.cwd_relative,
            )
            emit(result)
            return 0 if result["record"]["succeeded"] else 4
        elif arguments.command == "record-evidence":
            emit(
                workflow.record_evidence(
                    task_id=arguments.task_id,
                    source_file=Path(arguments.source_file),
                    producer=arguments.producer,
                    command_preview=arguments.command_preview,
                )
            )
        elif arguments.command == "assemble-proposal":
            emit(
                workflow.assemble_proposal(
                    task_id=arguments.task_id,
                    summary=arguments.summary,
                    recommendation=arguments.recommendation,
                )
            )
        elif arguments.command == "verify-integration":
            result = workflow.verify_integration_authority(arguments.task_id)
            emit(result)
            return 0 if result["authorized"] else 3
        elif arguments.command == "remove-worktree":
            emit(
                workflow.remove_task_worktree(
                    task_id=arguments.task_id,
                    delete_branch=arguments.delete_branch,
                )
            )
        else:
            raise WorkflowError(f"Unsupported command: {arguments.command}")

        return 0
    except WorkflowError as error:
        print(f"workflowctl: error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
