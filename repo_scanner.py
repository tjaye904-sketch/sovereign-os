#!/usr/bin/env python3
"""
Scans a git repo, computes REAL deterministic stats (commit count, file
counts, test count, recent history), then asks a model to turn those facts
into readable architecture/status documents. The model describes the given
data; it is explicitly told not to invent numbers.

Usage:
    python3 repo_scanner.py --repo /path/to/sovereign-os --out docs/generated
"""

import argparse
import json
import subprocess
import urllib.request
from pathlib import Path

OLLAMA_URL = "http://localhost:11434/api/generate"
MODEL = "sovereign-executor:latest"


def run(cmd, cwd):
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    return result.stdout.strip()


def gather_stats(repo: Path):
    files = run(["git", "ls-files"], repo).splitlines()
    by_ext = {}
    for f in files:
        ext = Path(f).suffix or "(no ext)"
        by_ext[ext] = by_ext.get(ext, 0) + 1

    commit_count = run(["git", "rev-list", "--count", "HEAD"], repo)
    recent_commits = run(["git", "log", "--oneline", "-20"], repo).splitlines()

    test_count = "unknown"
    if (repo / "build").exists():
        show = run(["ctest", "--test-dir", "build", "--show-only"], repo)
        test_count = str(sum(1 for line in show.splitlines() if "Test #" in line))

    vision_path = repo / "VISION.md"
    state_path = repo / "STATE.md"

    return {
        "total_files": len(files),
        "files_by_extension": by_ext,
        "commit_count": commit_count,
        "recent_commits": recent_commits,
        "test_count": test_count,
        "vision_doc": vision_path.read_text() if vision_path.exists() else None,
        "state_doc": state_path.read_text() if state_path.exists() else None,
    }


def call_model(stats: dict):
    prompt = f"""You are documenting a software project based ONLY on the
facts given below. Do not invent numbers, milestones, or history that
isn't stated here. If something isn't in the data, write "unknown" rather
than guessing.

FACTS:
{json.dumps(stats, indent=2)[:12000]}

Produce two things, clearly separated:

===ARCHITECTURE===
(YAML describing: scope, current build design, known policies/rules if
present in the vision doc, vision summary, source-of-truth file paths)

===STATUS===
(YAML describing: commit_count, test_count, files_by_extension, a short
factual summary of recent activity based on recent_commits, and
completion status ONLY if the vision/state docs explicitly state it —
otherwise "unknown")
"""
    payload = json.dumps({"model": MODEL, "prompt": prompt, "stream": False}).encode()
    req = urllib.request.Request(
        OLLAMA_URL, data=payload, headers={"Content-Type": "application/json"}
    )
    with urllib.request.urlopen(req, timeout=600) as resp:
        return json.loads(resp.read())["response"]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", required=True)
    parser.add_argument("--out", default="docs/generated")
    args = parser.parse_args()

    repo = Path(args.repo)
    out_dir = repo / args.out
    out_dir.mkdir(parents=True, exist_ok=True)

    stats = gather_stats(repo)
    (out_dir / "raw_stats.json").write_text(json.dumps(stats, indent=2))
    print(f"Wrote deterministic stats to {out_dir}/raw_stats.json")

    model_output = call_model(stats)

    if "===STATUS===" in model_output:
        arch_part, status_part = model_output.split("===STATUS===", 1)
        arch_part = arch_part.replace("===ARCHITECTURE===", "").strip()
    else:
        arch_part, status_part = model_output, ""

    (out_dir / "architecture.yaml").write_text(arch_part)
    (out_dir / "status.yaml").write_text(status_part)

    print(f"Wrote {out_dir}/architecture.yaml and {out_dir}/status.yaml")
    print("Review both by hand — the model describes the facts, but verify "
          "it didn't drift from raw_stats.json before trusting it.")


if __name__ == "__main__":
    main()
