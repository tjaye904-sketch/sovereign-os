# Milestone C — MFA

## Policy (locked)

1. Password (local secret) remains factor 1.
2. TOTP is factor 2 when enrolled; until enrolled, login behaves as today.
3. Step-up MFA for: `execute`, `loop --auto-approve`, future host disk install.
4. MFA does not write disks.
5. Recovery codes (C.3) before mandatory enrollment.

## C.1 (this) — Scaffold

- Types: `MfaRequired`, `MfaNotEnrolled`, `MfaInvalid`
- APIs: `GetMfaStatus`, `BeginTotpEnrollment`, `ConfirmTotpEnrollment`, `AuthenticateWithMfa`
- CLI: `mfa-status`
- Default: not enrolled; existing tests/login unchanged

## C.2 — TOTP crypto

- Generate Base32 secret, `otpauth://` URI
- Verify 6-digit TOTP (30s window, ±1 step)
- Persist enrollment under identity store

## C.3 — Recovery + step-up

- Recovery codes (hashed)
- Policy flag: step-up required for execute
