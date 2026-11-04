#!/usr/bin/env python3
"""WP-02: Truthful capability status for Sovereign host operators."""
from __future__ import annotations

import json
import os
import subprocess
import sys
from pathlib import Path
from urllib.request import urlopen, Request
from urllib.error import URLError, HTTPError

HOME = Path.home()


def _get(url: str, timeout: float = 2.0):
    try:
        with urlopen(url, timeout=timeout) as r:
            return json.loads(r.read().decode()), None
    except Exception as e:
        return None, str(e)


def unit_active(name: str) -> bool:
    p = subprocess.run(
        ["systemctl", "--user", "is-active", name],
        capture_output=True,
        text=True,
    )
    return p.stdout.strip() == "active"


def display_ready() -> dict:
    disp = os.environ.get("DISPLAY") or ""
    way = os.environ.get("WAYLAND_DISPLAY") or ""
    # systemd user environment (may differ from this shell)
    sysd = {}
    try:
        p = subprocess.run(
            ["systemctl", "--user", "show-environment"],
            capture_output=True,
            text=True,
            timeout=3,
        )
        for line in p.stdout.splitlines():
            if "=" in line:
                k, _, v = line.partition("=")
                if k in ("DISPLAY", "WAYLAND_DISPLAY", "XDG_RUNTIME_DIR"):
                    sysd[k] = v
    except Exception:
        pass
    has = bool(disp or way or sysd.get("DISPLAY") or sysd.get("WAYLAND_DISPLAY"))
    return {
        "shell_DISPLAY": disp or None,
        "shell_WAYLAND_DISPLAY": way or None,
        "systemd": sysd,
        "graphical_session_likely": has,
    }


def ollama_up() -> bool:
    host = os.environ.get("OLLAMA_HOST", "http://127.0.0.1:11434").rstrip("/")
    try:
        with urlopen(host + "/api/tags", timeout=2) as r:
            return r.status == 200
    except Exception:
        return False


def build_status() -> dict:
    cu_body, cu_err = _get("http://127.0.0.1:8765/health")
    hub_body, hub_err = _get("http://127.0.0.1:8766/api/status")
    sys_body, sys_err = _get("http://127.0.0.1:8767/health")

    disp = display_ready()
    cu_listening = bool(cu_body and cu_body.get("ok"))
    sys_listening = bool(sys_body and sys_body.get("ok"))
    hub_listening = bool(hub_body and hub_body.get("ok"))

    # Truthful CU: process up AND graphical session likely (headed requirement)
    cu_ready = cu_listening and disp["graphical_session_likely"]
    sys_ready = sys_listening
    daemon_ready = unit_active("sovereign-daemon.service") or unit_active(
        "sovereign-daemon"
    )

    operators_ready = daemon_ready and sys_ready and cu_ready

    reasons = []
    if not daemon_ready:
        reasons.append("daemon_inactive")
    if not sys_listening:
        reasons.append(f"sys_down:{sys_err or 'no_health'}")
    if not cu_listening:
        reasons.append(f"cu_down:{cu_err or 'no_health'}")
    elif not disp["graphical_session_likely"]:
        reasons.append("cu_listening_but_no_display_env")
    if not hub_listening:
        reasons.append(f"hub_down:{hub_err or 'no_health'}")

    return {
        "ok": True,
        "wp": "02",
        "operators_ready": operators_ready,
        "daemon": daemon_ready,
        "hub_listening": hub_listening,
        "sys": {
            "listening": sys_listening,
            "ready": sys_ready,
            "headed": (sys_body or {}).get("headed"),
            "health": sys_body,
            "error": sys_err,
        },
        "computer_use": {
            "listening": cu_listening,
            "ready": cu_ready,
            "health": cu_body,
            "error": cu_err,
            "note": (
                "ready requires health OK and DISPLAY/WAYLAND in shell or systemd user env"
            ),
        },
        "display": disp,
        "ollama": ollama_up(),
        "model": os.environ.get("SOVEREIGN_OLLAMA_MODEL", ""),
        "data_root": os.environ.get(
            "SOVEREIGN_DATA_ROOT", str(HOME / ".local/share/sovereign")
        ),
        "reasons_not_ready": reasons,
        "legacy_hub_status": hub_body,
    }


def main():
    if len(sys.argv) > 1 and sys.argv[1] in ("--pretty", "-p"):
        print(json.dumps(build_status(), indent=2))
    else:
        print(json.dumps(build_status()))


if __name__ == "__main__":
    main()
