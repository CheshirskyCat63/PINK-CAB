# PINK CAB · Vehicle Physics Calibration Program

Canonical planning/admin mirror for the vehicle-physics refinement program requested on 2026-09-26.

## Authority and scope

- Audited Git baseline: `main@7df0fc546e36141c2866a5f5028599eedd37c4a2`.
- Runtime dynamics owner: Unreal Engine 5.8.2 Native Chaos Vehicles behind `IPinkCabVehicleDynamicsProvider`.
- Current control/mechanics authority: Confluence page 47 / Jira CD-848.
- Single execution umbrella: Jira CD-648; existing subsystem cards remain domain owners. CD-921 is terminal integration/evidence only.
- Durable Confluence program: page `22413538`, child of page 47.
- This documentation/admin program does **not** change runtime, vehicle controls, city, MetaRoad, content or accepted world gates. The current city/world state is intentionally frozen while Vehicle Physics V2 executes.

## Documents

1. [PINK_CAB_VEHICLE_PHYSICS_CALIBRATION_PROGRAM_2026-09-26.md](PINK_CAB_VEHICLE_PHYSICS_CALIBRATION_PROGRAM_2026-09-26.md) — P00–P11 execution program with PHY-001..048.
2. [PARAMETER_LEDGER_2026-09-26.md](PARAMETER_LEDGER_2026-09-26.md) — audited current executable values, conflicting historical authorities and disposition.\n3. [TASKS.csv](TASKS.csv) — canonical PHY-001..048 execution registry; all rows begin PLANNED.\n4. [TESTS.csv](TESTS.csv) — 96 acceptance rows; all rows begin NOT RUN.\n5. [SOURCE_PACKAGE_MAP_2026-09-26.md](SOURCE_PACKAGE_MAP_2026-09-26.md) — complete disposition of the uploaded detailed-plan package and its source snapshots.\n6. [ADMIN_AND_HANDOFF_2026-09-26.md](ADMIN_AND_HANDOFF_2026-09-26.md) — clean authority graph, city freeze and exact next execution order.

## First execution order

Do not begin with “feel tuning.” Execute:

`P00-S minimal versioned profile envelope → P00 observability → P01 engine-off/idle correctness → P02 clutch/RPM continuity → P03 steering/input feel → P04 acceleration/gearing → P05 mass/wheels/inertia → P06 suspension → P07 tires/brakes/heat → P08 persistence/soak → P09 complete 613/603-I/77 profiles + migrations → P10 cabin extension seam → P11 evidence/human/admin convergence`.\n\n**Review correction:** the schema skeleton cannot wait until P09 because P00/P01 evidence already requires profile id/version/hash and P08 persists profile identity. Before PHY-001, establish only the minimum stable profile envelope: model/profile ID, schema/calibration version, units/provenance metadata and compatibility/migration identity. P09 still owns the full per-model parameter population, geometry calibration and migration completion.

The engine-off symptom is not yet assigned to a root cause. Current code already gates the partial-clutch external torque path on Running ignition, while final provider throttle/torque paths and Chaos mechanical-sim state have separate responsibilities. P00 instrumentation is therefore mandatory before any fix.

## Current numerical conflicts that must not be hidden

- Physical profile idle: 750 RPM; requested next warm carb calibration: 900–950 RPM.
- Physical profile max: 8500 RPM; gearbox safe-engine threshold: 6500 RPM.
- Executable boosted profile: 260 Nm / roughly 250 hp test target; older Confluence drivetrain text: 240 Nm / 180 hp.
- Executable ratios/final drive differ from historical page-27 ratios/final drive.
- Current 2980/1520/1520 geometry and 205/70R14 wheel data are donor Tatra 613 provenance, not automatic Tatra 603 Series 1 or Tatra 77 truth.

No one of those conflicts is “fixed” by this docs branch. They are explicitly converted into measured implementation gates.


## Current focus lock

The current city/MetaRoad/world state is frozen. Development focus is Vehicle Physics V2 only until the owner explicitly reopens world work. Execute from CD-648 and the canonical documents in this folder; use CD-921 only for terminal integration/evidence.
