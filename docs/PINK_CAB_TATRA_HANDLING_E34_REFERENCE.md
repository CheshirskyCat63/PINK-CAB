# PINK CAB · Tatra Handling — E34 Control Reference / Tatra Expression

**Status:** CURRENT HANDLING COMPANION · CONTROL/MECHANICS PRECEDENCE = `PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`
**Owner lock:** `CD-729`
**Vehicle technology:** native Unreal Chaos Vehicles / `CD-843`; implementation/calibration owners remain under `CD-648..659`
**Handling:** `CD-648`; detailed calibration/QA `CD-649..659`; drivetrain `CD-641..647`; physical systems `CD-722`
**Confluence:** handling `6225936`; acceptance `6160435`; Tatra/cockpit `6619137`; vehicle stack `10977288`; open decisions `5832744`

## 2026-09-18 owner correction

Current precedence: [`PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`](PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md) / Confluence [16744449](https://cheshire-division.atlassian.net/wiki/spaces/TWC/pages/16744449/47+PINK+CAB+Control+Vehicle+Mechanics+Release+Contract). It supersedes older 530i, fixed clutch-range, C+ yaw-guard, old reverse-plane and old interaction-grammar statements in this document. Existing calibration prose below survives only where non-conflicting.

## Authority rule

PINK CAB separates **how the car physically controls** from **how the old Tatra body communicates that control**.

- Road dynamics implementation = **FGear Vehicle Physics only**.
- Control/tire/steering behavior target = **BMW E34 535i-like**.
- Body/mass/presentation = bespoke rear-engined 1970s PINK CAB Tatra 603-family.
- Permanent mesh deformation = Debugging Delight Vehicle Damage System, not this handling layer.

No parallel second hero-Tatra road solver is permitted without an explicit migration decision.

## What E34-like means

Target: calm on-center steering; progressive response; predictable combined-grip build/release; readable/catchable RWD oversteer; straight-line composure; communicative steerable braking; natural settling; no Drift Mode; no snap-to-angle; no automatic countersteer.

Do not import BMW front-engine packaging, ABS, donor 218 PS/290 Nm, 235 km/h, exact BMW wheelbase/track/tire/brake/suspension hardware or cabin identity.

## Tatra physical identity retained

- rear-mounted air-cooled V8;
- RWD / rear-heavy;
- base 1450 kg / full-fuel 1550 kg;
- heroine 58 kg, daughter 49 kg, actual passenger/fuel mass;
- no ABS / no ESP;
- 180 hp / 240 Nm current product authority;
- 195 km/h flat-road terminal target;
- old-car physical control language.

## Closed calibration baseline

The latest 50-question Tatra pack is retired from owner intake. These values form the current implementation baseline; exact low-level plugin coefficients may be tuned only while the observable owner contract and acceptance remain green.

- static reference balance: **45/55 F/R @1657 kg**, only small final 3D-packaging correction allowed;
- historical chassis baseline: **~2750 mm wheelbase / ~1485 mm front track / ~1400 mm rear track**;
- tire visual family: **185R15-like tall sidewall**;
- inner-wheel steering lock: **41°±2°** unless final geometry causes kinematic collision;
- visible steering wheel: **~2.5 turns lock-to-lock**;
- mouse steering dead-zone: **none**, jitter filter only;
- speed-gain seed: **1.00 <=40 / ~0.72 @100 / ~0.52 @160 / ~0.42 @195**;
- useful sustained sideslip: **18–40°**;
- physical roll target: **~4.5–5.5° @~0.8g**;
- starting usable suspension travel: **~160 mm front / ~180 mm rear**;
- progressive bump stops: final **~25%** compression;
- nominal expression gains: roll **~1.35×**, pitch **~1.25×**, heave **~1.20×**;
- visual settling target after sharp lane change: **~0.9–1.2 s** with no growing oscillation;
- starting brake bias: **60/40 F/R**;
- differential: **mild LSD**, modest power lock / lighter coast lock;
- handbrake torque seed: **~1700 Nm per rear wheel**, progressive lever curve;
- clutch release timing: **player-adjustable continuous setting; final numeric range = RECONCILE / NOT OWNER-LOCKED**;
- headline performance/handling benchmark: **1657 kg full-fuel crew-only**, plus mandatory 2107 kg and intermediate tests.

Dry/wet/storm grip coefficients remain native Chaos/PINK CAB calibration seeds, not immutable owner law. Final tire curves are accepted by behavior/telemetry rather than by preserving arbitrary coefficient values.

## Power feel

Power is not the fantasy limiter. 0–100 and overtaking feel lively; 140 km/h is easy for healthy powertrain; throttle remains useful during high-speed travel/slide; 195 km/h is reachable without hidden velocity wall. High-speed difficulty comes from chassis/suspension/road/aero workload, not fake weak-engine behavior.

180 hp / 240 Nm headline peaks remain locked. Torque-curve shape may be tuned without silently changing those peaks.

## Speed character

- **0–100:** comfortable, lively, easy to place.
- **100–140:** confident, body motion increasingly visible.
- **140–170:** Tatra reads as a large soft old barge being pushed beyond suspension comfort; more float/roll/pitch/rut/pothole response and trim vibration.
- **170–195:** engine remains capable; driver manages chassis/road/rear-heavy balance. Car may look busy but must not become randomly unstable.

## Steering

Steering is continuous, degree-readable and player-owned. Road-wheel mapping is near-linear/mechanical; mouse command uses smooth speed-sensitive gain. No center dead-zone beyond noise filtering. High-speed gain reduction does not remove mechanical steering authority. 140–195 straight-line driving must not demand nervous micro-correction.

## Tires / grip / wet high-speed failure

One continuous native Chaos/PINK CAB tire model family covers `GRIP → SLIDE → DEEP SLIDE → SPIN` with no hidden mode swap.

Required wet chain:

`RAIN + RAPID LANE CHANGE >~160 + MAINTAINED/EXCESS THROTTLE → REAR COMBINED-GRIP SATURATION → PROGRESSIVE REAR SLIP → POSSIBLE SPIN`

`EASE EXCESS THROTTLE → REAR LATERAL RESERVE RETURNS → PLAYER STEERING RECOVERS / STRAIGHTENS`

No `if speed > 160 then drift`, no positive drift propulsion, no gear/clutch yaw reset, no automatic countersteer/ESP recovery. Full abrupt lift may create ordinary forward load transfer, but controlled throttle reduction must be the normal recovery aid.

Post-peak lateral force must remain progressive rather than cliff-like. Useful normal sustained drift is 18–40°; deeper angles are recovery/spin edge, not a second physics mode. Tuning priority is **hold/recovery/readability before effortless initiation**.

## Suspension / Tatra Expression

Native Chaos/PINK CAB vehicle dynamics owns physical suspension forces, tire loads, actual body state and trajectory. Tatra Expression is presentation-only:

`SURFACE → Chaos wheel/suspension → PHYSICAL CHASSIS → EXPRESSION ROLL/PITCH/HEAVE/SHAKE → STEERING/CABIN/CAMERA/AUDIO/VFX`

Physical suspension targets comfortable old-luxury compliance, not copied BMW spring rates. Visible body motion may be modestly amplified through the baseline gains above. Expression changes may never modify tire force, collision impulse or road-wheel command.

Ruts can moderately influence steering physically and read stronger through body/steering tremor. Large potholes may disturb wheel load/yaw; ordinary defects primarily communicate through coherent suspension/body/steering/audio rather than random scripted heading change.

Camera shake is bounded/accessibility-scalable and never the primary physics cue.

## Brakes / handbrake

Brake command is continuous 0–100%. No ABS. Full/near-full dry demand may lock front tires; controlled braking remains modulatable. Starting brake balance is 60/40 F/R; exact final torque split is native Chaos/PINK CAB profile calibration.

Old 100→0 references 38–42 m dry / 48–55 m wet are acceptance references only and must be re-measured at the 1657 kg benchmark plus heavier fixtures.

Handbrake is physical analog 0–100%, not drift button. Curve is progressive: softer beginning, near-linear middle, stronger final segment. ~1700 Nm/rear-wheel remains a tuning seed.

## Drivetrain / clutch

Current PINK CAB five-speed ratios remain the baseline; BMW ratios are not imported. Engine braking is moderate: enough for readable load transfer, not so strong that normal lift creates a required snap-oversteer characteristic.

Clutch state is continuous. Contextual mouse wheel adjusts **release-speed setting**, not instantaneous clutch pressure. The final numeric clutch-release range is **RECONCILE / NOT OWNER-LOCKED** after the 2026-09-18 correction; preserve continuous player-adjustable release timing. Very low throttle + fast engagement may stall; normal urban throttle + moderate release should be forgiving. No hidden auto-throttle/launch control.

## Rear-heavy load model

45/55 F/R at 1657 kg is the reference target; final 3D packaging may make a small nearby correction but may not silently replace the rear-heavy identity. Fuel and occupants physically shift CG once implemented.

More mass modestly worsens acceleration/braking, increases roll/pitch/settling/expression and gives only a modest single-digit-percent direction of improved wallride abrupt-separation resistance.

Level1 residual magnetism is **LOCKED LINEAR**:

- `m <= 1657 kg` → 5.0 s cap;
- `1657 < m < 2107 kg` → `timeout_s = 5.0 - (m - 1657)/450`;
- `m = 2107 kg` → 4.0 s.

## Mechanical health boundary

Damage/health is not solved inside the tire model. VDS supplies permanent deformation; Vehicle Health maps authored hit-zone/thermal/wear state to permitted Chaos/gameplay consequences.

Air-cooled V8 thermal authority uses oil/head temperature + fan/oil-cooler/airflow health. Generic coolant/radiator wording is superseded. Brake health includes heat/fade and hydraulic/brake-fluid degradation; no ABS rescue.

## Assistance boundary

**No C+ yaw edge guard / yaw rescue is allowed.** There is no non-diegetic rotational recovery layer. Deliberate spin and failed recovery remain possible.

## Telemetry

Record at minimum: total mass/load fixture; speed; raw/filtered steering, gain and road-wheel angles; throttle/brake/clutch/handbrake; clutch release speed; gear/rpm/requested/delivered torque; per-wheel normal load, slip ratio/angle and tire forces; yaw/sideslip; suspension travel/velocity; physical roll/pitch/heave; expression roll/pitch/heave/shake separately; surface event; camera/cabin/trim channels; explicit zero yaw-rescue contribution; `ABS=false`, `ESP=false`; Chaos/Tatra profile version.

## Calibration, not owner-open

Exact Chaos/PINK CAB tire coefficients, spring/damper/ARB constants, low-level steering filter coefficients, brake torque, LSD lock coefficients, detailed expression frequency/amplitude, physical CG XYZ from final model and similar plugin parameters are implementation calibration. They may change only while the locked observable behavior and QA remain green.

A change to 45/55 target identity, 41°±2° lock, 18–40° useful drift intent, 60/40 brake-bias starting contract, 180/240 power peaks, no-ABS/ESP/no-yaw-rescue rule, wet recovery chain, or 1657 benchmark requires an explicit decision update rather than silent tuning. The final clutch-release numeric range is intentionally excluded from this locked list pending reconciliation.

## QA / maturity

Executable acceptance: `docs/qa/PINK_CAB_TATRA_HANDLING_ACCEPTANCE.md`; integrated stack `docs/qa/PINK_CAB_VEHICLE_STACK_ACCEPTANCE.md`; mass/vertical `CD-701`.

Documentation establishes SPECIFIED. Runtime remains **NOT IMPLEMENTED / NOT VERIFIED** until exact-build evidence passes `CD-738` and subordinate tests.
