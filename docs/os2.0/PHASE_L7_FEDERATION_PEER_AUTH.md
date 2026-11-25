# L7: Federation peer auth scaffold

Branch: `leap/L7-federation-peer-auth`

**Scope.** Local-only authentication scaffold for federation peers. No networking,
consistent with the 2026-07-30 owner decision (network mesh not enabled by default).

- New library `sovereign_federation` (`core/federation/peer_auth.{hpp,cpp}`).
- A peer can be enrolled only if its node id exists in `NodeRegistry`.
- Enrollment returns a one-time 256-bit secret; only a derived verifier is stored in
  `data_root/federation/peers.dat` (mode 0600). Secrets never go in peer bundles.
- Handshake: server issues a single-use, TTL-bound challenge; peer answers
  `HMAC-SHA256(SHA256(secret), "sov-peer-auth-v1|<node>|<challenge>")`.
- Revocation takes effect immediately and drops pending challenges.
- SHA-256/HMAC are self-contained (checked against FIPS 180-4 / RFC 4231 vectors in the test).

**Known limits (scaffold).**
- The stored verifier is the HMAC key, so a stolen `peers.dat` allows impersonation
  of enrolled peers. Treat it as a credential. A public-key scheme is the follow-up.
- Pending challenges are in-memory (lost on restart; peers simply re-request).
- Not yet wired into CLI, audit log, or peer-bundle import.

**Test.** `peer_auth_smoke_test` (registered via `sovereign_add_smoke`).
