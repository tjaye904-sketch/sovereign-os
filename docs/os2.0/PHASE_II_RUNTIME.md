# Phase II — Operating System Runtime

## Role

Sovereign is the **primary orchestration service** (under systemd on Linux ISO, or
embedded ServiceOrchestrator for in-process composition).

## Capabilities (this tree)

| Capability | Implementation |
|------------|----------------|
| service manager | `core/os_runtime/ServiceOrchestrator` |
| dependency graph | topological `after` edges |
| lifecycle | StartAll/StopAll/StartOne/StopOne |
| watchdog | `WatchdogTick` + RestartPolicy |
| health monitoring | per-service `health` callback |
| crash recovery | OnFailure / Always restart with max_restarts |
| event routing | existing `core/runtime` EventBus |
| logging | Audit + runtime log manager |
| metrics | health_monitor (runtime) + orchestrator status |

## systemd mapping (ISO)

| Unit | Role |
|------|------|
| sovereign.target | isolate target |
| sovereign-daemon.service | agent loop |
| sovereign-getty.service | console |

## Next

Service discovery registry persistence; metrics export; PID 1 supervisor option for minimal images.
