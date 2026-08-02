# Phase VI — Execution plans & confidence

`CognitiveService::BuildPlan` → CLI `plan <goal...>`

Returns ordered steps (inspect → policy-dry-run → recommend → execute), confidence 0..1, uncertainty notes. Governed steps require approval.
