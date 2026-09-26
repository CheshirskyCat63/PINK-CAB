# PINK CAB — Vehicle Physics V2

Date: 2026-09-26  
Jira integration gate: CD-921 (child of CD-848)  
Confluence: page 22413517, "PINK CAB · Vehicle Physics V2 Calibration Program · 2026-09-26"  
Audited main at program creation: `7df0fc546e36141c2866a5f5028599eedd37c4a2`  
Latest owner-accepted road runtime at audit: R3 source `aa5d7c99c72b805e5eee8c7d45bc6d2390ebb752`, PR #22 squash `55e9fd8481a114265fd43f1c6433d86df53ba991`.

## Goal

Improve the base vehicle into a responsive, quick and convincing realistic simulation-arcade car without changing the accepted player-control grammar or gameplay mechanics.

This program covers engine-state correctness, warmed idle, drivetrain/clutch continuity, steering/pedal response, acceleration/gearing, mass/CoM/inertia, wheel-contact geometry, suspension, tires/brakes/wet RWD behavior, Vehicle Health/persistence, data-driven Tatra 613 / 603 Series 1 / 77 physical profiles and future state ports for windows/heater/electrical/thermal systems.

## Production authority

Native Unreal Engine 5.8.2 Chaos Vehicles behind `IPinkCabVehicleDynamicsProvider` is the only production road-dynamics solver. FGear/VDS are archived historical research only.

Control authority remains Confluence page 47 plus `docs/PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`.

## Confirmed current-code facts

On audited `main`, `FPinkCabChaosPhysicalProfile` contains a current observed profile around:

- 250 hp design target;
- 260 Nm max torque;
- 8500 rpm max;
- 750 rpm idle;
- 0.17 engine rev-up MOI;
- 3.2 final drive;
- 4.6 / 2.2 / 1.5 / 1.1 / 0.85 forward ratios;
- 4.6 reverse;
- 32.13 cm wheel radius / 20.5 cm width;
- 41 degree max steer;
- front/rear nominal friction-force multipliers 2.00 / 0.50;
- suspension travel seed roughly 160 mm front / 180 mm rear;
- ABS/traction-control disabled.

These are CURRENT_OBSERVED values, not automatically owner-accepted final tuning.

The new owner requirement is healthy warmed idle **900–950 rpm**. Use 925 rpm only as the first calibration center candidate.

## Engine-off invariant

A stopped/stalled engine must never create positive engine-authorized wheel drive torque.

This does **not** mean velocity must be forced to zero. The vehicle may physically coast, roll on grade, back-drive wheel/shaft rotation and react to contacts. The first job is to instrument the final torque/energy path and distinguish propulsion from inertia/gravity.

## Hard locks

Vehicle Physics V2 must preserve:

- mouse steering by default;
- Space hold = gaze/free-look;
- Q clutch, W brake, E throttle;
- Q+wheel adjusts clutch release timing, not instantaneous pressure;
- E+wheel re-doses throttle for every new launch from standstill;
- W+E coexist;
- wheel recipient priority E -> W -> Q;
- quick 1/2/3/4 prompts are ephemeral/key-held;
- RMB optional acquire/retain, no actuation by itself;
- LMB/wheel may execute authored contextual actions without RMB first;
- lever manipulation owns XY while steering holds current command; release returns XY to steering;
- H gate 1/3/5 top, 2/4/R bottom with usable neutral cross-gate and accepted enlarged throws;
- requested gear != engaged gear;
- clutch and handbrake remain continuous;
- no ABS/ESP/auto-countersteer/auto-rev-match/autothrottle/yaw rescue/lane hold/hidden trajectory correction;
- no second vehicle solver;
- no AddForce/AddTorque propulsion boost used to fake responsiveness.

## Execution order

P00 baseline/telemetry -> P01 engine/off/idle -> P02 drivetrain/clutch/gearbox -> P03 steering/pedals -> P04 acceleration/gearing -> P05 mass/CoM/inertia/wheels -> P06 suspension -> P07 tires/brakes/wet -> P08 health/persistence -> P09 three Tatra profiles -> P10 cabin-system readiness -> P11 integrated acceptance/admin.

No handling phase is accepted from documentation or automation alone. Owner feel gates remain mandatory.

## Files

- `AUDIT_AND_CONFLICTS.md` — live Git/Jira/Confluence findings and conflict resolution.
- `EXECUTION_PLAN_P00_P11.md` — detailed phase plan and gates.
- `PARAMETER_LEDGER.md` — current/locked/candidate/historical parameter provenance.
- `TASKS.csv` — 60 execution units mapped to current owners.
- `TESTS.csv` — 132 planned checks; all start NOT RUN.
- `ADMIN_AND_HANDOFF.md` — source-of-truth sync and executor rules.

Truth rule: CANON -> SPECIFIED -> IMPLEMENTED -> VERIFIED. A Jira/Confluence/Git text does not prove runtime behavior.
