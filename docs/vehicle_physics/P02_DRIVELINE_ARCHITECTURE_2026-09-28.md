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
- the base 240-cell coupling matrix uses a fresh physically settled vehicle for every repeat, then samples exactly the first mechanical response from that proven reset state. This keeps the clutch solver's consumed shaft state identical without introducing a dyno/brake override and prevents later tire/chassis divergence from contaminating the local 0.999→1.000 continuity measurement;
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


- With ignition Off/Stalled, the native mechanical step still advances wheel rotation/contact in neutral, but its idle-engine evolution is discarded; only P02 clutch reaction may mechanically back-drive the engine.


### D3 hot/worn capacity verification
- Capacity 1.0 and 0.5 are verified at coupling 0.999 and 1.0 in both first and reverse.
- Every cell uses a fresh physically settled vehicle and records exactly the first mechanical response step.
- Measured-state service brake/dyno loads are forbidden for this matrix; wheel RPM must remain within the stationary first-step envelope.
- Reduced capacity must reduce transmitted torque while preserving the 0.999→1.0 continuity contract.


### D4 moving-load verification
- D4 uses the real Chaos contact/wheel path with production service brake, tire and suspension calibration.
- Flat first/reverse launch, an 8% incline load, lift-off engine braking and stop-in-gear stall are timed from the authoritative mechanical-step clock rather than wall time.
- Fixture teleport is permitted only before measurement to place a fresh vehicle on the isolated incline; velocities are zeroed before the physical rest gate and no runtime velocity/force injection is permitted.


### D5 cadence invariance
- Supported game/render caps 30, 60 and 120 are measured rather than assumed.
- Each cap executes five fresh physically settled first-step responses and records actual game-thread and mechanical-step deltas.
- No physics timestep, substep or solver setting is modified for D5; the accepted project configuration remains frozen.
- Median torque and engine response must remain within 5% across the observed cadences.

- D4 incline uses a fresh pawn spawned directly at the fixture transform; no teleport or velocity reset is used after physics creation.


### D3 measured-dt engine response normalization
- Single-step Hot/Worn capacity verification compares engine response as `(RPM_after - RPM_reset) / measured mechanical dt`, not raw endpoint RPM.
- This removes render/game-cadence sensitivity from the observation without forcing a global physics timestep, changing solver settings, or relaxing the existing 5% continuity tolerance.
- Torque continuity and the stationary first-step shaft envelope remain independent required gates.


### PHY-009 boundary measured-time window
- The 0.999→1.000 moving boundary fixture uses a fixed 0.05 s settle and 0.50 s measured simulation window, driven by observed mechanical dt rather than a fixed step count.
- Torque/RPM/authority telemetry is time-weighted. Final translational kinetic energy is interpolated to the exact end of the 0.50 s window, so render/game cadence cannot change the physical duration being compared.
- The existing 10% boundary tolerance is unchanged and no project physics timestep is forced.


### D5 normalized engine-response cadence metric
- D5 compares single-step engine response as `(RPM_after - RPM_reset) / measured mechanical dt` across 30/60/120 game caps.
- Raw endpoint RPM is retained as telemetry but is not used as the cadence-invariance metric because the accepted project does not force one global mechanical timestep.
- The 5% cadence-invariance tolerance is unchanged.


### D4 engine-braking fixture isolation
- The moving-load test no longer closes the clutch from 0.75 to 1.0 on the same mechanical boundary as throttle lift.
- After launch it holds throttle with full clutch for 0.50 s, exceeding the 0.20 s accepted clutch synchronization horizon, then starts the engine-braking measurement from that already-coupled state.
- This isolates engine braking from clutch-engagement acceleration without changing engine-brake calibration or the production clutch model.


### D4 reverse direction evidence
- Chaos `FWheelStatus.DriveTorque` is treated as wheel-torque magnitude evidence in the moving fixture, not as the semantic gear-direction authority.
- Reverse direction is proven by signed chassis displacement under load; D3 independently proves the PINK CAB effective ratio is negative in R and mirrored in magnitude against 1st.
- D4 retains the same >1 Nm wheel-torque presence threshold and the same >100 cm / <-100 cm directional-travel gates.


### D4 engine-brake onset gate
- Throttle lift first enters an explicit onset phase. The forward driveline must produce rear-wheel torque below -1 Nm within 0.25 s of measured mechanical time.
- The 1.00 s chassis-deceleration window starts on the first observed negative-torque mechanical state, not on the game-thread command write. This separates command propagation from the physical braking interval.
- Failure to produce negative torque within the onset bound is a hard D4 failure; no engine-brake coefficient, clutch parameter, speed threshold, or project timestep is modified.


### P01 slope fixture determinism hardening
- The engine-off neutral slope acceptance no longer teleports an already-live Chaos vehicle from the flat fixture or overwrites body velocity after physics creation.
- A fresh fixture pawn is spawned directly at the ramp transform with ignition Off / neutral / zero external drive torque.
- Measurement begins only after five consecutive mechanical observations with low velocity normal to the ramp, proving stable physical contact while leaving tangential downhill motion unconstrained.
- The original downhill travel, rolling-speed and wheel-rotation acceptance thresholds are unchanged.
