# Phase III — Identity System (maturity increment)

## Added

| Capability | API / CLI |
|------------|-----------|
| Service principal | `CreateServicePrincipal` → `sovereign-cli service-create <name>` |
| Machine/device principal | `CreateMachinePrincipal` → `sovereign-cli machine-create <name>` |
| Role | services get `service` role; machines get no default elevated role |

## Still open (later)

MFA, certificates, hardware binding, delegation tokens, cryptographic machine attestation.

## Test

```bash
sovereign-cli bootstrap owner 'secret'
sovereign-cli login owner 'secret'
sovereign-cli service-create agent-worker
sovereign-cli machine-create dell-5550
```
