# PINK CAB · Vehicle Physics admin handoff · 2026-09-26

## Administrative state

**READY FOR VEHICLE-PHYSICS EXECUTION.** This handoff records documentation/authority convergence only; it does not claim runtime implementation.

- Canonical repository: `CheshirskyCat63/PINK-CAB`.
- Pre-convergence docs HEAD: `7560628addcb24cedd58cf1492762094828daa18` (PR #23, docs only).
- Canonical Git folder: `docs/vehicle_physics/`.
- Single Jira execution umbrella: **CD-648**.
- Terminal integration/evidence gate: **CD-921** under CD-848.
- Canonical Confluence program: **22413538**.
- Product Authority Index: **6586369**.
- Duplicate Confluence page **22413517** is archived and must not be executed from.
- Native Unreal Chaos Vehicles is the sole production road-dynamics solver; FGear/VDS is historical evidence where older text conflicts.
- PR #24 was closed instead of merging its duplicate `docs/vehicle_physics_v2/` tree.

## Scope freeze

The city is intentionally left exactly as it is now.

- no city-generation work;
- no MetaRoad changes;
- no road-width/curb/world-geometry changes;
- no R1/R2/R3 reopening;
- no fare/economy expansion through this program;
- no input-grammar redesign.

Vehicle Physics work may consume the current city only as a human/soak test environment.

## Canonical next execution order

`P00-S → P00 → P01 → P02 → P03 → P04 → P05 → P06 → P07 → P08 → P09 → P10 → P11`

1. **P00-S** — create the minimum stable versioned profile envelope before PHY-001: model/profile id, schema/calibration version, unit/provenance metadata and migration identity.
2. **P00** — exact executable baseline, writer/consumer inventory, causal drivetrain telemetry and repeatable fixtures.
3. **P01** — prove Off/Stalled cannot generate positive engine drive torque; preserve coast/grade/inertia; move healthy warm idle from current 750 toward owner target 900–950.
4. Continue sequentially through drivetrain continuity, response, acceleration, mass/inertia, suspension, tires/brakes, persistence, model profiles, future cabin seam and terminal evidence.

## Evidence discipline

- `TASKS.csv`: 48 canonical PHY execution rows, all `PLANNED`.
- `TESTS.csv`: 96 acceptance rows, all `NOT RUN`.
- No old green test run certifies a future candidate.
- Every runtime change requires exact SHA/profile/load/evidence.
- Handling-feel gates require owner acceptance even when automation is green.
- Compilation alone is never completion.

## Known first technical conflicts

- executable warm idle is 750 RPM; owner target is 900–950 RPM;
- physical profile allows 8500 RPM while an independent gearbox safe threshold is 6500;
- executable ~250 hp / 260 Nm / current ratios conflict with older 180 hp / 240 Nm / old-ratio documentation;
- rear friction multiplier 0.50 must be proven as legitimate combined-grip calibration rather than a wheelspin shortcut;
- 613 donor geometry must not silently become 603 Series 1 or 77 truth;
- engine-off movement must be classified as propulsion vs coast/grade/inertia/contact before any fix.

## Stop conditions

Do not start feel tuning before P00 observability. Do not “fix” engine-off movement by zeroing chassis velocity. Do not create a second vehicle solver or duplicate Pawn family. Do not reopen city/world scope during this physics workstream.
