# PINK CAB · Gate 1 Road Handling · CanonICAL INTEGRATION · 2026-09-28

**Administrative status:** INTEGRATED PLAN / runtime gate NOT ACCEPTED  
**Source:** `PINK-CAB_Gate-1_Road_Handling_v1.0.zip`  
**Source SHA-256:** `3eed0b832eedc7411971c0010c75ce2d5c9bfc9bc453d84fc9037dbf41c20f43`  
**Source package validation:** 20 tasks, 14 acceptance criteria, 12 test protocols; DAG/CSV/JSON/links/hash validation PASS. This is package integrity, not runtime evidence.  
**Jira execution umbrella:** CD-648  
**Terminal integration/evidence:** CD-921  
**Confluence Gate-1 canon:** page 24707073, child of physics program 22413538  
**Accepted road baseline:** R1–R5 HUMAN ACCEPTED / INTEGRATED / FROZEN  
**Open diagnostic candidate:** draft PR #47 / R6 tire calibration / HUMAN_PENDING

## Authority / dedup ruling

The uploaded Gate-1 package is a calibration + acceptance overlay on the existing P00–P11 / PHY-001..048 program. It does not create a second vehicle-physics architecture, solver, backlog, or Jira epic.

Local `G1-001..G1-020` IDs are retained for provenance and mapped to existing owners. No new Jira cards are required.

The current PINK CAB no-assist doctrine overrides the package's conditional-assist option. `G1-015` is therefore **NOT_NEEDED / PROHIBITED_BY_CURRENT_CANON**: no ABS/TC/ESP, auto-countersteer, yaw rescue, hidden steering/trajectory correction, hidden pedal/gear decision, snap, velocity overwrite or direct-force boost.

## Baseline / R6 ruling

R1–R5 remain the accepted road substrate. R6's automated green evidence is useful but does not close tire/handling acceptance.

The R6 candidate (`front=1.05`, `rear=0.95`, `WheelLoadRatio=1.0`) is treated as an A/B diagnostic point until the physical foundation is proved. Fine tire acceptance must not precede:

`road/contact → wheel/body geometry → mass/CoM/inertia → suspension/load transfer → tire combined grip/slip → drivetrain/brake response → handling acceptance`.

This prevents tire coefficients from masking contact, center-of-mass, inertia, suspension or road-seam defects.

## Canonical mapping

| Gate-1 | Canonical owner(s) | Disposition |
|---|---|---|
| G1-001 baseline/rollback | PHY-001 / CD-648 | reuse exact baseline + rollback; R6 is not accepted baseline |
| G1-002 scope/contract | CD-648 | A01–A14/T00–T11 are frozen as proposed conditions before tuning |
| G1-003 control path | PHY-013/014/016 / CD-649/CD-611 | accepted grammar frozen; speed may change sensitivity, not held target |
| G1-004 telemetry/repeatability | PHY-003/004 / CD-657 | reuse causal telemetry/fixtures; unavailable channels stay BLOCKED |
| G1-005 infinite-road disturbance isolation | CD-869 | accepted R1–R5 road; T07 real-vs-monolithic and seam correlation |
| G1-006 wheel/body contact | PHY-021/028 / CD-748/CD-855/CD-652 | prove centers/radii/clearance/contact before trace-mode change |
| G1-007 mass/CoM/inertia | PHY-022/023 / CD-748/CD-648 | measure runtime CoM/static loads; make inertia reproducible |
| G1-008 suspension | PHY-025..028 / CD-652 | derive from measured sprung load; verify sag/reserve/decay |
| G1-009 tire grip/slip | PHY-029 / CD-650 | continuous combined grip; no permanent low-rear-grip drift shortcut |
| G1-010 throttle/torque/gears | PHY-009..020 / CD-611/CD-643..645/CD-659 | causal torque + dosability; don't cure power delivery with low rear grip |
| G1-011 braking/lift | PHY-030 + PHY-026/029 / CD-656/CD-650/CD-652 | no ABS rescue; isolate brake/engine/load/tire cause |
| G1-012 straight stability | CD-658 + causal owner | acceptance only; no duplicate tuning owner |
| G1-013 lane change | CD-658 + CD-649/CD-650/CD-652 | T02/T03 acceptance; defects routed back to owner |
| G1-014 recovery envelope | CD-654/CD-658 | player-action recovery; committed error may spin |
| G1-015 conditional assistance | none | NOT_NEEDED / PROHIBITED_BY_CURRENT_CANON |
| G1-016 camera/readability | CD-652 presentation / CD-658 | camera cannot hide or change physical trajectory |
| G1-017 FPS/reset/long-run | CD-657/CD-658 / PHY-045 | editor/package separated; no state leak/config drift |
| G1-018 optional drift | CD-654 | optional/deferred; only after base handling acceptance |
| G1-019 human gate | CD-658 / PHY-046 | exact build/profile owner verdict |
| G1-020 freeze/handoff | CD-648 + CD-921 / PHY-047/048 | exact SHA/profile/evidence/rollback + source convergence |

Machine-readable mirror: `GATE1_OWNER_MAPPING_2026-09-28.csv`.

## Physical foundation

### Contact / curb

Do not assume a sweep mode is correct because flat-road driving works. First prove a contact defect. Then A/B Raycast/Spherecast/Shapecast one variable at a time and record wheel contact, hit normal, suspension position/velocity, chassis contact, speed loss and CPU cost.

The existing Gate-1 T06 smooth bump remains the standard suspension disturbance. A dedicated curb fixture is added only for representative Level-1 geometry and does not expand scope into off-road tuning.

### Mass / CoM / inertia

Record total mass, runtime CoM, per-wheel/axle static load and inertia source. Do not lower CoM or inflate yaw inertia merely to suppress oversteer. Small-steer, braking and lane-change traces must distinguish yaw, roll and pitch.

### Suspension

Use measured corner/axle load (or verified sprung-mass calculation) to derive a starting working point. Verify static sag, bump/droop reserve, single-bump decay, repeated-bump stability and load variants. Front/rear values may differ. Camera/presentation may communicate body work but cannot restore physical tire contact.

The package's 1.1–1.3 Hz value is a seed only, not accepted Tatra history or a mandatory production target.

### Tires / drift

Normal driving and drift use one continuous model:

high base grip → torque/load consumes combined grip → progressive rear breakaway → manual throttle/countersteer remains live → excessive demand can still produce a spin.

Do not create drift by permanently reducing rear grip. `WheelLoadRatio=1.0` remains the physical-reference A/B from the parent program, not an automatically accepted final value. The old 2.00/0.50 F/R shortcut is non-canonical unless evidence proves a vehicle-specific reason.

If `LateralSlipGraph` or equivalent curve authoring is required, first verify Chaos axis, units and evaluation semantics in the exact UE build. The audit's 7–10° reference is not a ready-made UE key set.

### Steering / throttle

Speed-dependent shaping may change mouse travel/sensitivity; vehicle speed alone must not shrink a held steering target. Full mechanically available countersteer remains accessible.

`pow(driver,0.55)` is an explicit pedal-dosability A/B after drivetrain continuity. Tire grip must not be reduced to compensate for an aggressive pedal transfer.

## Gate-1 acceptance import

`GATE1_ACCEPTANCE_PROTOCOLS_2026-09-28.md` imports A01–A14 and T00–T11. All are **PROPOSED / NOT RUN** until exact-candidate evidence exists.

Key discipline:

- quantitative conditions use the package's repeatability rules; INCONCLUSIVE is never PASS;
- critical single failures are not averaged away;
- T07 requires 10 minutes and >=100 road seams, extending beyond 10 minutes when required;
- editor and packaged results are distinct;
- optional drift can be DEFERRED without failing the base handling gate;
- HUMAN_ACCEPTED names an exact build/profile.

## Execution constraint

The existing P00–P11 sequence remains authoritative. Gate-1 constrains the handling slice:

1. preserve accepted/frozen P01;
2. P02 drivetrain energy continuity — **HUMAN ACCEPTED / INTEGRATED / FROZEN** on `8d68e456d1944be295281535cf9fd103ecf05d52`;
3. P03 steering/pedal transfer;
4. P05 contact/geometry + CoM/inertia evidence;
5. P06 suspension/load transfer;
6. P07 tire/brake calibration;
7. Gate-1 T01–T07/T09 objective acceptance;
8. T08 optional drift;
9. T10 owner HUMAN gate;
10. G1-020/P11 freeze, rollback and source convergence.

## ADMIN_CLOSED vs GATE1_ACCEPTED

**ADMIN_CLOSED** means the source package is fully dispositioned, all G1 work is mapped without duplicate backlog, conflicts are resolved, Git/Jira/Confluence agree, and next runtime ownership is unambiguous.

It does **not** mean **GATE1_ACCEPTED**. Runtime implementation, T00–T11, A01–A14 evidence and the owner handling gate remain future work.


## P02 prerequisite checkpoint · 2026-10-01

P02 / PHY-009..012 is **HUMAN ACCEPTED / INTEGRATED / FROZEN**. PR #49 merged to canonical `main@104295ab6329e85b5998e8df298770255ad2dd05`; accepted runtime HEAD is `8d68e456d1944be295281535cf9fd103ecf05d52`; exact-head runs `36868646398` and `36868646970` are SUCCESS.

The handling sequence therefore advances to P03 input-response calibration. R6 remains a separate **HUMAN_PENDING / diagnostic** tire candidate under CD-650; Gate-1 runtime acceptance remains open.
