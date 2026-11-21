# L2.1–L2.5 Node federation foundation

| Leap | Feature |
|------|---------|
| L2.1 | Node registry (local + peer) |
| L2.2 | Heartbeat + health + stale mark |
| L2.3 | Bind/unbind workspace name |
| L2.4 | Per-node capability projection |
| L2.5 | Append-only node lifecycle events |

Store: `data_root/node/nodes.dat`, `events.dat`  
No remote RPC yet — registry is the federation control plane MVP.
