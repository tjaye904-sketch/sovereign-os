# EVIDENCE WP-18 / WP-19 / WP-20

## WP-18 Model enrichment
```bash
bash scripts/verify-model-enrichment.sh
# Expect PASS tags + generate; enrichment=model_enriched when model healthy
```

## WP-19 Session login
```bash
bash scripts/session-login-setup.sh
# stores SOVEREIGN_OWNER_SECRET in ~/.config/sovereign/env (mode 600)
source ~/.config/sovereign/env.bash
sovereign-session
```

## WP-20 Full host smoke
```bash
bash scripts/smoke-full-host.sh
# or: sovereign-smoke-full
```

Commit: `WP-18/19/20: model probe, session login, full host smoke`
