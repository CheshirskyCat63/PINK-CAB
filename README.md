# PINK CAB

> First-person neon-pink Tatra taxi-work / arcade-sim / vehicle-parkour game in an effectively endless retrofuturist longitudinal city.

**Repository authority:** `CheshirskyCat63/PINK-CAB` is the sole active Git technical source of truth for PINK CAB. The historical `CheshirskyCat63/DEADRACE` repository is migration-source / legacy-salvage only.

## Start here

- Jira Game Studio product lane: `CD-519`
- Core Code Complete: `CD-793` — DONE
- Administrative / Design Reconciliation gate: `CD-841`
- Native Unreal Chaos vehicle program: `CD-785..CD-792`
- Repository cut-over gate: `CD-558`
- BASE-100 program: `CD-746`; scope owner: `CD-753`
- Confluence authority index: `6586369`
- Repository authority page: `12451841`
- Machine-readable authority: `docs/AUTHORITY.yaml`
- Current vehicle stack: `docs/PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md`

## Truth rule

`CANON -> SPECIFIED -> IMPLEMENTED -> VERIFIED`

Documentation never implies runtime verification. Exact executable evidence wins for implementation/verification status.

## FIRST EURO

First 12 months: **PC / single-player / Level 1 + approved Level 2 foundation and gameplay scope**. Mechanics Freeze remains a later gate; unresolved world/content scope is tracked by `CD-841`.
## Vehicle stack authority

- **Chaos Vehicles / native Unreal physics**: sole production hero-Tatra road-dynamics owner.
- **Native bounded PS2-style damage/destruction**: authored damage states, detachable parts, pooled debris and selective Chaos events.
- **PINK CAB**: input/profile adapters, Tatra calibration, Vehicle Health, persistence, normalized telemetry and presentation.
- Required third-party vehicle/damage dependency spend: **EUR 0**.
- FGear/VDS evaluation material is archived fallback research only and is not production authority.

The current executable path is already verified through clean build, automation, PIE/runtime and packaged Win64 smoke. Any future external solver adoption requires an explicit new decision and migration gate.

## Development entry

Current work follows `CD-841` reconciliation before Mechanics Freeze. Use short-lived Jira-keyed branches/worktrees; `main` is canonical integration. Read `CONTRIBUTING.md`, `docs/README.md`, `docs/AUTHORITY.yaml`, and the current gate/task evidence before changing product code or authority.

Historical PRE-FGEAR material remains in the repository for audit context only; it must not be interpreted as the next production step.

## Legacy boundary

Do not import DEADRACE pursuit/police/combat/launcher/destruction/chase-drone mechanics or old input mappings unless a current PINK CAB owner decision explicitly re-adopts them. DEADCORN and DEADBALL remain separate products.