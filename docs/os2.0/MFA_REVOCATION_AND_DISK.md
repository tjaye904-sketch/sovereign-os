# MFA, revocation, and disk writing — how they relate

These are **different layers**. Confusing them causes fear of “installer = MFA” or “revoke = wipe disk.”

## 1. Disk writing (installer / ISO)

| Action | What it does | Authority |
|--------|----------------|-----------|
| Build ISO | Writes a **file** under `dist/` | Your user account |
| Boot live ISO in QEMU | Reads ISO; **does not** change host disks | QEMU |
| `sovereign-install` / `nixos-install` | **Partitions and formats a block device** | Explicit root + **you name `/dev/sdX`** |

Disk install is **hardware lifecycle**. It is gated by:

- Human naming the target disk
- Root/sudo on the machine
- (Future) Secure Boot / TPM measured boot

**MFA is not required for disk write today**, and MFA does not by itself perform disk writes.

## 2. Revocation (identity / sessions)

| Action | What it does | Disk impact |
|--------|----------------|-------------|
| `logout` / `session-revoke` | Invalidates **this CLI session** in the identity store | Writes only under `SOVEREIGN_DATA_ROOT` (small state files) |
| `principal-suspend` | Principal cannot act until re-enabled | Identity store only |
| `principal-revoke` | Principal permanently disabled | Identity store only |
| Governance reject | Blocks a **pending decision** | Governance store only |

Revocation **stops authority** inside Sovereign’s data root. It does **not** format drives, uninstall the OS, or delete the ISO.

## 3. MFA (not implemented yet)

Multi-factor authentication means: password **plus** a second factor (TOTP, hardware key, TPM quote) before issuing a session or approving high-risk actions.

Planned binding (when built):

- **Login MFA** → required to get a session
- **Step-up MFA** → required for `loop --auto-approve`, `execute`, or (later) **confirm disk install**
- **Recovery MFA** → owner recovery path if factors are lost

MFA would **gate** dangerous operations; it would not replace the explicit disk-device confirmation.

## 4. Recommended policy when installer is built

```
install-to-disk:
  requires: owner session + step-up MFA (future) + typed disk path + typed CONFIRM
  never: auto-approve from cognitive loop
  always: audit entry before first write to block device
```

**Summary:** Revocation and MFA protect **who may act**. Disk writing is a **separate, explicit, human-confirmed** physical operation. Today, only Sovereign’s own data directory is written by identity/governance commands—not your NVMe/SSD partitions.
