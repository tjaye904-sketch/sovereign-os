#!/usr/bin/env python3
"""
Orchestration agent: creates an isolated git worktree + branch for each
development session, tracks its status, and merges to main only on
explicit approval (and only if the quality gate passed).

The isolation is structural: the coding agent is pointed at the worktree
directory and never given a path back to the main working tree.

Usage:
    python3 orchestrator.py start   --task "add policy engine validation"
    python3 orchestrator.py gate    --session <session_id>
    python3 orchestrator.py approve --session <session_id>
    python3 orchestrator.py abort   --session <session_id>
    python3 orchestrator.py list
"""

import argparse
import json
import subprocess
import sys
import time
from pathlib import Path

REPO_ROOT = Path.cwd()
SESSIONS_FILE = REPO_ROOT / ".sessions.json"
WORKTREES_DIR = REPO_ROOT.parent / "worktrees"


def run(cmd, cwd=None, check=True):
    print(f"$ {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if result.stdout:
        print(result.stdout)
    if result.returncode != 0:
        print(result.stderr, file=sys.stderr)
        if check:
            sys.exit(result.returncode)
    return result


def load_sessions():
    if SESSIONS_FILE.exists():
        return json.loads(SESSIONS_FILE.read_text())
    return {}


def save_sessions(sessions):
    SESSIONS_FILE.write_text(json.dumps(sessions, indent=2))


def start_session(task: str):
    WORKTREES_DIR.mkdir(exist_ok=True)
    session_id = f"{int(time.time())}"
    branch = f"session/{session_id}"
    worktree_path = WORKTREES_DIR / session_id

    run(["git", "worktree", "add", "-b", branch, str(worktree_path), "main"])

    sessions = load_sessions()
    sessions[session_id] = {
        "task": task,
        "branch": branch,
        "worktree": str(worktree_path),
        "status": "open",
        "last_gate": None,
    }
    save_sessions(sessions)

    print(f"\nSession {session_id} ready.")
    print(f"Branch:   {branch}")
    print(f"Worktree: {worktree_path}")
    print(f"Point your coding agent's working directory at exactly that "
          f"path and nowhere else.")


def run_gate(session_id: str):
    sessions = load_sessions()
    session = sessions.get(session_id)
    if not session:
        sys.exit(f"No such session: {session_id}")

    result = subprocess.run(
        [sys.executable, str(REPO_ROOT / "quality_gate.py"),
         "--worktree", session["worktree"],
         "--repo", str(REPO_ROOT)],
    )
    passed = result.returncode == 0
    session["last_gate"] = "PASS" if passed else "FAIL"
    save_sessions(sessions)
    print(f"\nGate result for session {session_id}: {session['last_gate']}")


def approve(session_id: str, force: bool = False):
    sessions = load_sessions()
    session = sessions.get(session_id)
    if not session:
        sys.exit(f"No such session: {session_id}")

    if session.get("last_gate") != "PASS" and not force:
        sys.exit(
            "Refusing to merge: quality gate has not passed for this "
            "session. Run `gate` first, or pass --force to override "
            "deliberately."
        )

    run(["git", "merge", "--no-ff", session["branch"],
         "-m", f"Merge session {session_id}: {session['task']}"])
    run(["git", "worktree", "remove", session["worktree"]])
    run(["git", "branch", "-d", session["branch"]])

    session["status"] = "merged"
    save_sessions(sessions)
    print(f"Session {session_id} merged and cleaned up.")


def abort(session_id: str):
    sessions = load_sessions()
    session = sessions.get(session_id)
    if not session:
        sys.exit(f"No such session: {session_id}")

    run(["git", "worktree", "remove", "--force", session["worktree"]], check=False)
    run(["git", "branch", "-D", session["branch"]], check=False)

    session["status"] = "aborted"
    save_sessions(sessions)
    print(f"Session {session_id} aborted, no changes merged.")


def list_sessions():
    sessions = load_sessions()
    if not sessions:
        print("No sessions recorded.")
        return
    for sid, s in sessions.items():
        print(f"{sid}  [{s['status']:8}]  gate={s['last_gate']}  "
              f"branch={s['branch']}  task={s['task']}")


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd", required=True)

    p_start = sub.add_parser("start")
    p_start.add_argument("--task", required=True)

    p_gate = sub.add_parser("gate")
    p_gate.add_argument("--session", required=True)

    p_approve = sub.add_parser("approve")
    p_approve.add_argument("--session", required=True)
    p_approve.add_argument("--force", action="store_true")

    p_abort = sub.add_parser("abort")
    p_abort.add_argument("--session", required=True)

    sub.add_parser("list")

    args = parser.parse_args()

    if args.cmd == "start":
        start_session(args.task)
    elif args.cmd == "gate":
        run_gate(args.session)
    elif args.cmd == "approve":
        approve(args.session, args.force)
    elif args.cmd == "abort":
        abort(args.session)
    elif args.cmd == "list":
        list_sessions()


if __name__ == "__main__":
    main()
