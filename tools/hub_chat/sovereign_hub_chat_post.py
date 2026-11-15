#!/usr/bin/env python3
"""POST /api/chat {prompt} -> run sovereign-hub-chat.sh and return JSON."""
from __future__ import annotations
import json, os, subprocess, sys
from http.server import BaseHTTPRequestHandler, HTTPServer

CHAT = os.path.expanduser(
    os.environ.get(
        "SOVEREIGN_HUB_CHAT_BIN",
        os.path.expanduser("~/.local/bin/sovereign-hub-chat"),
    )
)
PORT = int(os.environ.get("SOVEREIGN_HUB_CHAT_PORT", "8768"))


class H(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):  # quieter
        sys.stderr.write("[hub-chat] " + (fmt % args) + "\n")

    def _json(self, code: int, obj: dict):
        body = json.dumps(obj).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        if self.path not in ("/api/chat", "/chat"):
            return self._json(404, {"ok": False, "error": "not found"})
        n = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(n) if n else b"{}"
        try:
            data = json.loads(raw.decode() or "{}")
        except json.JSONDecodeError:
            return self._json(400, {"ok": False, "error": "bad json"})
        prompt = str(data.get("prompt") or data.get("message") or "").strip()
        if not prompt:
            return self._json(400, {"ok": False, "error": "empty prompt"})
        if not os.path.isfile(CHAT) or not os.access(CHAT, os.X_OK):
            return self._json(
                500,
                {
                    "ok": False,
                    "error": f"chat bridge missing: {CHAT}",
                    "hint": "install scripts/sovereign-hub-chat.sh to ~/.local/bin/sovereign-hub-chat",
                },
            )
        try:
            proc = subprocess.run(
                [CHAT, prompt],
                capture_output=True,
                text=True,
                timeout=float(os.environ.get("SOVEREIGN_HUB_CHAT_TIMEOUT", "180")),
                env=os.environ.copy(),
            )
        except subprocess.TimeoutExpired:
            return self._json(504, {"ok": False, "error": "timeout"})
        text = (proc.stdout or "") + (proc.stderr or "")
        if "command not found" in text.lower():
            return self._json(500, {"ok": False, "error": "command not found", "raw": text[-2000:]})
        return self._json(
            200,
            {
                "ok": proc.returncode == 0,
                "exit_code": proc.returncode,
                "text": text[-12000:],
            },
        )

    def do_GET(self):
        if self.path in ("/health", "/api/health"):
            return self._json(200, {"ok": True, "service": "sovereign-hub-chat", "chat_bin": CHAT})
        return self._json(404, {"ok": False})


if __name__ == "__main__":
    print(f"sovereign-hub-chat API on http://127.0.0.1:{PORT}  bin={CHAT}", flush=True)
    HTTPServer(("127.0.0.1", PORT), H).serve_forever()
