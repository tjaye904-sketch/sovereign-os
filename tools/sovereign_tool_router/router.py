#!/usr/bin/env python3
"""
WP-03: Chat → tool router for Sovereign host operators.

Maps natural-language requests to computer-use (:8765) and sys (:8767)
actions. Does not grant unrestricted access — only known tools + allowlists
enforced by those services.

Usage:
  python3 -m sovereign_tool_router "open python docs"
  sovereign-tools "list windows"
  sovereign-tools --dry-run "take a desktop screenshot"
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys
import urllib.error
import urllib.request
from typing import Any

CU_URL = os.environ.get("SOVEREIGN_CU_URL", "http://127.0.0.1:8765")
SYS_URL = os.environ.get("SOVEREIGN_SYS_URL", "http://127.0.0.1:8767")


def _post(base: str, action: str, args: dict[str, Any] | None = None, approved: bool = False) -> dict:
    body = json.dumps({"action": action, "args": args or {}, "approved": approved}).encode()
    req = urllib.request.Request(
        f"{base.rstrip('/')}/v1/action",
        data=body,
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    try:
        with urllib.request.urlopen(req, timeout=120) as r:
            return json.loads(r.read().decode())
    except urllib.error.HTTPError as e:
        try:
            return json.loads(e.read().decode())
        except Exception:
            return {"ok": False, "error": f"HTTP {e.code}: {e}"}
    except Exception as e:
        return {"ok": False, "error": str(e)}


def _get(url: str) -> dict:
    try:
        with urllib.request.urlopen(url, timeout=5) as r:
            return json.loads(r.read().decode())
    except Exception as e:
        return {"ok": False, "error": str(e)}


# (regex, service, action, args_builder)
# First match wins — order matters (more specific first).
ROUTES: list[tuple[re.Pattern[str], str, str, Any]] = [
    (re.compile(r"\b(ping|health)\b.*\b(sys|system)\b|\bsys\b.*\bping\b", re.I), "sys", "ping", lambda m: {}),
    (re.compile(r"\b(ping|health)\b.*\b(cu|computer|browser)\b|\bcu\b.*\bping\b", re.I), "cu", "ping", lambda m: {}),
    (re.compile(r"\bping\b", re.I), "cu", "ping", lambda m: {}),
    (
        re.compile(
            r"\b(open|go to|navigate|browse)\b.{0,40}?(https?://\S+)",
            re.I,
        ),
        "cu",
        "browser_open",
        lambda m: {"url": m.group(2).rstrip(".,)")},
    ),
    (
        re.compile(
            r"\b(open|go to|navigate|browse)\b.{0,60}?\b(python\s*docs?|docs\.python)\b",
            re.I,
        ),
        "cu",
        "browser_open",
        lambda m: {"url": "https://docs.python.org/3/"},
    ),
    (
        re.compile(r"\b(purpose\s*list|list\s*purposes)\b", re.I),
        "cu",
        "purpose_list",
        lambda m: {},
    ),
    (
        re.compile(
            r"\b(run\s*purpose|purpose\s*run)\b.{0,40}?(research\.web_collect|ops\.desktop_probe|\w+\.\w+)",
            re.I,
        ),
        "cu",
        "purpose_run",
        lambda m: {
            "purpose_id": m.group(2),
            "url": "https://docs.python.org/3/",
        },
    ),
    (
        re.compile(r"\b(research|collect)\b.{0,40}\b(web|docs?|documentation)\b", re.I),
        "cu",
        "purpose_run",
        lambda m: {
            "purpose_id": "research.web_collect",
            "url": "https://docs.python.org/3/",
        },
    ),
    (
        re.compile(r"\b(snapshot|read\s*(the\s*)?page|page\s*text)\b", re.I),
        "cu",
        "browser_snapshot",
        lambda m: {"max_chars": 3000},
    ),
    (
        re.compile(r"\b(browser\s*)?screenshot\b", re.I),
        "cu",
        "browser_screenshot",
        lambda m: {"name": "router_browser.png"},
    ),
    (
        re.compile(r"\b(list\s*windows|window\s*list|what\s*windows)\b", re.I),
        "cu",
        "desktop_list_windows",
        lambda m: {},
    ),
    (
        re.compile(r"\b(desktop\s*)?screenshot\b|\bscreen\s*shot\b", re.I),
        "cu",
        "desktop_screenshot",
        lambda m: {"name": "router_desktop.png"},
    ),
    (
        re.compile(
            r"\b(open|launch|start)\b.{0,40}\b(system\s*monitor|gnome-system-monitor)\b",
            re.I,
        ),
        "sys",
        "app_open",
        lambda m: {"target": "gnome-system-monitor"},
    ),
    (
        re.compile(r"\b(open|launch|start)\b.{0,40}\b(terminal|gnome-terminal)\b", re.I),
        "sys",
        "app_open",
        lambda m: {"target": "gnome-terminal"},
    ),
    (
        re.compile(r"\b(open|launch)\b.{0,40}\b(files|nautilus|file\s*manager)\b", re.I),
        "sys",
        "app_open",
        lambda m: {"target": "nautilus"},
    ),
    (
        re.compile(r"\b(sys\s*)?status\b|\bsystem\s*status\b", re.I),
        "sys",
        "sys_status",
        lambda m: {},
    ),
]


def plan(text: str) -> dict[str, Any] | None:
    text = (text or "").strip()
    if not text:
        return None
    for rx, service, action, builder in ROUTES:
        m = rx.search(text)
        if m:
            return {
                "service": service,
                "action": action,
                "args": builder(m),
                "matched": rx.pattern[:80],
            }
    return None


def execute(plan_d: dict[str, Any], *, approved: bool = False) -> dict:
    base = CU_URL if plan_d["service"] == "cu" else SYS_URL
    return _post(base, plan_d["action"], plan_d.get("args") or {}, approved=approved)


def route(text: str, *, dry_run: bool = False, approved: bool = False) -> dict:
    p = plan(text)
    if not p:
        return {
            "ok": False,
            "routed": False,
            "error": "no_tool_match",
            "hint": (
                "Try: open https://docs.python.org/3/ | open python docs | "
                "list windows | desktop screenshot | open system monitor | "
                "run purpose research.web_collect | snapshot | ping"
            ),
            "text": text,
        }
    if dry_run:
        return {"ok": True, "routed": True, "dry_run": True, "plan": p}
    result = execute(p, approved=approved)
    return {
        "ok": bool(result.get("ok")),
        "routed": True,
        "plan": p,
        "result": result,
    }


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Sovereign WP-03 chat → tool router")
    ap.add_argument("text", nargs="*", help="Natural language request")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--approved", action="store_true", help="Pass approved=true for high-risk tools")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--list-routes", action="store_true")
    args = ap.parse_args(argv)

    if args.list_routes:
        for rx, svc, act, _ in ROUTES:
            print(f"{svc:4} {act:24} {rx.pattern[:70]}")
        return 0

    text = " ".join(args.text).strip()
    if not text and not sys.stdin.isatty():
        text = sys.stdin.read().strip()
    if not text:
        ap.print_help()
        return 2

    out = route(text, dry_run=args.dry_run, approved=args.approved)
    if args.json or not sys.stdout.isatty():
        print(json.dumps(out, indent=2))
    else:
        if not out.get("routed"):
            print("No tool match:", out.get("error"))
            print(out.get("hint", ""))
            return 1
        if out.get("dry_run"):
            print("PLAN:", json.dumps(out["plan"], indent=2))
            return 0
        print("PLAN:", json.dumps(out.get("plan"), indent=2))
        print("RESULT:", json.dumps(out.get("result"), indent=2)[:4000])
        return 0 if out.get("ok") else 1
    return 0 if out.get("ok") or out.get("dry_run") else 1


if __name__ == "__main__":
    raise SystemExit(main())
