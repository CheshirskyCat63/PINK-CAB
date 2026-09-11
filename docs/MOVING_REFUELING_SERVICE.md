# PINK CAB · Moving Refueling Service

**Status:** CURRENT FIRST-EURO IMPLEMENTATION-FACING SPEC / NOT IMPLEMENTED / NOT VERIFIED
**Authority:** Confluence `6062115`; Jira `CD-540`, numeric/state owner `CD-593`, BASE-100 service `CD-752`; product root `CD-519`.

## FIRST EURO role

Moving refueling is a year-one gameplay mechanic. BASE-100 covers its state machine, numeric eligibility envelopes, traffic/streaming ownership, fuel/economy settlement, persistence, failure/retry and tests. Worker/hose/terminal appearance and animation polish do not enter code readiness except where physical collision/constraint is an authored gameplay boundary.

## Ownership

`MovingRefuelSession` owns only the bounded service session. It reuses:

- shared `FuelTank`;
- shared `EconomyService` and stable `TransactionId`;
- normalized `VehicleTelemetry` rather than spreading FGear vendor types through service code;
- common `RoadGraph` / traffic placement;
- common save/schema/evidence infrastructure.

It may not create a second money balance, fuel balance, vehicle solver, input framework or persistence owner. Presentation actors follow authoritative session state; animation completion is not transaction truth.

## Proposed state family — owner confirmation pending

`SearchingQueue → Queued → ApproachingSlot → Connected → Fueling → PaymentPending → Disconnecting → Completed | Aborted`

State semantics/transaction boundaries remain owner-pack work until explicitly accepted.

## Current gameplay direction

- bounded moving service queue/encounter;
- player can join, leave, abort and re-approach without teleport or arbitrary mission fail;
- connection requires explicit distance/speed/lateral/grade eligibility;
- historical **±25 cm around target gap** remains a calibration seed, not final runtime tolerance;
- fuel is credited only while valid connected/fueling state is authoritative;
- voluntary early stop/abort remains supported direction;
- loss of tolerance should resolve through bounded warning/grace/disconnect behavior if current proposed default is accepted;
- collision consequences use normal FGear → VDS/Vehicle Health path;
- success/abort/restart/chunk recycle must converge to one clean exactly-once fuel/economy result.

## Tier scope / pricing

FIRST EURO requires Level1 + Level2 service compatibility. Current economic direction is **L1 cheaper than L2**; exact year-one prices are versioned `FuelPriceProfile` owner/calibration values.

Level3 gameplay is POST-FIRST-EURO. The older `T1 < T2 < T3` direction may be preserved as future design intent, but Tier3 service/pricing does not block year-one BASE-100.

## Input authority

Moving fuel uses the same cabin grammar as the rest of PINK CAB:

- mouse by default = steering;
- hold Space = gaze/free-look and bounded target search;
- `1–4` may quick-recall authored frequent physical targets but never directly actuate them;
- RMB brings/retains the right hand on the current target where that endpoint requires a physical grip;
- LMB presses/holds momentary controls where declared;
- mouse wheel adjusts detents/rotaries/incremental controls where declared.

The universal `LMB ATTENTION / RMB GO` wording is SUPERSEDED. Moving refuel may not fork its own control framework; each endpoint declares which of grip / momentary press-hold / wheel adjustment it supports.

Physical card/terminal interaction is a presentation/interaction endpoint over the authoritative payment state; its visual choreography cannot independently charge or credit fuel.

## Code-facing OPEN values

- queue capacity / active-slot policy;
- service-vehicle speed profile by level/road class;
- target gap and final warning/grace/disconnect bands;
- lateral/speed/grade/relative-height tolerances;
- gameplay-relevant hose break/constraint thresholds if retained;
- fuel transfer rate;
- settlement timing/model;
- insufficient-funds behavior shared with `CD-601/CD-750`;
- interruption/equipment fees if retained;
- telemetry acceptance tolerances.

## Required evidence

`PC-T-FUEL-001..007` plus settlement/reload coverage must prove queue ownership, valid connection, transfer, early abort, tolerance breach, collision, restart/chunk interruption and exactly-once monetary/fuel state.

Every VERIFIED run records exact build/commit, UE/FGear/VDS/profile/config/schema versions where relevant, CityCode/traffic/service seed, session/vehicle/transaction IDs, gap/error telemetry, expected/observed result and artifact paths.

Documentation establishes SPECIFIED only. Runtime remains **NOT IMPLEMENTED / NOT VERIFIED**.
