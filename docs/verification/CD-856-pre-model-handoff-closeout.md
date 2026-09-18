# CD-856 · Pre-Model Vehicle Handoff Closeout

Date: 2026-09-18
Gate: `CD-856`
Broader independent gate: `CD-848`
Next presentation task: `CD-855`

## Canonical Git evidence

- Canonical integration before this administrative sync: `742c6ab2ad8fd1dfa3a2646491e950baedd20e75`.
- Reviewed pre-model PR head: `7b8809fb1fa7877de9ba55f100c6877b34159cd2`.
- Both commits resolve to the same tree: `6c7a395177206f384553d1fdafc371538ac616ed`.
- `git diff 7b8809f..742c6ab` is empty: the verified PR payload and canonical merge contain identical project bytes.
- PR #1 is merged to `main`.

## Executable evidence inherited by identical tree

PR #1 / the identical tree records:

- UE 5.8.2 `PinkCabEditor Win64 Development`: PASS.
- Full `PinkCab` automation: 285 success / 0 fail, exit 0.
- Vehicle and cockpit focused suites: PASS.
- PIE drive smoke: PASS with live first gear, engine RPM and four active wheels.
- Development Win64 BuildCookRun: PASS / AutomationTool exit 0.
- Packaged `L_PinkCab_ChaosWeave` smoke: PASS; no fatal/assert/ensure.
- Staged/archive executable SHA-256 matched.

## 2026-09-18 administrative verification

- Local `main` was fast-forwarded to `origin/main` before audit changes.
- `git diff --check`: PASS.
- `scripts/check-authority.sh`: PASS after the authority-sync edit; Bash syntax check also PASS.
- Stale worktree metadata whose gitdirs no longer existed was pruned; branches and commits were preserved.
- `PINK-CAB_ALL_REFS.bundle` remains preserved locally and is explicitly ignored rather than deleted.

A fresh local editor-build rerun was attempted on the canonical tree but the engine `Build.bat` lock never became available. No UnrealBuildTool/dotnet compiler process started, so this is not a compiler failure and does not invalidate the identical-tree PR evidence. The reproducibility/CI/lock hygiene problem is tracked under `CD-559`.

## Authority correction

The Git mirrors previously named `CD-848` as the direct pre-model owner and contained no `CD-856` reference. This contradicted Jira and Confluence page `15663105`.

This closeout corrects the authority chain to:

`CD-848 broader Mechanics Freeze (independent) → CD-856 pre-model vehicle handoff → CD-855 model/presentation consumer`.

No gameplay, vehicle dynamics, input, economy, taxi, persistence or art asset behavior is changed by this administrative sync.
