#!/usr/bin/env python3
import http.server, json, os, subprocess, sys, tempfile, threading
step = [0]
class H(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        self.rfile.read(int(self.headers.get("Content-Length", 0)))
        step[0] += 1
        msg = {"role": "assistant", "content": '{"name":"system_status","arguments":{}}'} if step[0] == 1 else {"role": "assistant", "content": "Sovereign OS 2.0 verified via autonomous tool loop."}
        body = json.dumps({"model": "mock-qwen", "message": msg, "done": True}).encode()
        self.send_response(200); self.send_header("Content-Type", "application/json"); self.send_header("Content-Length", str(len(body))); self.end_headers(); self.wfile.write(body)
    def log_message(self, *a): pass
srv = http.server.HTTPServer(("127.0.0.1", 0), H)
threading.Thread(target=srv.serve_forever, daemon=True).start()
with tempfile.TemporaryDirectory() as d:
    env = dict(os.environ, SOVEREIGN_DATA_ROOT=d, OLLAMA_HOST=f"http://127.0.0.1:{srv.server_address[1]}")
    out = subprocess.check_output([sys.argv[1], "chat-tools", "status"], env=env, text=True)
    assert "tool_calls_executed=1" in out and "autonomous tool loop" in out, out
    print(out.strip())
srv.shutdown()
