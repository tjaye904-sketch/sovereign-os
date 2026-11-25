"""L5 — capability list/toggle helpers for Mission Control hub."""
from __future__ import annotations

import json
import os
import re
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


def cli_path() -> str:
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


def _run(args: list[str]) -> tuple[int, str]:
    env = os.environ.copy()
    env.setdefault("SOVEREIGN_DATA_ROOT", str(data_root()))
    try:
        p = subprocess.run(
            [cli_path(), *args],
            capture_output=True,
            text=True,
            timeout=60,
            env=env,
        )
    except FileNotFoundError:
        return 127, "sovereign-cli not found"
    except subprocess.TimeoutExpired:
        return 124, "timeout"
    return p.returncode, ((p.stdout or "") + (p.stderr or "")).strip()


def list_capabilities() -> dict[str, Any]:
    _run(["capability-seed"])
    code, out = _run(["capability-list"])
    items: list[dict[str, Any]] = []
    # Formats: id=foo enabled=1 source=seed desc=...
    for line in out.splitlines():
        if "id=" not in line:
            continue
        parts: dict[str, str] = {}
        for tok in line.split():
            if "=" in tok:
                k, v = tok.split("=", 1)
                parts[k] = v
        if "id" not in parts:
            continue
        items.append(
            {
                "id": parts.get("id", ""),
                "enabled": parts.get("enabled", "0") in ("1", "true", "True"),
                "source": parts.get("source", ""),
                "description": parts.get("desc", parts.get("description", "")),
            }
        )
    return {
        "ok": code == 0,
        "capabilities": items,
        "raw": out if code != 0 else "",
        "error": out if code != 0 and not items else None,
    }


def set_capability(cap_id: str, enabled: bool) -> dict[str, Any]:
    if not re.match(r"^[a-zA-Z0-9._-]+$", cap_id):
        return {"ok": False, "error": "invalid capability id"}
    cmd = "capability-enable" if enabled else "capability-disable"
    code, out = _run([cmd, cap_id])
    # verify
    check = list_capabilities()
    row = next((c for c in check.get("capabilities", []) if c["id"] == cap_id), None)
    return {
        "ok": code == 0,
        "id": cap_id,
        "enabled": row["enabled"] if row else enabled,
        "detail": out,
        "error": None if code == 0 else out,
    }


def gate_check(action_or_id: str) -> dict[str, Any]:
    code, out = _run(["capability-gate-check", action_or_id])
    allowed = "allowed=1" in out
    cap_id = action_or_id
    reason = ""
    for line in out.splitlines():
        if "capability_id=" in line:
            for tok in line.split():
                if tok.startswith("capability_id="):
                    cap_id = tok.split("=", 1)[1]
                if tok.startswith("reason="):
                    reason = tok.split("=", 1)[1]
            break
    return {
        "ok": True,
        "allowed": allowed,
        "capability_id": cap_id,
        "reason": reason,
        "exit_code": code,
        "raw": out,
    }


def desktop_status() -> dict[str, Any]:
    code, out = _run(["desktop-status", "--json"])
    if code != 0:
        return {"ok": False, "error": out or "desktop-status failed"}
    try:
        data = json.loads(out.splitlines()[-1])
        data["ok"] = True
        return data
    except Exception as exc:
        return {"ok": False, "error": f"invalid json: {exc}", "raw": out}


def federation_status() -> dict[str, Any]:
    code, out = _run(["node-list"])
    nodes = [line for line in out.splitlines() if line.strip()]
    return {"ok": code == 0, "nodes": nodes, "count": len(nodes)}


def list_pending_decisions() -> dict[str, Any]:
    code, out = _run(["pending-decisions"])
    items = []
    for line in out.splitlines():
        line = line.strip()
        if not line or line.startswith("pending="):
            continue
        parts = {}
        for tok in line.split():
            if "=" in tok:
                k, v = tok.split("=", 1)
                parts[k] = v
        items.append({"raw": line, **parts})
    return {"ok": code == 0, "pending": items, "count": len(items), "raw": out}


def decide_governance(decision_hex: str, approve: bool, reason: str = "mission-control") -> dict[str, Any]:
    if not re.match(r"^[0-9a-fA-F]{32}$", decision_hex):
        return {"ok": False, "error": "invalid decision id"}
    cmd = "approve" if approve else "reject"
    code, out = _run([cmd, decision_hex, reason or "mission-control"])
    return {"ok": code == 0, "decision_id": decision_hex, "status": "approved" if approve else "rejected", "detail": out}


def fleet_status() -> dict[str, Any]:
    code, out = _run(["fleet-status", "--json"])
    import json as _j
    return {"ok": code == 0, **(_j.loads(out.splitlines()[-1]) if code == 0 and out else {"raw": out})}


def observe_status() -> dict[str, Any]:
    c1, o1 = _run(["audit-list", "closed_loop"])
    c2, o2 = _run(["audit-list", "verification"])
    events = [l for l in (o1 + "\n" + o2).splitlines() if l.strip()]
    return {"ok": c1 == 0 and c2 == 0, "count": len(events), "events": events[-20:]}


def curiosity_status() -> dict[str, Any]:
    c, out = _run(["audit-list", "curiosity"])
    probes = [l for l in out.splitlines() if l.strip()]
    return {"ok": c == 0, "count": len(probes), "probes": probes[-20:]}


def inventory_status() -> dict[str, Any]:
    c, out = _run(["audit-list", "inventory"])
    events = [l for l in out.splitlines() if l.strip()]
    return {"ok": c == 0, "count": len(events), "events": events[-20:]}


def purposes_status() -> dict[str, Any]:
    c, out = _run(["purpose-list"])
    packs = [l for l in out.splitlines() if l.strip()]
    return {"ok": c == 0, "count": len(packs), "purposes": packs}


def desktop_drive_status() -> dict[str, Any]:
    c, out = _run(["audit-list", "desktop_drive"])
    drives = [l for l in out.splitlines() if l.strip()]
    return {"ok": c == 0, "count": len(drives), "drives": drives[-20:]}


def voice_status() -> dict[str, Any]:
    c, out = _run(["audit-list", "voice"])
    turns = [l for l in out.splitlines() if l.strip()]
    return {"ok": c == 0, "count": len(turns), "turns": turns[-20:]}
