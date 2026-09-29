# PINK CAB · P02 / PHY-009 · Authoritative Driveline Architecture · 2026-09-28

Base authority: `main@4cc96ab3361039f053d609ba4a6e1020adb45c0b`.
Owner: CD-644 under CD-648. This document is an implementation contract, not a completion claim.

## Problem proven by runtime evidence

The accepted P01 engine baseline is not the problem. The current P02 boundary is.

UE 5.8 simple Chaos Vehicles has no clutch simulation in `FSimpleEngineSim`; when mechanically connected, engine RPM is matched directly to wheel/transmission speed. PINK CAB therefore created a partial-clutch external wheel-torque path and handed off to native Chaos mechanical transmission only at full coupling.

Runtime RED evidence proved that this architecture is non-causal:
- `0.999 → 1.000` can change torque/RPM because the solver itself changes;
- partial clutch transmits engine→wheel torque but has no wheel→engine reaction;
- full coupling and partial coupling do not share one energy path.

PHY-009 is not allowed to be closed by retuning thresholds or hiding the discontinuity.

## Authority boundaries

P01 remains frozen:
- ignition / Off / Running / Stalled semantics;
- warm idle target 925 RPM;
- limiter / health permission;
- accepted torque curve and engine response;
- engine-off coast behavior.

P02 owns:
- clutch torque transfer;
- wheel/shaft→engine reaction;
- transmission ratio application;
- engine braking through the driveline;
- continuous transition from open to fully engaged clutch;
- reverse using the same physical model;
- clutch condition/heat/wear torque-capacity input.

Chaos remains authoritative for:
- rigid body;
- wheel contact;
- suspension;
- tire forces/slip;
- wheel angular state.

## Target architecture

There is exactly one driveline equation for every coupling value from 0 to 1.

`engine authority → clutch torque solver ↔ input/output shaft → gear/final ratio → rear-wheel torque → Chaos tire/contact`

The clutch reaction is equal/opposite on the engine side. A faster wheel/shaft can back-drive the engine. A faster engine can drive the wheels. Full coupling is a state of the same solver, not a switch to another propulsion implementation.

### Domain layer

`FPinkCabClutchDrivelineModel` is a pure deterministic model with explicit:
- Open / Slipping / Locked state;
- engine-side slip;
- authored maximum clutch torque;
- clutch coupling;
- condition/heat/wear capacity multiplier;
- effective engine inertia;
- synchronization horizon;
- signed engine-side transmitted torque;
- signed axle torque after ratio/efficiency;
- equal/opposite engine reaction impulse.

No world access, no UObject, no hidden assist. The clutch domain uses local numerical substeps derived from the authored synchronization horizon; this does not change the project or Chaos timestep.

### UE adapter layer

A dedicated PINK CAB Chaos adapter will:
- read live engine RPM and driven-wheel angular velocity;
- preserve native P01 engine evolution as the single engine authority;
- derive the clutch predictor's signed free-engine net torque from the actual native angular-momentum change over that same physics step, rather than re-estimating the torque curve/drag in a second engine model;
- feed the domain model once per control/physics step;
- apply rear-wheel drive/reaction torque through Chaos wheel torque APIs;
- feed clutch reaction into the existing P01 engine state without resetting chassis/wheels;
- keep a single manual H-pattern gear authority;
- export causal telemetry for every state/torque term.

The runtime adapter must not integrate the clutch twice because of UI/cockpit events.

## Explicitly rejected implementations

1. **0.995/0.999/full-coupling threshold handoff** — changes solver at the boundary.
2. **Keep Chaos gear engaged and only scale engine torque** — UE simple engine still hard-couples RPM to wheel speed.
3. **Per-frame full vehicle SetSnapshot to force RPM** — rewrites unrelated wheel/chassis state and is not an acceptable production coupling mechanism.
4. **Direct body velocity/force boost** — bypasses tire/contact causality.
5. **One-way external launch torque** — cannot represent hill load, engine braking or wheel→engine reaction.
6. **Hidden rev-match / anti-stall / launch helper** — prohibited by no-assist doctrine.
7. **Raw CMV clutch module copied as final physics** — UE 5.8 modular clutch is useful reference code but current implementation contains fixed angular-velocity equalization behavior and is not an accepted PINK CAB physical calibration.

## Verification ladder

### D1 — pure domain
Must pass deterministic unit tests:
- open clutch isolation;
- steady locked torque transfer;
- engine-fast reaction;
- wheel-fast back-drive;
- capacity/wear limiting;
- continuous 0.999→1.000;
- reverse sign;
- timestep consistency, including composed engine-demand + clutch-reaction integration across 30/60/120 numerical step sizes.

### D2 — engine / Chaos adapter
Must prove:
- no snapshot reset of chassis/wheels;
- one update per runtime step;
- native Chaos mechanical transmission is not a second propulsion source;
- engine RPM telemetry reports the same state consumed by the clutch model;
- no ABS/TC/ESP/yaw/throttle helpers introduced.

### D3 — runtime causal matrix
Five identical-reset repeats per condition:
- coupling 0 / 0.25 / 0.50 / 0.75 / 0.999 / 1.000;
- 1st and R;
- zero/25/50/100% throttle;
- the base 240-cell coupling matrix runs as a stationary driveline bench with an explicit test-only physical dynamometer brake on the driven wheels. The dynamometer torque exceeds the maximum clutch/first-gear wheel torque, is compiled only for automation, and does not modify production brake/tire calibration. D3 additionally asserts near-zero measured driven-wheel RPM during every sample, so 0.999→1.000 compares one shaft/load state rather than diverging tire/chassis trajectories;
- low and high wheel-speed back-drive is a separate moving-shaft comparison against an open-clutch control;
- limiter approach;
- engine-off coast;
- low-RPM overload/stall;
- hot/worn capacity input.

D3 does not use body-force, velocity, snapshot-loop or threshold handoff helpers. Moving load behavior belongs to D4.

### D4 — load fixtures
- flat launch;
- incline load;
- throttle lift / engine braking;
- stop-in-gear stall;
- reverse;
- no synthetic movement with engine off.

### D5 — determinism / delivery
- the accepted P01 project physics configuration remains frozen; P02 must not force a new global async/fixed timestep to make its tests pass;
- D3/D5 evidence is accumulated inside the physics simulation over exact simulated-time windows and time-weighted, so render/game cadence cannot silently change the compared physical duration;
- D3 records the observed mechanical timestep as evidence but does not require one hard-coded global timestep; the accepted P01 project physics configuration remains the authority;
- supported 30/60/120 render/game FPS with the same frozen project physics configuration;
- exact-head build + full `PinkCab.Vehicle.Physics`;
- zero-debt/writer guard;
- packaged runtime;
- HUMAN gate before P02 acceptance.

## Completion rule

PHY-009 is GREEN only when the single-path implementation passes D1–D5 and the previous P01 evidence remains green. Passing a boundary test alone is insufficient.
