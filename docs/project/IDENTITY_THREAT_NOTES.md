# Identity Service — Threat Notes (v1)

**Date:** 2026-07-29  
**Status:** Accepted with known debt

## Assets

- Principal records and role bindings
- Credential verifiers (never raw secrets)
- Sessions (bearer of authentication)

## Threats in scope (v1)

| Threat | Mitigation |
|--------|------------|
| Offline verifier cracking | Production must use Argon2id (`sov-id-v2-argon2id`). Current builds without argon2 headers use `sov-id-v1` only when allowed (Debug or `SOVEREIGN_ALLOW_INSECURE_VERIFIER=1`). |
| Stolen session file | Sessions expire (default 12h); revoke on credential change; revoke-all on suspend/revoke principal |
| Double owner bootstrap | Rejected after first Owner exists |
| Operator privilege | Only owner session may create operators / change status / assign roles |
| Secret leakage in logs/events | Verifiers only; tests must not print secrets |

## Known debt

1. **Argon2id** not linked in this environment (no headers). Implementation reserved; scheme id defined.
2. No rate limiting on Authenticate yet (add before network exposure).
3. File store is not encrypted at rest (OS filesystem permissions assumed for local-first v1).

## Production gate

Do not enroll real human credentials until Argon2id path is implemented and default under NDEBUG without insecure override.


Argon2id parameters are hardware-tuned (2026-07-29 owner decision).
