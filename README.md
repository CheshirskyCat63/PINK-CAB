# PINK CAB

> First-person neon-pink Tatra taxi-work / arcade-sim / vehicle-parkour game in an effectively endless retrofuturist longitudinal city.

**Repository authority:** `CheshirskyCat63/PINK-CAB` is the sole active Git technical source of truth for PINK CAB. The historical `CheshirskyCat63/DEADRACE` repository is migration-source / legacy-salvage only.

## Start here

- Jira Game Studio product lane: `CD-519`
- Core Code Complete: `CD-793` — DONE
- Administrative / Design Reconciliation gate: `CD-841` — DONE
- Broader Mechanics Freeze: `CD-848` — independent of the hero-model handoff
- Pre-model vehicle / asset handoff gate: `CD-856`
- Native Unreal Chaos vehicle program: `CD-785..CD-792`
- Repository cut-over gate: `CD-558`
- BASE-100 program: `CD-746`; scope owner: `CD-753`
- Confluence authority index: `6586369`
- Repository authority page: `12451841`
- Machine-readable authority: `docs/AUTHORITY.yaml`
- Current control/mechanics release contract: `docs/PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md` / Confluence `16744449` / Jira `CD-848`
- Current vehicle stack: `docs/PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md`
- Hero-vehicle pre-model freeze: `docs/PINK_CAB_PRE_MODEL_VEHICLE_FREEZE.md`
- Tatra asset import contract: `docs/PINK_CAB_TATRA_ASSET_IMPORT_CONTRACT.md`

## Truth rule

`CANON -> SPECIFIED -> IMPLEMENTED -> VERIFIED`

Documentation never implies runtime verification. Exact executable evidence wins for implementation/verification status.

The 2026-09-18 control/mechanics owner correction is SPECIFIED but its QA-01..QA-20 matrix remains NOT RUN until exact executable evidence exists.

## FIRST EURO

First 12 months: **PC / single-player / Level 1 + approved Level 2 foundation and gameplay scope**. Full Mechanics Freeze remains a separate gate; unresolved world/content scope does not reopen the verified hero-vehicle pre-model contract.

The hero-vehicle lane is frozen for asset substitution under `CD-856`. After that terminal handoff is synchronized across Git/Jira/Confluence, `CD-855` consumes the pre-model vehicle freeze and Tatra import contract without changing Chaos/input/gameplay authority.

## Vehicle stack authority

- **Chaos Vehicles / native Unreal physics**: sole production hero-Tatra road-dynamics owner.
- **Native bounded PS2-style damage/destruction**: authored damage states, detachable parts, pooled debris and selective Chaos events.
- **PINK CAB**: input/profile adapters, Tatra calibration, Vehicle Health, persistence, normalized telemetry and presentation.
- Required third-party vehicle/damage dependency spend: **EUR 0**.
- FGear/VDS evaluation material is archived fallback research only and is not production authority.

The current executable path is already verified through clean build, automation, PIE/runtime and packaged Win64 smoke. Any future external solver adoption requires an explicit new decision and migration gate.

## Development entry

Current work keeps full Mechanics Freeze governance separate from the hero-vehicle model lane. `CD-856` is the terminal handoff owner; `CD-855` proceeds only against that verified pre-model and asset-import contract. `main` remains canonical integration. Use short-lived Jira-keyed branches/worktrees and read `CONTRIBUTING.md`, `docs/README.md`, `docs/AUTHORITY.yaml`, and current gate evidence before changing product code or authority.

Historical PRE-FGEAR material remains in the repository for audit context only; it must not be interpreted as the next production step.

## Legacy boundary

Do not import DEADRACE pursuit/police/combat/launcher/destruction/chase-drone mechanics or old input mappings unless a current PINK CAB owner decision explicitly re-adopts them. DEADCORN and DEADBALL remain separate products.
