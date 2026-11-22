"""L4 — query Sovereign capability gate from Python (CU/sys)."""
from __future__ import annotations

import json
import os
import shutil
import subprocess
from pathlib import Path
from typing import Any


def data_root() -> Path:
    return Path(
        os.environ.get(
            "SOVEREIGN_DATA_ROOT",
            str(Path.home() / ".local/share/sovereign"),
        )
    )


def _cli() -> str:
    env = os.environ.get("SOVEREIGN_CLI")
    if env and Path(env).is_file():
        return env
    for c in (
        str(Path.home() / "src/sovereign-os/sovereign-work/build/apps/cli/sovereign-cli"),
        str(Path.home() / ".local/bin/sovereign-cli"),
        shutil.which("sovereign-cli") or "",
    ):
        if c and Path(c).is_file():
            return c
    return "sovereign-cli"


def gate_check(action_or_id: str) -> dict[str, Any]:
    """
    Returns {allowed: bool, capability_id: str, reason: str, raw: str}.
    Fail closed if CLI missing or non-zero without parseable output.
    """
    cli = _cli()
    env = os.environ.copy()
    env.setdefault("SOVEREIGN_DATA_ROOT", str(data_root()))
    try:
        p = subprocess.run(
            [cli, "capability-gate-check", action_or_id],
            capture_output=True,
            text=True,
            timeout=30,
            env=env,
        )
    except FileNotFoundError:
        return {
            "allowed": False,
            "capability_id": action_or_id,
            "reason": "sovereign-cli not found",
            "raw": "",
        }
    except subprocess.TimeoutExpired:
        return {
            "allowed": False,
            "capability_id": action_or_id,
            "reason": "capability-gate-check timeout",
            "raw": "",
        }
    raw = (p.stdout or "") + (p.stderr or "")
    allowed = False
    cap_id = action_or_id
    reason = "unknown"
    for line in raw.splitlines():
        if line.startswith("capability_id="):
            # capability_id=host.execute allowed=1 reason=allowed
            parts = dict(
                kv.split("=", 1)
                for kv in line.split()
                if "=" in kv
            )
            cap_id = parts.get("capability_id", cap_id)
            allowed = parts.get("allowed", "0") == "1"
            reason = parts.get("reason", reason)
            break
    if "allowed=1" in raw:
        allowed = True
    if "allowed=0" in raw:
        allowed = False
    if p.returncode == 0 and "allowed=" not in raw:
        # old CLI without gate-check
        reason = "gate-check unsupported; fail closed"
        allowed = False
    return {
        "allowed": allowed,
        "capability_id": cap_id,
        "reason": reason,
        "raw": raw.strip(),
        "exit_code": p.returncode,
    }


def require_action(action: str) -> dict[str, Any]:
    """Raise-style result for CU dispatch: ok False + capability_denied."""
    r = gate_check(action)
    if r["allowed"]:
        return {"ok": True, **r}
    return {
        "ok": False,
        "error": f"capability_denied id={r['capability_id']} reason={r['reason']}",
        "denied": True,
        "capability_id": r["capability_id"],
        "reason": r["reason"],
    }


# Action map aligned with C++ CapabilityGate
ACTION_MAP = {
    "browser_open": "browser_open",
    "browser_snapshot": "browser_open",
    "browser_screenshot": "browser_open",
    "purpose_run": "purpose-run",
    "purpose_list": "purpose-run",
    "app_open": "app_open",
    "desktop_list_windows": "app_open",
    "desktop_focus": "app_open",
    "desktop_screenshot": "app_open",
}


def require_for_cu_action(action: str) -> dict[str, Any]:
    key = ACTION_MAP.get(action, action)
    return require_action(key)
