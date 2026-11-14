#!/usr/bin/env python3
"""
Multi-point quality gate: build, test, lint (deterministic checks), then
an independent local model reviews the diff for anything the deterministic
checks can't catch. The model can only ADD flags, never override a failed
build or failed tests.

Usage:
    python3 quality_gate.py --worktree ../worktrees/<id> --repo /path/to/main/repo
"""

import argparse
import json
import shutil
import subprocess
import sys
import urllib.request

OLLAMA_URL = "http://localhost:11434/api/generate"
REVIEWER_MODEL = "llama3.1:8b"  # different lineage from the Qwen-based coder


def run(cmd, cwd):
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    return result.returncode == 0, result.stdout + result.stderr


def check_build(worktree):
    ok1, out1 = run(["cmake", "-S", ".", "-B", "build"], worktree)
    ok2, out2 = run(["cmake", "--build", "build"], worktree) if ok1 else (False, "")
    return ok1 and ok2, out1 + out2


def check_tests(worktree):
    if not (worktree / "build").exists():
        return False, "No build directory; build must run first."
    ok, out = run(["ctest", "--test-dir", "build", "--output-on-failure"], worktree)
    return ok, out


def check_lint(worktree):
    if not shutil.which("clang-tidy"):
        return None, "clang-tidy not installed, skipped."
    ok, out = run(
        ["bash", "-c", "git diff --name-only main | grep -E '\\.(cpp|h|hpp)$' "
                        "| xargs -r clang-tidy"],
        worktree,
    )
    return ok, out


def get_diff(worktree, repo):
    ok, out = run(["git", "diff", "main"], worktree)
    return out if ok else ""


def call_model_review(diff, build_ok, test_ok):
    if not diff.strip():
        return {"verdict": "PASS", "issues": ["No diff to review."]}

    prompt = f"""You are an independent, skeptical code reviewer. Do not
assume the code is correct. Build passed: {build_ok}. Tests passed: {test_ok}.

Review this diff for logic errors, missed edge cases, and anything that
doesn't actually do what it appears intended to do. Be specific.

DIFF:
{diff[:12000]}

Respond ONLY with valid JSON in this exact form, no other text:
{{"verdict": "PASS" or "FAIL", "issues": ["specific issue 1", "specific issue 2"]}}
"""
    payload = json.dumps({
        "model": REVIEWER_MODEL, "prompt": prompt, "stream": False
    }).encode()
    req = urllib.request.Request(
        OLLAMA_URL, data=payload, headers={"Content-Type": "application/json"}
    )
    with urllib.request.urlopen(req, timeout=300) as resp:
        raw = json.loads(resp.read())["response"].strip()
    try:
        return json.loads(raw)
    except json.JSONDecodeError:
        return {"verdict": "FAIL", "issues": [f"Reviewer returned unparsable output: {raw[:300]}"]}


def main():
    from pathlib import Path

    parser = argparse.ArgumentParser()
    parser.add_argument("--worktree", required=True)
    parser.add_argument("--repo", required=True)
    args = parser.parse_args()
    worktree = Path(args.worktree)

    report = {}

    build_ok, build_out = check_build(worktree)
    report["build"] = {"passed": build_ok, "log_tail": build_out[-2000:]}
    print(f"[build] {'PASS' if build_ok else 'FAIL'}")

    test_ok, test_out = check_tests(worktree) if build_ok else (False, "skipped, build failed")
    report["tests"] = {"passed": test_ok, "log_tail": test_out[-2000:]}
    print(f"[tests] {'PASS' if test_ok else 'FAIL'}")

    lint_ok, lint_out = check_lint(worktree)
    report["lint"] = {"passed": lint_ok, "log_tail": lint_out[-2000:]}
    print(f"[lint]  {'PASS' if lint_ok else ('SKIPPED' if lint_ok is None else 'FAIL')}")

    diff = get_diff(worktree, args.repo)
    review = call_model_review(diff, build_ok, test_ok)
    report["model_review"] = review
    print(f"[review] {review['verdict']}")
    for issue in review.get("issues", []):
        print(f"    - {issue}")

    overall_pass = (
        build_ok
        and test_ok
        and (lint_ok is not False)
        and review.get("verdict") == "PASS"
    )
    report["overall"] = "PASS" if overall_pass else "FAIL"

    (worktree / ".quality_gate_report.json").write_text(json.dumps(report, indent=2))
    print(f"\nOverall: {report['overall']}  (full report: "
          f"{worktree}/.quality_gate_report.json)")

    sys.exit(0 if overall_pass else 1)


if __name__ == "__main__":
    main()
