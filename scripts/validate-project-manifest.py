#!/usr/bin/env python3

from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
manifest = root / "PROJECT_MANIFEST.yaml"

required_paths = [
    "docs/project/VISION.md",
    "docs/project/SOVEREIGN_BOOTSTRAP.md",
    "docs/project/IDENTITY_SERVICE_SPEC.md",
    "docs/project/PROJECT_STATUS.md",
    "docs/project/IMPLEMENTATION_MATRIX.md",
    "docs/project/IMPLEMENTATION_ROADMAP.md",
    "docs/project/ENGINEERING_JOURNAL.md",
    "docs/project/AUDITS/Audit-001-Runtime-v1.md",
    "docs/project/governance/GD-001-Engineering-Governance.md",
]

errors = []

if not manifest.is_file():
    errors.append("PROJECT_MANIFEST.yaml is missing.")
else:
    text = manifest.read_text(encoding="utf-8")
    for key in (
        "schema_version:",
        "project:",
        "state:",
        "canonical_documents:",
        "governance:",
        "audits:",
        "validation:",
    ):
        if key not in text:
            errors.append(f"Manifest is missing required key: {key}")

for relative_path in required_paths:
    if not (root / relative_path).is_file():
        errors.append(f"Canonical document is missing: {relative_path}")

if errors:
    for error in errors:
        print(f"ERROR: {error}", file=sys.stderr)
    raise SystemExit(1)

print("Project manifest validation passed.")
