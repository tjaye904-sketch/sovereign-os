# L1.3 Workspace Controller MVP

Logical lifecycle only (no headless node / VM start — that is L2.x).

| Status | Meaning |
|--------|---------|
| active | Available for planning / binding |
| suspended | Paused; still listed; not “archived” |
| archived | Existing archive path |

CLI: `workspace-activate`, `workspace-suspend`, `workspace-status`  
API: `Activate`, `Suspend`, `FindByName`, `ListByStatus`, `StatusName`
