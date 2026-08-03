# Recommendations: MFA, revocation, disk writing

## Policy (adopted)

1. **Revocation is mandatory and offline-safe** — session/principal revoke must work with only local identity store; no network dependency.
2. **Disk install is never implied by revoke or MFA** — separate explicit human confirmation of block device.
3. **MFA (when built) is step-up for risk**, not a substitute for disk confirmation:
   - Login MFA → issue session
   - Step-up MFA → `execute`, `loop --auto-approve`, and **future** install
4. **`loop --auto-approve` must never call installer APIs.**
5. **Installer path** (future): owner session + (future MFA) + typed `/dev/...` + typed `CONFIRM` + audit before first write.
6. **Default cognitive loop stays paused** for human approval.

## Implementation order (this program)

A.1 Session/principal revoke verified ← **current**  
A.2 Model-enrichment reliability  
A.3 Audit on all sensitive paths  
A.4 Quarantine dead runtime bulk  
A.5 PROJECT_TREE current  
Then B (VM installer), C (MFA), …
