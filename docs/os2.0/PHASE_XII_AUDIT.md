# Phase XII — Audit integrity chain

Append-only `audit.dat` plus `audit.chain` (prev_hash|entry_hash FNV-1a).

```bash
sovereign-cli audit-verify
# audit_chain_ok=1 tip=<hex>
```
