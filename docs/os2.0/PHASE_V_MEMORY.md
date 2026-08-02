# Phase V — Memory aging & importance

| API | CLI |
|-----|-----|
| AgeScore / Ranked | `memory-rank` |
| SetImportance | `memory-importance <id-hex> <0..1>` |
| PruneBelow | `memory-prune <min-score>` |
| Touch | (internal on access paths later) |

Half-life ~30 days; access_count boosts score.
