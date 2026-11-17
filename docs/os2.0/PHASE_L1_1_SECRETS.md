# L1.1 Secret Manager MVP

## Behavior
- Store: `data_root/secrets/items/<name>/value` (owner read/write) + `meta`
- Names: alphanumeric, `_`, `-`, `.` only; no path separators
- Audit category `secrets` with action put|get|list|exists|delete — **never** the value
- CLI: `secret-put`, `secret-get`, `secret-list`, `secret-exists`, `secret-delete`

## Not in MVP
- Encryption at rest (0600 files only)
- Federation replication of secrets
- Injection into remote nodes (later leaps)
