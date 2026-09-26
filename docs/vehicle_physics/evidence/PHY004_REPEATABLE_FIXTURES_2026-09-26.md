# PHY-004 — Repeatable Calibration Fixtures Evidence — 2026-09-26

Status: **DONE / exact canonical main verified**

- runtime PR: #37
- runtime merge: `138b74601815ff668b7eed956e6907f2c124775f`
- exact-main Windows run: **36255150359**
- repo/code-health: **32/32 PASS**
- zero-debt: **0**
- writer guard: **68 groups / 80 occurrences PASS**
- focused physics: **12/12 PASS**
- Editor/Game builds: **PASS**
- baseline validation: **PASS**

## Canonical fixture families

1. `FLAT_LAUNCH`
2. `GRADE_LAUNCH_8PCT`
3. `LIMITER_COUPLING_SWEEP`
4. `CONSTANT_RADIUS`
5. `COMBINED_BRAKE_TURN`
6. `COMBINED_THROTTLE_TURN`
7. `LIFT_OFF`
8. `SPLIT_MU_BRAKE`
9. `STRAIGHT_BRAKING`
10. `SLALOM`
11. `ROUGH_ROAD`

Each fixture binds: deterministic seed, physics dt, FPS cap, model/profile/schema/calibration/hash, vehicle mass/load state, surface/friction, tire state/temperature/wear and exact timestamped throttle/brake/clutch/steering/gear input keyframes.

Fixture/input hashes are deterministic. Five run descriptors generated from the same fixture are comparable; FPS, seed or load drift invalidates comparability.

This closes reusable deterministic fixture infrastructure. It does **not** claim the later handling calibration outcomes in `TESTS.csv` have already been physically executed; those rows remain NOT RUN until their owning stages.

No vehicle tuning, controls, city, MetaRoad or world behavior changed.

Next: **PHY-005 · Single combustion permission**.
