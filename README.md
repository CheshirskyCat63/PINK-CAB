# PINK CAB

> First-person neon-pink Tatra taxi-work / arcade-sim / vehicle-parkour game in an effectively endless retrofuturist longitudinal city.

**Repository authority:** `CheshirskyCat63/PINK-CAB` is the sole active Git technical source of truth for PINK CAB. The historical `CheshirskyCat63/DEADRACE` repository is migration-source / legacy-salvage only.

## Start here

- Jira Game Studio product lane: `CD-519`
- Core Code Complete: `CD-793` — DONE
- Administrative / Design Reconciliation gate: `CD-841` — DONE
- Broader Mechanics Freeze: `CD-848` — independent of the hero-model handoff
- Pre-model vehicle / asset handoff gate: `CD-856`
- Current Native Chaos vehicle program: `CD-648` / P00-P11; terminal gate `CD-921`; administrative closeout `CD-952`
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

Accepted P04 measured gearing runtime: **52239b61bc80e5a63716b9b09b87c520fa09fd05**, delivery **37149462470**, attempt 1; explicitly owner accepted on 2026-10-03. PR #62 integrated it as **4a313d38f0674a3e5048f832a07428defa31ab62**. Read GitHub for later main commits; integration/admin commits never rename accepted executable bytes.

## FIRST EURO

First 12 months: **PC / single-player / Level 1 + approved Level 2 foundation and gameplay scope**. Full Mechanics Freeze remains a separate gate; unresolved world/content scope does not reopen the verified hero-vehicle pre-model contract.

The current Tatra is a replaceable placeholder. CD-855 polish/replacement is deferred until vehicle calibration; only test-blocking defects justify immediate work. CD-856 contracts remain binding; public asset rights stay separate.

## Vehicle stack authority

- **Chaos Vehicles / native Unreal physics**: sole production hero-Tatra road-dynamics owner.
- **Native bounded PS2-style damage/destruction**: authored damage states, detachable parts, pooled debris and selective Chaos events.
- **PINK CAB**: input/profile adapters, Tatra calibration, Vehicle Health, persistence, normalized telemetry and presentation.
- Required third-party vehicle/damage dependency spend: **EUR 0**.
- FGear/VDS evaluation material is archived fallback research only and is not production authority.

Accepted P04 has exact-source tests, fresh package, installed-payload and owner-acceptance evidence. Full clean-source/full-product/packaged-input qualification remains CD-559 engineering; one candidate does not certify the entire product. External solver adoption requires a new explicit decision.

## Development entry

Local build, pinned dependencies, runner identity and acceptance boundaries: [Project setup](docs/PROJECT_SETUP.md).

Current work keeps full Mechanics Freeze governance separate from the frozen working vehicle baseline. `main` is the sole active integration branch; use one short-lived Jira-keyed task branch and one PR per change. The merged recovery history is evidence only, not a continuing execution lane. Read `CONTRIBUTING.md`, `docs/README.md`, `docs/AUTHORITY.yaml`, and current gate evidence before changing product code or authority.

Historical PRE-FGEAR material remains in the repository for audit context only; it must not be interpreted as the next production step.

## Legacy boundary

Do not import DEADRACE pursuit/police/combat/launcher/destruction/chase-drone mechanics or old input mappings unless a current PINK CAB owner decision explicitly re-adopts them. DEADCORN and DEADBALL remain separate products.
