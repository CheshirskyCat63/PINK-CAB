# PINK CAB Vehicle Physics V2 — live audit and conflicts

## Git / runtime baseline

Canonical repository: `CheshirskyCat63/PINK-CAB`.  
Canonical `main` at audit start: `7df0fc546e36141c2866a5f5028599eedd37c4a2`.  
Latest owner-accepted road runtime remains R3 source `aa5d7c99c72b805e5eee8c7d45bc6d2390ebb752`; calibration/bake run `36094789710`; final HUMAN delivery run `36095423828`; PR #22 squash `55e9fd8481a114265fd43f1c6433d86df53ba991`.

R1/R2/R3 are accepted/frozen. R4 road/world branches are separate and may not be used to reopen accepted vehicle controls.

This audit used the remote GitHub technical source. A remote connector cannot prove the developer machine's local uncommitted state, so **P00 implementation preflight still requires local root/branch/HEAD/remote/upstream/status/untracked/active-PR checks before any code edit**. This planning pass did not edit a local source worktree.

## Current source findings

### Physical profile

`Source/PinkCabVehicle/Private/Vehicle/PinkCabChaosPhysicalProfile.cpp` and its public profile show current observed 250 hp / 260 Nm / 8500 max rpm / 750 idle rpm, current gearing and the low rear-friction nominal value 0.50.

The rear friction value is explicitly exercised by current tests as an easy-wheelspin/power-oversteer characteristic. Vehicle Physics V2 must test whether that is a coherent tire calibration or a shortcut compensating for drivetrain/chassis tuning.

### Engine-off force path

`FPinkCabChaosCockpitBridge` currently:

1. considers the engine running only when ignition state is Running;
2. enables Chaos mechanical simulation only for Running;
3. exposes a mechanical gear to Chaos only at fully coupled clutch;
4. calculates partial-clutch external rear-wheel torque only if engine is Running, gear != N and coupling is between open/full;
5. sets external rear-wheel torque to zero otherwise.

`FPinkCabVehicleControlRuntime` may retain semantic pedal command state while Off/Stalled. That is not automatically a physics bug. The invariant is that semantic input cannot produce positive engine-authorized propulsion while the engine is not Running.

Therefore, an observed moving car with engine off must be separated into:

- positive combustion/engine drive torque;
- existing chassis velocity/coast;
- grade/gravity;
- wheel/shaft rotational inertia/back-drive;
- collision/contact impulse;
- stale duplicated external torque/force.

Never "fix" this report by zeroing velocity.

### Steering and pedal response

The steering controller already exposes independent center curve, standstill response, rolling response and high-speed gain. The pawn also has separate throttle/brake/clutch response times. These are legitimate calibration surfaces: responsiveness can improve without changing mouse/Q/W/E/Space mechanics.

### Mass/profile architecture

The runtime still effectively has one main Tatra physical profile/mass contract. Current load bridge behavior is not yet a clean three-model profile system, and current center-of-mass handling is narrower than the desired model-specific XYZ/inertia contract.

## Jira audit

New integration gate: **CD-921**, Subtask under **CD-848**, status BACKLOG.

CD-921 does not replace subsystem owners. It coordinates existing tasks:

- CD-641 engine/capability;
- CD-642 old gear/final-drive numeric contract;
- CD-643 H-gate/engagement;
- CD-644 clutch/load;
- CD-645 drivetrain telemetry/regression;
- CD-648 handling pack;
- CD-649 steering;
- CD-650 tire/wet;
- CD-652 suspension;
- CD-653 analog handbrake;
- CD-654 drift/recovery;
- CD-655 drivetrain continuity;
- CD-656 braking/no ABS;
- CD-657 telemetry;
- CD-658 acceptance/soak;
- CD-659 clutch/launch;
- CD-670 persistence/reset;
- CD-722 physical systems/failure symptoms;
- CD-740 Vehicle Health/save;
- CD-855 model/presentation consumer.

CD-843 is the completed Native Chaos authority cleanup. CD-562 and CD-732 are superseded FGear-era evidence, not current physics owners.

Important status rule: historical text such as "AUTOMATED VERIFIED @ fe52868; UNDER REVIEW" inside a BACKLOG issue is evidence metadata, not current DONE state.

## Confluence audit / repairs

Created page **22413517 — PINK CAB · Vehicle Physics V2 Calibration Program · 2026-09-26**.

Conflicts found and repaired:

### C01 — idle
Old code current observed: 750. Old drivetrain page: 850. New owner requirement: healthy warmed **900–950**. Result: 900–950 becomes the acceptance range; 925 is only a first candidate.

### C02 — engine power/RPM
Old durable surfaces retained 180 hp / 240 Nm / ~6200-era limits while current audited code/tests use 250 / 260 / 8500. Result: do not average. Record provenance, A/B and accept one versioned gameplay profile in P04.

### C03 — RMB/LMB
Pages 26/27/28 still carried mandatory RMB+LMB or old START->ATTENTION->GO semantics. Current page47 says RMB is optional retain and LMB/wheel may act contextually without RMB. Result: active Confluence authority blocks were corrected to page47 semantics.

### C04 — hidden trajectory assist
Page26 described marked-lane/current-trajectory continuity. That contradicts no hidden trajectory correction. Result: active section replaced with "hold current steering command only while ownership is elsewhere"; no lane/yaw rescue.

### C05 — clutch range
Old 0.20–1.20 s / >=64 wording was still presented as locked in old surfaces, while current page47/CD-659 require numeric reconciliation. Result: mechanic remains locked, numeric range is P02 calibration/reconciliation.

### C06 — FGear/VDS
Some active Jira wording still says FGear/VDS owns road dynamics. Result: CD-843 / Confluence 13303842 wins; CD-722 relation/comment now explicitly records current Chaos precedence.

### C07 — hero identity vs profiles
Current durable canon is still a bespoke early/Gen-1 603-family PINK CAB hero. Tatra 613 is a donor/presentation source. Result: Vehicle Physics V2 supports 613/603 Series 1/77 physical profiles without silently changing hero canon.

### C08 — R4 isolation
World/MetaRoad R4 work is a separate lane. Vehicle Physics V2 must never be implemented by editing an R4 branch merely because it is active.

## Current administration state

Confluence page26 semantics and no-assist text are reconciled. Page27 old numeric sections are explicitly historical/reference pending P00–P04. Page28 points execution to CD-921/current runtime provenance. Page47 and Product Family / Authority Index register the new program without reopening accepted controls.
