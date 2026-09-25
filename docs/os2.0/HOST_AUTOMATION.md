# Host OS Automation — Purpose-Driven Spec

## Intent

General automation of **browsers, windows, and desktop programs** for an operator-defined **purpose**, always **observable** on the desktop, always under **governance**.

## Purpose object (YAML/JSON)

```yaml
purpose_id: research.web_collect
description: Collect text from allowlisted docs sites
require_headed: true
max_risk_auto: low
allowed_hosts:
  - docs.python.org
  - manpages.ubuntu.com
allowed_apps:
  - chromium
  - firefox
allowed_tools:
  - browser.open
  - browser.snapshot
  - browser.screenshot
  - os.notify
steps_hint:
  - open url
  - snapshot
  - store summary via sovereign memory (separate CLI)
```

## Risk

| Risk | Examples | Default |
|------|----------|---------|
| low | open allowlisted URL, snapshot, screenshot | auto |
| medium | click/type in allowlisted context | policy |
| high | download, open arbitrary path | approve |
| critical | destructive, financial submit, credentials | approve always |

## Runtime

- **sovereign-cu-server** — long-lived headed session (required for multi-step)
- **sovereign-cu** — CLI; prefers server when healthy
- **Mission Control / hub** — start purpose, stream actions, show status

## Observability checklist

- [ ] Operator can see the window(s)
- [ ] Each action in audit JSONL
- [ ] Optional PNG artifacts
- [ ] Hub or terminal shows purpose_id active

## Out of scope

Single-vendor lock-in; silent full-session hijack; root automation by default.
