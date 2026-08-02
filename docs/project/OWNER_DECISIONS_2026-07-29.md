# Owner Decisions — 2026-07-29

| # | Topic | Decision |
|---|--------|----------|
| 1 | Product surface v1 | **CLI only** (`sovereign-cli`). No local HTTP API in v1. |
| 2 | Hermes dashboard | **Separate**. No integration bridge in v1. |
| 3 | Sync | **Journal only**. Offline export/import intents; no multi-device peer protocol yet. |
| 4 | Argon2id costs | **Tuned for hardware** at process start (see below), overridable by env. |

## Argon2 hardware tuning

Auto-selected once per process:

- **Parallelism** = `min(4, hardware_concurrency)` (at least 1)
- **Memory** ≈ RAM/32, clamped **[32 MiB, 256 MiB]**; hosts with **&lt; 3 GiB RAM** use **32 MiB**
- **Time cost** = 3 on strong hosts (≥8 GiB and ≥4 cores), else **2**

Overrides:

```bash
export SOVEREIGN_ARGON2_T=3      # time cost
export SOVEREIGN_ARGON2_M=65536  # memory KiB
export SOVEREIGN_ARGON2_P=2      # parallelism
```

`sovereign-cli status` prints the active `argon2=t=… m_kib=… p=…` line.
