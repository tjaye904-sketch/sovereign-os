# origin/main divergence

Local `main` is ahead of `origin/main` with host/cognitive work; remote has 1 commit not in local.

**Do not** `git pull` blindly (may rewrite history unexpectedly).

Options when you care about GitHub:
1. `git fetch origin && git log --oneline main..origin/main` inspect remote-only commit
2. `git log --oneline origin/main..main` inspect local-only
3. Prefer merge: `git merge origin/main` if remote commit is valuable
4. Push only when intentional: `git push origin main` (may need `--force-with-lease` only if history rewritten — avoid unless you understand)

Daily use does not require origin to match.
