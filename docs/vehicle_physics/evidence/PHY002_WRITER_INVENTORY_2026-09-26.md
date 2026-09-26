# PHY-002 — State Writer Inventory Evidence — 2026-09-26

Status: **DONE / exact canonical main verified**

- merge commit: `e5077942ee93499247e61d358c55f0b2c222cb72`
- PR: **#31**
- exact-main Windows run: **36230537265**
- writer groups: **68**
- executable writer occurrences: **80**
- unknown / stale / mismatched writer groups: **0 / 0 / 0**
- forbidden direct side paths: **0**

Authority artifacts:
- `docs/vehicle_physics/STATE_WRITER_INVENTORY.csv`
- `docs/vehicle_physics/STATE_WRITER_CONSUMER_MAP.md`
- `scripts/ci/verify_vehicle_physics_writers.py`

Key results:
- final Chaos steering/throttle/brake/wheel torque writer: `FPinkCabChaosVehicleDynamicsProvider`;
- semantic ignition authority: `FPinkCabCockpitState`; normal mechanical-sim authority: `FPinkCabChaosCockpitBridge`;
- requested/engaged gear authority: `FPinkCabGearboxController`; Chaos target gear writer: `FPinkCabChaosCockpitBridge`;
- runtime mass/CoM authority: `FPinkCabChaosLoadBridge`;
- engine RPM has no project-side direct writer and is read from Chaos;
- partial-clutch external rear torque remains inventoried as a known P02 path.

RED audit found dormant `AlignInitialPresentationToGround()` that could teleport the pawn and zero chassis linear/angular velocity. No production call site existed, so the dead method was deleted. The guard now rejects direct force/impulse/torque/velocity/actor-teleport paths and any non-false Chaos assist-control assignment.

Exact-main run **36230537265**:
- repository/code-health: **32/32 PASS**
- zero-debt: **0**
- writer inventory: **PASS**
- PinkCabEditor build: **PASS**
- PinkCab.Vehicle.Physics: **3/3 PASS**
- PinkCab Game build: **PASS**
- baseline collector: **PASS**

The verification workflow emitted `PHYSICS_BASELINE_e507794_RUN36230537265`; this is a verification snapshot and does not replace the accepted PHY-001 frozen baseline.

No city/MetaRoad, accepted R1/R2/R3 world behavior, control grammar or tuning values changed.

Next: **PHY-003 · Causal drivetrain telemetry**.
