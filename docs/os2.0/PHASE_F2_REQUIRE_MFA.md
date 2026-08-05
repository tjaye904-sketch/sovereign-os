# F.2 — Mandatory MFA on login (optional policy)

## Behavior

| `SOVEREIGN_REQUIRE_MFA` | TOTP enrolled | `login name secret` | `login name secret totp` |
|-------------------------|---------------|---------------------|---------------------------|
| unset / 0 | no | OK | OK |
| unset / 0 | yes | OK (password only) | OK + TOTP check |
| **1** | no | OK | OK |
| **1** | yes | **MfaRequired** | OK if TOTP valid |

Execute step-up (C.3) is separate: `execute … --mfa` when enrolled.

## Usage

```bash
export SOVEREIGN_REQUIRE_MFA=1
sovereign-cli login owner 'secret'          # fails if TOTP enrolled
sovereign-cli login owner 'secret' 123456   # AuthenticateWithMfa
```
