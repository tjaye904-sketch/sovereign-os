# Maturity hardening (current)

## In place

- Fail-closed governance; deny-by-default policy
- Argon2id secrets (tunable)
- Append-only audit + hash chain (`audit-verify`)
- Session file local to data_root; logout + session-revoke
- Principal suspend/revoke (owner session)
- Cognitive loop defaults to human approval
- Peer import does not auto-execute
- Release/ASan path on host builds when requested

## Added this pass

- `session-revoke`, `principal-suspend`, `principal-revoke` CLI
- Docs: MFA vs disk writes
- PROJECT_TREE.md for model context refresh

## Next hardening

- MFA/TOTP or WebAuthn for login and step-up
- Rate-limit auth failures
- Signed audit (ed25519) beyond FNV chain
- Installer: typed disk path + CONFIRM; never via `loop --auto-approve`
