# Phase IV — Governance / Policy versioning & dry-run

| Feature | CLI |
|---------|-----|
| Versioned rules file | `data_root/policy/rules.v1` |
| Inspect version | `sovereign-cli policy-version` |
| Dry-run evaluation | `sovereign-cli policy-dry-run <role> <action>` |
| Set rule (bumps version) | `sovereign-cli policy-rule-set <action> <min-role> allow\|deny` |
| Check (live) | `sovereign-cli policy-check <role> <action>` |
