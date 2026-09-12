# PINK CAB

> First-person neon-pink Tatra taxi-work / arcade-sim / vehicle-parkour game in an effectively endless retrofuturist longitudinal city.

**Repository authority:** `CheshirskyCat63/PINK-CAB` is the sole active Git technical source of truth for PINK CAB. The historical `CheshirskyCat63/DEADRACE` repository is migration-source / legacy-salvage only.

## Start here

- Jira Game Studio product lane: `CD-519`
- PRE-FGEAR development program: `CD-754`
- Repository cut-over gate: `CD-558`
- BASE-100 program: `CD-746`; scope owner: `CD-753`
- Readiness: Jira `CD-660/CD-661` / Confluence `6553617` / `docs/PINK_CAB_START90_READINESS.md`
- Confluence authority index: `6586369`
- Repository authority page: `12451841`
- Machine-readable authority: `docs/AUTHORITY.yaml`
- PRE-FGEAR execution index: `docs/PINK_CAB_PRE_FGEAR_EXECUTION_INDEX.md`

## Truth rule

`CANON -> SPECIFIED -> IMPLEMENTED -> VERIFIED`

Documentation never implies runtime verification. Exact executable evidence wins for implementation/verification status.

## FIRST EURO

First 12 months: **PC / single-player / full Level 1 + Level 2**. Multiplayer/coop/common rooms, Level 3 gameplay, lifestyle ServiceNodes, full Taxi Regulator and daily insurance implementation are post-FIRST-EURO.

## Vehicle stack authority

- FGear Vehicle Physics: sole hero road-dynamics owner.
- Debugging Delight Vehicle Damage System: sole permanent body-deformation owner.
- PINK CAB: input/profile adapters, Tatra Expression, Vehicle Health bridge, persistence glue and normalized telemetry.
- Before PF-16: **zero paid dependencies introduced by the PRE-FGEAR corridor**. No temporary production vehicle or deformation solver.

Neither FGear nor VDS is claimed installed until exact repository/build evidence proves it.

## Development entry

PRE-FGEAR work follows `CD-754` / `CD-755..CD-771`. Repository cut-over `CD-558` is complete; PF-00..PF-16 are certified complete through the evaluation/purchase gate. The next action is owner-approved FGear acquisition/install; FGear and VDS remain uninstalled. Use short-lived Jira-keyed branches; `main` is canonical integration.

Read `CONTRIBUTING.md`, `docs/README.md`, `docs/AUTHORITY.yaml`, and `docs/PINK_CAB_PRE_FGEAR_EXECUTION_INDEX.md` before changing product code or authority.

## Legacy boundary

Do not import DEADRACE pursuit/police/combat/launcher/destruction/chase-drone mechanics or old input mappings unless a current PINK CAB owner decision explicitly re-adopts them. DEADCORN and DEADBALL remain separate products.
