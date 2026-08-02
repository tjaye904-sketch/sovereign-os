# Transparency Completion Rules — Satisfied

Sovereign exposes (via `inspect` / `ask`):

| Requirement | Section | Command example |
|-------------|---------|-----------------|
| current identity and active authority | identity | `ask who am I` / `inspect identity` |
| loaded memory and provenance | memory | `ask about memory` |
| selected model/provider and routing reason | model | `ask which model` |
| policy evaluations | policy | `inspect policy` |
| planned actions | plans | `ask pending plans` |
| approvals and denials | approvals | `ask approvals` |
| executed commands | executions | `ask executed commands` |
| state changes | state | `inspect state` |
| timing and resource usage | timing | `inspect timing` |
| errors and recovery actions | errors | `ask about errors` |
| immutable audit evidence | audit | `inspect audit` |
| subsystem health | health | `inspect health` |
| specification coverage | coverage | `inspect coverage` |

Full dump: `sovereign-cli inspect` or `inspect all`
