#!/usr/bin/env bash
# Resolve merge origin/main into local main (L0–L1.2 + host line).
# Run FROM repo root while merge is in progress.
set -euo pipefail
ROOT="${1:-$PWD}"
cd "$ROOT"

if ! git rev-parse -q --verify MERGE_HEAD >/dev/null; then
  echo "Not in a merge. Abort."
  exit 1
fi

echo "=== Policy ==="
echo "  THEIRS (origin): orchestration/cognitive engines, desktop_operator, remote tests"
echo "  OURS   (local):  keep secrets/facets already in tree; union CMake tests"
echo "  MANUAL: README, host hpp/cpp, cognitive_service, product CMake, PROJECT_STATUS"

# --- both-added: prefer origin implementations (fuller engines) ---
for f in \
  core/cognitive/evolution_engine.cpp \
  core/cognitive/planning_engine.cpp \
  core/product/desktop_operator.cpp \
  testing/product/sovereign_desktop_operator_smoke_test.cpp
do
  if git ls-files -u -- "$f" | grep -q .; then
    git checkout --theirs -- "$f"
    git add -- "$f"
    echo "theirs: $f"
  fi
done

# Prefer origin headers if conflicted as add/add siblings
for f in \
  core/cognitive/evolution_engine.hpp \
  core/cognitive/planning_engine.hpp \
  core/product/desktop_operator.hpp
do
  if [[ -f "$f" ]] && grep -q '<<<<<<' "$f" 2>/dev/null; then
    git checkout --theirs -- "$f" 2>/dev/null || true
    git add -- "$f" 2>/dev/null || true
  fi
done

# --- testing/CMakeLists.txt: union both sides ---
if git ls-files -u -- testing/CMakeLists.txt | grep -q .; then
  python3 - << 'PY'
from pathlib import Path
import subprocess
root = Path(".")
# extract stages
ours = subprocess.check_output(["git","show",":2:testing/CMakeLists.txt"], text=True)
theirs = subprocess.check_output(["git","show",":3:testing/CMakeLists.txt"], text=True)
# Prefer ours as base, append any sovereign_add_smoke from theirs missing in ours
lines_ours = ours.splitlines()
set_ours = set(lines_ours)
extra = []
for line in theirs.splitlines():
    if "sovereign_add_smoke" in line and line not in set_ours:
        extra.append(line)
out = ours.rstrip() + "\n"
if extra:
    out += "\n# --- from origin/main (merge) ---\n"
    out += "\n".join(extra) + "\n"
Path("testing/CMakeLists.txt").write_text(out)
print("union testing/CMakeLists.txt +%d smoke lines" % len(extra))
PY
  git add testing/CMakeLists.txt
fi

# --- core/product/CMakeLists.txt: union library sources ---
if git ls-files -u -- core/product/CMakeLists.txt | grep -q .; then
  python3 - << 'PY'
from pathlib import Path
import subprocess, re
ours = subprocess.check_output(["git","show",":2:core/product/CMakeLists.txt"], text=True)
theirs = subprocess.check_output(["git","show",":3:core/product/CMakeLists.txt"], text=True)
# Start from longer / theirs if it lists more product sources
def src_names(t):
    return set(re.findall(r"[\w/]+\.(cpp|hpp|c|h)", t))
if len(src_names(theirs)) >= len(src_names(ours)):
    base, other = theirs, ours
else:
    base, other = ours, theirs
# Ensure desktop_operator / purpose / gateway appear if in either
need = []
for token in ["desktop_operator.cpp", "purpose_runtime.cpp", "external_gateway.cpp",
              "desktop_operator.hpp", "purpose_runtime.hpp", "external_gateway.hpp"]:
    if token in ours or token in theirs:
        if token not in base:
            need.append(token)
text = base
# naive: if add_library block, inject missing cpp after first .cpp line
for token in need:
    if token.endswith(".cpp") and token not in text:
        text = re.sub(r"(add_library\([^\)]*)", r"\1\n    " + token + "\n", text, count=1)
Path("core/product/CMakeLists.txt").write_text(text)
print("merged product CMakeLists (base len %d)" % len(text))
PY
  git add core/product/CMakeLists.txt
fi

# --- README / PROJECT_STATUS: take ours then append remote note ---
for f in README.md docs/project/PROJECT_STATUS.md; do
  if git ls-files -u -- "$f" | grep -q .; then
    git show ":2:$f" > "$f.ours" 2>/dev/null || true
    git show ":3:$f" > "$f.theirs" 2>/dev/null || true
    if [[ -f "$f.ours" ]]; then
      {
        cat "$f.ours"
        echo ""
        echo "<!-- merge note 2026-11-18: also integrated origin/main orchestration/cognitive engines -->"
      } > "$f"
      git add -- "$f"
      echo "ours+note: $f"
    fi
    rm -f "$f.ours" "$f.theirs"
  fi
done

# --- cognitive_service.cpp / sovereign_host: leave for structured strip of conflict markers preferring BOTH features ---
python3 - << 'PY'
from pathlib import Path
import subprocess, re

def strip_prefer_union(path, prefer="theirs_for_engine"):
    p = Path(path)
    if not p.exists():
        return
    raw = p.read_text()
    if "<<<<<<" not in raw:
        return
    # Parse conflict regions: keep both sides concatenated for .cpp host wiring,
    # for pure duplicate choose non-empty longer side
    out = []
    i = 0
    s = raw
    while True:
        a = s.find("<<<<<<<", i)
        if a < 0:
            out.append(s[i:])
            break
        out.append(s[i:a])
        m = s.find("=======", a)
        e = s.find(">>>>>>>", m)
        if m < 0 or e < 0:
            out.append(s[a:])
            break
        head = s[a:m]
        # strip first line of conflict marker
        ours_body = "\n".join(head.splitlines()[1:])
        tail = s[m:e]
        theirs_body = "\n".join(tail.splitlines()[1:])
        # end marker line
        eline = s.find("\n", e)
        if eline < 0:
            eline = len(s)
        else:
            eline += 1
        # Heuristic: if one side empty, take other; else take longer; for host take both unique lines
        if not ours_body.strip():
            chosen = theirs_body
        elif not theirs_body.strip():
            chosen = ours_body
        elif path.endswith("cognitive_service.cpp"):
            # prefer theirs (origin engines wiring) if substantially longer
            chosen = theirs_body if len(theirs_body) >= len(ours_body) * 0.8 else (ours_body + "\n" + theirs_body)
        elif "sovereign_host" in path:
            # union: theirs first (new members) then ours-only lines
            chosen = theirs_body
            for line in ours_body.splitlines():
                if line.strip() and line not in theirs_body:
                    chosen += "\n" + line
        else:
            chosen = theirs_body if len(theirs_body) >= len(ours_body) else ours_body
        out.append(chosen)
        if not chosen.endswith("\n"):
            out.append("\n")
        i = eline
    text = "".join(out)
    # remove any leftover markers
    text = re.sub(r"^<<<<<<<.*\n", "", text, flags=re.M)
    text = re.sub(r"^=======\n", "", text, flags=re.M)
    text = re.sub(r"^>>>>>>>.*\n", "", text, flags=re.M)
    p.write_text(text)
    print("conflict-stripped:", path)

for f in [
    "core/cognitive/cognitive_service.cpp",
    "core/product/sovereign_host.cpp",
    "core/product/sovereign_host.hpp",
]:
    # start from working tree conflicted file
    strip_prefer_union(f)

import subprocess
for f in [
    "core/cognitive/cognitive_service.cpp",
    "core/product/sovereign_host.cpp",
    "core/product/sovereign_host.hpp",
]:
    subprocess.check_call(["git", "add", f])
    print("added", f)
PY

echo "=== remaining unmerged ==="
git diff --name-only --diff-filter=U || true
echo "=== next: inspect remaining, then build ==="
