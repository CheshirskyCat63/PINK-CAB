# PINK CAB · Control & Vehicle Mechanics Release Contract

**Status:** CURRENT OWNER CONTROL / VEHICLE-MECHANICS AUTHORITY MIRROR
**Date:** 2026-09-18 · recovery amendment 2026-09-20
**Jira umbrella:** `CD-848`
**Confluence authority:** page `16744449` — `47 · PINK CAB · Control & Vehicle Mechanics Release Contract`
**Runtime technology:** Unreal Engine 5.8.2 / native Chaos Vehicles
**Implementation state:** RECOVERY-R1 ACTIVE / ADMIN FREEZE / **AUTO GATE FAILED AT PACKAGED OS-INPUT / HUMAN NOT ACCEPTED**

This file mirrors the owner-approved 2026-09-18 release matrix into Git. It does not claim that the current executable already behaves this way.

> **CONTROL-PLANE STATUS · 2026-09-22**  
> Single active implementation lane: PR #7 `fix/CD-848-recovery-r1-input-contract` → `main`.  
> Last runtime-affecting candidate before admin-only cleanup: `8bed51e5c47e037eb45a670f9d6c1c2cdd28b712`; run `35746083559` passed code-health, build, focused/full automation, package and packaged smoke, then **FAILED packaged Windows OS-input throttle dosing**.  
> No current HUMAN_ACCEPTED vehicle build. Heavy CI is manual-only during cleanup; see `docs/CONTROL_PLANE.md`.

## 1. Owner control lock

| Input | Canonical meaning |
|---|---|
| Mouse | Steering by default |
| Hold Space | Gaze; releasing Space returns mouse to steering |
| Q | Clutch |
| Q + wheel | Adjust clutch release time/speed |
| E + wheel | Dose throttle target; **required again for every new launch from standstill** |
| W + wheel | Dose brake target |
| W + E | Throttle and brake may coexist |
| 1 / 2 / 3 / 4 | Quick recall: indicators / horn / gearbox / handbrake |
| RMB | Acquire/retain selected physical control with right hand |
| LMB | Perform the action authored by that control |
| Q/W/E overlap + wheel | Single recipient only, priority **E → W → Q** |

Hard rules:

- quick recall, gaze and grip never actuate the control by themselves;
- throttle-by-wheel on every new launch is a required mechanic, **not a defect**;
- the launch reset occurs exactly once per new launch event, not every frame and not from speed jitter during one held E event;
- no auto-throttle;
- no auto-rev-match;
- no auto-countersteer;
- no yaw rescue / rotational edge guard;
- no ABS or ESP;
- no hidden trajectory correction;
- focus loss/menu clears transient ownership/presses and cannot confirm a gear accidentally.

Older START→ATTENTION→GO wording is superseded where it conflicts with this contract.

## 1.1 Recovery input amendment · 2026-09-20

The later owner correction supersedes the old interpretation that RMB+XY alone manipulates Gearbox/Handbrake.

- RMB = optional universal acquire/retain for any valid selected physical control. RMB by itself never actuates that control.
- LMB and mouse wheel do **not** require RMB first: when the remembered/current target supports the gesture, they execute that contextual action directly.
- LMB on Gearbox/Handbrake may begin direct lever manipulation without RMB; RMB remains available when the driver wants to retain the target.
- While lever manipulation is active, mouse XY belongs exclusively to that lever and the current steering command is held.
- Releasing LMB ends lever manipulation and returns XY to steering. Releasing RMB ends only the optional retain.
- Quick recall `1/2/3/4` is visually key-held only: its prompt disappears immediately on key release even if the underlying remembered target remains available for tap-then-act.
- Gear request is produced by the physical H-gate path; actual engagement remains owned by the common validator. LMB/RMB release never performs a second commit.
- Q may stage Gearbox while clutch remains independent, but Q never grips/manipulates the lever and cannot steal an active grip.

Detailed executable contract: `docs/recovery/RECOVERY_INPUT_CONTRACT_R1.md`.

## 2. H-gate / gearbox topology

Canonical physical topology:

| Position | Left | Center | Right |
|---|---:|---:|---:|
| Forward / top | 1 | 3 | 5 |
| Center cross-gate | N | N | N |
| Back / bottom | 2 | 4 | R |

Rules:

- changes traverse the neutral cross-gate; no diagonal wall teleport;
- neutral is a usable corridor, not a pixel midpoint;
- reverse is right-bottom; the old protected left-forward secondary reverse plane is superseded;
- lever/request state and actually engaged gear are separate;
- all selection paths, including any retained wheel/ShiftBy path, use the same engagement validator;
- engagement depends on clutch state, transmitted load and speed mismatch rather than an unconditional target-gear write;
- low-load unloading to neutral may succeed without arbitrary damage;
- forcing a loaded/mismatched shift may resist, grind and accumulate wear;
- matched-speed clutchless engagement may remain possible as a difficult player skill;
- dangerous downshift damage occurs when the driveline actually connects into a dangerous mismatch;
- UI must distinguish requested vs actual gear truth.

## 3. Continuous controls

Steering, throttle, brake, clutch and handbrake are continuous states/commands.

- clutch torque transfer must be continuous; a binary neutral/gear cut at 50% clutch is not the target;
- Q+wheel adjusts the release-time setting, never instantaneous clutch pressure;
- another Q press interrupts release and disengages again;
- analog handbrake 0..1 must remain analog through the physics path;
- 25%, 50% and 100% handbrake pull must produce distinguishable braking;
- brake and throttle remain independently controllable and may coexist;
- wheel routing must never apply one detent to both pedal logic and a cockpit control.
- isolated wheel detents remain precise; sustained same-direction wheel input accelerates progressively, while a pause or direction reversal resets the burst response.

### Clutch numeric range

The old documentation says `0.20–1.20 s` / `>=64` settings. The inspected implementation uses a different range (`0.15–2.50 s` was observed in the 2026-09-18 analysis).

**Neither range is declared final owner lock by this document.** The mechanic is locked; the final numeric range is a calibration/reconciliation item and must be explicitly accepted before final release authority claims it.

## 4. Driving character

PINK CAB is city arcade-sim racing/taxi driving with mechanical skill, not a 1:1 historical Tatra simulation.

- Tatra supplies general mass/size/rear-engine/RWD identity.
- **BMW E34 535i** is the current gameplay capability/control reference.
- Older BMW E34 530i wording is superseded.
- Target feel: calm readable center, repeatable small corrections, deliberate fast countersteer, progressive grip breakaway/recovery, readable RWD oversteer, visible mass without endless boat oscillation.
- Control quality must come from input transfer, tires, suspension, load transfer and feedback — not hidden assists.

Current project power/terminal calibration remains 180 hp / 240 Nm / about 195 km/h until separately recalibrated and verified; the 535i reference does not silently import BMW factory powertrain numbers.

## 5. Causal mechanical consequences

Required cause → symptom → behavior → repair model:

1. insufficient throttle + clutch engagement → RPM drop / possible stall → normal restart, no scene reset;
2. prolonged clutch slip → heat → warning → reduced torque transfer → wear/damage;
3. high-RPM harsh clutch engagement → jerk/wheelspin/shock; repeated abuse accumulates wear;
4. braking to stop with clutch coupled may stall;
5. engine RPM against a held handbrake with clutch disengaged does not heat rear brakes by itself;
6. clutch slipping against a stationary handbrake heats the clutch;
7. rotating wheels forced through applied brakes heat/wear the relevant brakes;
8. short handbrake use may produce predictable rear slip; no scripted spin;
9. sustained handbrake use creates drag/heat/fade/wear;
10. no ABS: wheel lock remains possible and brake dosing is player skill;
11. overheated service brakes fade; cooling may recover reversible fade while permanent damage persists;
12. a correctly rev-matched downshift reduces shock; there is no automatic rev-match;
13. a genuinely dangerous connected downshift may overspeed/damage the drivetrain;
14. repeated gearbox conflict may worsen engagement of affected gears;
15. wheel/curb impacts may damage tire, wheel, alignment or suspension with concrete symptoms;
16. tire slip produces proportional heat/wear, not instant puncture for every drift;
17. existing door/glass/light/Vehicle Health consequences remain;
18. damage/wear persists through save/load; loading is not free repair;
19. immobilized vehicle uses the normal repair/recovery flow, not a free reset shortcut.

Balance target: a healthy car should survive **30 minutes of normal active urban driving without forced service**. This is a release acceptance target, not a measured PASS.

## 6. Implementation findings from the inspected CD-855 worktree

These are implementation tasks/findings, not runtime proof:

- explicit steering-sign inversion exists in the Chaos provider path; trace the full sign chain and fix only the wrong conversion;
- steering is explicitly **manual / no hydraulic or electric assist**: heavy and slow at standstill, lighter once rolling, but high-speed target gain and response must calm rather than increase; measure full transfer behavior and FPS sensitivity;
- mouse delta clamp/interpolation may lose or accumulate fast motion; verify at 30/60/120 FPS;
- per-launch throttle target reset is required; verify exactly one reset per new launch;
- current clutch bridge behaves like a binary threshold; replace with continuous torque transfer;
- current target gear propagation is too immediate; separate requested vs engaged state;
- retained ShiftBy/wheel shifting must not bypass the common engagement validator;
- current visual H-map already matches 1/3/5 over 2/4/R; preserve it;
- lever movement must obey H topology and neutral corridor;
- reverse uses the common validator, not a new button or old secondary-plane arm gesture;
- current handbrake bridge must not collapse analog magnitude to bool;
- stall state must be driven by actual drivetrain load and proven in runtime;
- Vehicle Health needs dedicated clutch and gearbox/transmission channels without deleting existing channels;
- save schema/versioning must preserve new drivetrain damage;
- clutch release numeric range remains an explicit reconciliation item.

## 7. Existing implementation owners

No duplicate control/mechanics Jira task is created. Work is distributed through existing owners:

- `CD-609` floor shifter / H-gate;
- `CD-611` Q/W/E wheel dosing and per-launch throttle reset;
- `CD-643` H-gate topology / engagement validation;
- `CD-644` clutch/load synchronization and deterministic engagement;
- `CD-645` transmission/control telemetry + regression;
- `CD-648` handling production pack;
- `CD-649` steering transfer/sign/FPS behavior;
- `CD-651` zero yaw-rescue verification;
- `CD-653` analog handbrake;
- `CD-659` continuous clutch release/launch behavior;
- `CD-670` persistence/reset matrix;
- `CD-740` Vehicle Health / clutch + gearbox damage persistence;
- `CD-848` broader Mechanics Freeze umbrella;
- `CD-855` hero-model/presentation consumer only;
- `CD-856` historical pre-model handoff evidence, DONE.

`CD-638` is closed as the owner/design lock for quick targets 1/2; this does not claim runtime implementation.

## 8. Release acceptance matrix

All checks below are **NOT RUN** until executable evidence is captured on an exact Git SHA.

| ID | Acceptance |
|---|---|
| QA-01 | Low-speed right/left steering produces correct physical wheel and trajectory direction |
| QA-02 | Reverse steering geometry is physically consistent; no sign flip |
| QA-03 | Same steering trace at 30/60/120 FPS stays within declared tolerance |
| QA-04 | 20 fast lane changes show no lost/accumulated steering input |
| QA-05 | 20 new launches each require E+wheel dosing |
| QA-06 | Speed jitter during one launch event does not repeatedly reset throttle |
| QA-07 | Q+E+wheel changes throttle only |
| QA-08 | W+E coexist; wheel has one clear recipient |
| QA-09 | Clutch release profile follows selected release timing |
| QA-10 | Half-clutch gives continuous torque transfer, no 50% N/gear jump |
| QA-11 | 1–5/N/R H-gate layout and neutral crossing are correct |
| QA-12 | 50 learned shifts do not get software-selected into a neighboring gear |
| QA-13 | No-clutch mismatch gives refusal/mechanical conflict, not free engagement |
| QA-14 | Cancel/focus-loss during a shift cannot cause delayed engagement |
| QA-15 | 25/50/100% handbrake produces distinct rear braking |
| QA-16 | Throttle vs handbrake correctly distinguishes stall, clutch heat and brake heat |
| QA-17 | Braking/slide/slippery surface has no hidden steering/ABS/ESP/yaw rescue |
| QA-18 | Stall and restart work without resetting vehicle/race state |
| QA-19 | Drivetrain damage persists through save/load with schema handling |
| QA-20 | 30-minute normal active-driving owner gate without forced repair |

Passing automated checks does not replace the owner's subjective handling gate.

## 9. Technology decision

Native Unreal Chaos Vehicles remains the production road-dynamics path behind the existing PINK CAB adapter boundary.

The 2026-09-18 package research did not establish any drop-in UE 5.8.2 replacement that preserves this exact input grammar, clutch, H-gate and damage model without integration work. Third-party packages may be studied as references/demos, but solver replacement is not the current task.

## 10. Administrative precedence

This contract supersedes contradictory older PINK CAB wording specifically for:

- throttle-wheel reset being treated as a defect;
- START→ATTENTION→GO interaction wording;
- protected left-forward reverse / wheel-down reverse arm;
- fixed `0.20–1.20 s` clutch-release range being presented as final owner lock;
- BMW E34 530i as current behavior reference;
- any C+/mild yaw-rescue allowance;
- blanket “accepted downshift can never overspeed” rules.

Historical build/package/test evidence remains historical evidence. It is not retroactively promoted to proof of these newer requirements.

Confluence durable authority:
https://cheshire-division.atlassian.net/wiki/spaces/TWC/pages/16744449/47+PINK+CAB+Control+Vehicle+Mechanics+Release+Contract
