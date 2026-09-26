# PINK CAB · Vehicle Technology Stack — Native Unreal Chaos

**Status:** CURRENT IMPLEMENTATION-FACING VEHICLE STACK AUTHORITY  
**Reconciled:** 2026-09-26 · `CD-843` + `CD-648`  
**Current execution umbrella:** `CD-648`; terminal integration/evidence gate: `CD-921`  
**Canonical calibration program:** Confluence `22413538` + `main:docs/vehicle_physics/`  
**Technology page:** Confluence `13303842` — `PINK CAB · Native Unreal Chaos Arcade-Sim Program`  
**Audit baseline before this reconciliation:** `main@55ee8173af3c627cf26a06b95ec8628f5077179c`

## 1. Hard stack boundary

- **Chaos Vehicles / native Unreal physics** is the sole production road-dynamics owner for the hero Tatra.
- `IPinkCabVehicleDynamicsProvider` remains the game-facing abstraction; the production provider is Chaos-backed.
- PINK CAB owns versioned Tatra physical profiles, semantic input translation, telemetry, Vehicle Health consequences, persistence and presentation.
- There is no required FGear or VDS production dependency and no vehicle-plugin purchase gate.
- FGear/VDS documents and evaluation tasks are historical research/fallback evidence only.
- A second competing vehicle solver is forbidden.
- **No hidden driving assists are permitted in the base road-physics path:** no ABS, TC, ESP, auto-countersteer, yaw rescue, automatic throttle/brake/clutch/gear decisions, hidden trajectory correction, velocity overwrite or direct-force boost used to fake responsiveness. Device filtering/sensitivity is allowed only as input translation and must not rewrite an already-authored steering/trajectory command.
- Wallride/magnet/contact mechanics remain explicit gameplay layers with their own gates; they are not evidence that road handling may use hidden rescue logic.

## 2. Damage / destruction boundary

FIRST EURO destruction targets maximum perceived destruction per millisecond:

- authored damage-state mesh swaps;
- detachable doors, bumpers, wheels, lamps, glass and trim;
- pooled low-poly rigid-body debris;
- sparks, dust, smoke, decals, skid marks, sound and camera response;
- selective Chaos rigid-body / Geometry Collection events only where justified;
- deterministic Vehicle Health consequences and persistence.

Continuous third-party soft-body deformation is not required.
## 3. Locked physical / handling identity

The solver change does not reopen the approved Tatra identity:

- bespoke early/Gen-1 Tatra 603-family retrofuturist taxi;
- rear-engine RWD, air-cooled V8;
- 1450 kg base / 1550 kg full fuel;
- 58 kg heroine / 49 kg daughter;
- 1657 kg full-fuel crew-only reference fixture;
- current declared five-passenger max fixture 2107 kg;
- no ABS / no TC / no ESP;
- current executable gameplay tune observed at the audit baseline is approximately **250 hp / 260 Nm / 8500 RPM max / 750 RPM idle**; it is a gameplay tune, not a historical factory claim;
- next healthy warmed carbureted idle target is **900–950 RPM** (925 RPM is only the initial calibration center);
- the older 180 hp / 240 Nm / 6000–6200 RPM values are historical reference until an explicit profile decision re-accepts them;
- BMW E34 535i remains a control/readability reference; **Assetto Corsa is a quality reference for physical causality and connected tire/suspension/drivetrain behavior, not a donor of numeric racing-car parameters**;
- progressive/catchable RWD oversteer and visible old-Tatra body work remain target behavior, but a committed bad input must still be able to lock, stall or spin the car.

Calibration values belong to `FPinkCabChaosPhysicalProfile` and related versioned data, not vendor types.

### Open calibration blockers from the 2026-09-26 causality audit

These are **observed current settings/paths, not accepted targets**:

- partial-clutch drive uses an external rear-wheel torque path that must share one combustion/limiter/health/torque-capacity result with the fully coupled Chaos path;
- `WheelLoadRatio = 0.38` weakens load sensitivity and must be A/B tested against a full-load-sensitivity physical reference rather than treated as an accessibility requirement;
- nominal friction multipliers front/rear = `2.00 / 0.50` intentionally make the rear axle easy to spin and must not remain solely as a drift shortcut;
- current steering multiplies the held target by speed down to `0.55` at 120 km/h; speed may shape device sensitivity, but must not silently shrink an already-authored road-wheel target or remove needed countersteer authority;
- throttle response `pow(driver, 0.55)` amplifies 25% pedal input to roughly 47% command and requires dosability A/B after drivetrain causality is fixed.

Runtime acceptance for these items is owned by `CD-648` stages P00–P07 and remains **NOT VERIFIED** until exact-SHA telemetry/tests plus the owner feel gate are recorded.

## 4. Runtime / traffic doctrine

- Hero taxi uses native Chaos.
- Only physically relevant nearby traffic may promote into full physics.
- Far traffic remains bounded logical/kinematic simulation.
- Wallride/magnet behavior is a bounded external contact/state layer over Chaos, never a replacement vehicle solver.
- Presentation may amplify readable roll/pitch/heave/cabin response but cannot silently rewrite authoritative trajectory.

## 5. Verification authority

Current proof is executable evidence, not documentation:

- native Chaos provider and pawn tests;
- Chaos calibration/baseline runtime smoke;
- cockpit playable runtime tests;
- CD-802 integrated PIE gate;
- full `PinkCab` automation matrix;
- clean editor build + packaged Win64 boot smoke.

FGear/VDS-specific tests may remain only where they prove those dependencies are absent or archived.