# PINK CAB · Vehicle Technology Stack — Native Unreal Chaos

**Status:** CURRENT IMPLEMENTATION-FACING VEHICLE STACK AUTHORITY  
**Reconciled:** 2026-09-16 · `CD-843`  
**Production program:** `CD-785..CD-792`  
**Confluence:** `13303842` — `PINK CAB · Native Unreal Chaos Arcade-Sim Program`  
**Canonical verified head entering reconciliation:** `e3241dc92a4d89d7a9abd22c2e3920a1ff91a25b`

## 1. Hard stack boundary

- **Chaos Vehicles / native Unreal physics** is the sole production road-dynamics owner for the hero Tatra.
- `IPinkCabVehicleDynamicsProvider` remains the game-facing abstraction; the production provider is Chaos-backed.
- PINK CAB owns Tatra physical profiles, input adaptation, telemetry, bounded arcade controls, Vehicle Health consequences, persistence and presentation.
- There is no required FGear or VDS production dependency and no vehicle-plugin purchase gate.
- FGear/VDS documents and evaluation tasks are historical research/fallback evidence only.
- A second competing vehicle solver is forbidden. Narrow native corrections are allowed only when bounded, data-driven, observable and independently disableable.

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
- no ABS / no ESP;
- current target 180 hp / 240 Nm / 195 km/h;
- BMW E34 535i remains the current behavioral/control-capability reference only;
- progressive/catchable RWD oversteer and visible soft-barge body work remain target behavior.

Calibration values belong to `FPinkCabChaosPhysicalProfile` and related versioned data, not vendor types.

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