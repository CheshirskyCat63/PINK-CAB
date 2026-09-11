# PINK CAB · FGear + Vehicle Damage System Integrated Acceptance

**Status:** SPECIFIED / RUNTIME NOT VERIFIED
**Jira:** `CD-738`
**Authority:** `CD-730`, `CD-729`, `CD-600`, Confluence `10977288`

## Purpose

Prove the chosen vehicle technology stack without allowing documentation or plugin installation to masquerade as verified handling.

## Stack-integrity tests

### PC-VSTACK-001 — sole dynamics owner

FGear is the only hero-Tatra road dynamics solver. No parallel Chaos/custom tire, suspension-force or drivetrain solver contributes force/torque.

### PC-VSTACK-002 — sole deformation owner

Debugging Delight Vehicle Damage System is the only permanent runtime body mesh deformation system. No second custom deformation solver is active.

### PC-VSTACK-003 — version evidence

Every run records UE version, FGear version/package identifier, VDS version/package identifier, build/commit and active Tatra physics/damage profile versions.

## Handling tests

### PC-T-WET-160-001 — progressive rear slip

At wet surface profile and declared load, a reproducible rapid lane change above ~160 km/h under maintained/excess throttle may progressively saturate rear combined grip and increase sideslip/yaw. No discrete force/tire-mode switch occurs exactly at 160 km/h.

### PC-T-WET-160-002 — throttle reduction recovery

Replay the same entry but reduce throttle after rear slip begins. Rear longitudinal demand decreases, lateral reserve recovers and the player can settle/straighten with steering. No auto-countersteer or ESP-style individual-wheel braking is permitted.

### PC-T-WET-160-003 — maintained throttle spin path

Maintain/exceed throttle through the same rear-slip event. A committed spin remains possible. No yaw clamp prevents it.

### PC-T-STR-STACK-001

Steering remains calm around center at 100/140/170/195 km/h while deliberate large input can still reach the physical steering envelope.

### PC-T-ANALOG-001

Prove reproducible intermediate steering, throttle, brake and handbrake commands, plus at least 64 addressable clutch-release-speed targets over the 0.20–1.20 s envelope.

## Suspension / expression tests

### PC-T-EXP-001 — separation of authority

Log physical FGear chassis roll/pitch/heave separately from Tatra expression roll/pitch/heave/shake. Expression may exceed physical values but may not change tire forces or collision outcome.

### PC-T-EXP-002 — speed progression

At 100/140/170/195 km/h on identical road/load fixtures, presentation workload increases coherently while physical chassis stability remains bounded.

### PC-T-EXP-003 — road defect chain

For rut/pothole/patch fixture, demonstrate one coherent event chain:

`surface -> wheel/suspension -> physical chassis -> steering tremor -> expression/camera/cabin -> audio/VFX`

No random camera-only shake qualifies.

## Mass/load tests

### PC-T-MASS-001

1657 kg full-fuel crew-only is the standard benchmark fixture.

### PC-T-MASS-002

2107 kg declared max-load fixture and at least one intermediate fixture are tested against the same commands.

### PC-T-MASS-003

More mass modestly worsens acceleration/braking, increases roll/pitch/settling and does not switch to a different hidden vehicle preset.

### PC-T-WALL-LOAD-001

At matched wallride entry, 2107 kg should show only a modest single-digit-percent improvement in abrupt-separation resistance relative to 1657 kg; it must not become a separate easy mode.

### PC-T-MAG-LOAD-001

Residual magnet timeout is linear by total mass between 5.0 s at 1657 kg and 4.0 s at 2107 kg; legal lighter states cap at 5.0 s.

## Brakes / drivetrain tests

### PC-T-BRK-ABS-001

ABS=false. Near-full dry brake demand can lock wheels. No hidden ABS pulsing or ESP correction.

### PC-T-BRK-BIAS-001

Start with 60/40 front/rear bias and record final calibrated profile. Final acceptance uses declared load fixtures rather than one load-independent stopping-distance number.

### PC-T-HB-ANALOG-001

0/25/50/75/100% handbrake commands yield ordered rear-brake effects. The handbrake is not a binary drift mode.

### PC-T-CLUTCH-001

Clutch-release-speed targets from 0.20 s through 1.20 s are monotonic, reproducible and do not directly set instantaneous clutch pressure.

### PC-T-LSD-001

Mild LSD profile provides readable power-on rear behavior and lighter coast locking; no welded-diff snap behavior.

## Damage/deformation tests

### PC-T-DMG-001 — collision deformation

A declared collision produces permanent VDS deformation. Deformation persists according to save policy.

### PC-T-DMG-002 — cosmetic damage isolation

A cosmetic body dent with no authored functional hit-zone mapping does not change FGear handling values.

### PC-T-DMG-003 — authored wheel-corner consequence

A configured wheel/front-corner hit may create alignment/steering-center or suspension-effectiveness change through the Vehicle Health bridge. The effect is deterministic and telemetered.

### PC-T-DMG-004 — door jam

A configured passenger-door hit can create explicit jam/latch failure without silently corrupting fare/passenger state machine.

### PC-T-DMG-005 — air-cooled thermal damage

A rear-engine-bay/fan/oil-cooler health fixture affects oil/head thermal state and produces diegetic warning + bounded power symptom. No coolant/radiator state is created.

### PC-T-DMG-006 — brake health

Brake heat/fade or hydraulic/brake-fluid degradation produces a measurable braking symptom. Asymmetric vehicle pull occurs only from an explicitly asymmetric fault.

### PC-T-DMG-007 — repair

Service/repair explicitly transforms or clears the relevant versioned damage/health state. No hidden full-health reset occurs outside authority.

## Persistence / determinism

### PC-T-DMG-SAVE-001

Damage, deformation reference state, passenger load, fuel load, thermal/brake/alignment state and profile versions reconstruct without reroll or duplicate application.

### PC-T-DET-001

Identical seeded input traces on the same build/profile reproduce discrete outcomes and continuous channels within declared tolerances.

### PC-T-SOAK-001

30-minute mixed drive at road/defect/wet/drift/brake/damage/load transitions shows no NaN, growing oscillation, stuck input owner, duplicated damage event, physics/deformation desync or monotonic resource leak.

## Evidence bundle

Every accepted run stores:

- commit/build;
- UE version;
- FGear/VDS versions;
- physics/tire/surface/expression/damage profile ids;
- total mass/fuel/passenger identities;
- speed, steering, pedals, clutch release-speed, handbrake;
- wheel loads/slip/forces where available;
- yaw/sideslip;
- physical chassis and expression channels separately;
- damage hit-zone/event ids;
- vehicle-health state before/after;
- FPS cap and artifact paths.

## Truth rule

Passing documentation review is not runtime verification. `VERIFIED` requires executable evidence from the exact integrated build.
