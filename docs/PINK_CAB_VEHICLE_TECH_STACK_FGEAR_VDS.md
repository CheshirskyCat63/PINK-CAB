# PINK CAB · Vehicle Technology Stack — FGear + Vehicle Damage System

**Status:** ARCHIVED / SUPERSEDED — historical FGear/VDS plan only
**Jira:** `CD-730`; integration `CD-731..738`; damage state `CD-740`; handling `CD-729`; damage decision `CD-600`
**Confluence:** `10977288` — `38 · PINK CAB · Vehicle Technology Stack, FGear/VDS & Tatra Calibration Contract`
**Superseded by:** `docs/PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md` / Confluence `13303842` / `CD-785..792`

> **Archive rule (CD-843, 2026-09-16):** the sections below preserve historical evaluation/calibration context only. They are not current production authority and must not be used to require FGear/VDS, vendor purchase, or a non-Chaos runtime path.

## 1. Historical stack boundary

- **FGear Vehicle Physics** is the sole road-dynamics owner for the hero Tatra: tires/contact, steering response, drivetrain/transmission integration, suspension-force state, braking/handbrake behavior and plugin telemetry.
- **Vehicle Damage System (Debugging Delight Studios)** is the sole runtime permanent body-mesh deformation owner.
- PINK CAB code is limited to input adaptation, versioned Tatra parameters, Tatra Expression Layer, Vehicle Health bridge, persistence and normalized telemetry.
- Do not run a second Chaos/custom vehicle solver in parallel with FGear for the hero Tatra.
- Do not write a second mesh-deformation solver in parallel with VDS.
- VDS deformation does not alter handling unless an authored hit-zone consequence explicitly maps damage into a vehicle-health/FGear parameter change.

## 2. Locked physical identity

- bespoke early/Gen-1 Tatra 603-family retrofuturist taxi;
- rear-mounted air-cooled V8;
- RWD;
- 1450 kg base / 1550 kg full fuel;
- 58 kg heroine / 49 kg daughter;
- female passenger 55–70 kg / male passenger 65–90 kg actual authored mass;
- 1657 kg full-fuel crew-only reference fixture;
- 2107 kg declared current max five-passenger fixture;
- no ABS;
- no ESP;
- current engine target 180 hp / 240 Nm;
- terminal target 195 km/h;
- current five-speed PINK CAB ratios remain authoritative baseline;
- historical geometry baseline: ~2750 mm wheelbase, ~1485 mm front track, ~1400 mm rear track, tall 185R15-like tire visual family;
- body/tail stretch is bespoke art-sheet work and must not uniformly rescale the whole chassis.

## 3. Handling owner contract

Control layer target: **E34-like calm/progressive steering, predictable combined grip and readable/catchable RWD oversteer**.

Physical identity: **rear-heavy old Tatra**.

Presentation identity: **old soft barge that visibly works harder than the underlying control layer**.

Speed bands:

- 0–100 km/h: lively and easy;
- 100–140: confident; body motion increasingly visible;
- 140–170: clear soft-suspension/barge workload;
- 170–195: engine still capable; chassis/road/suspension management is the challenge.

Wet high-speed chain:

`WET + RAPID LANE CHANGE ABOVE ~160 + MAINTAINED/EXCESS THROTTLE -> REAR COMBINED-GRIP SATURATION -> PROGRESSIVE REAR SLIP -> POSSIBLE SPIN`

`REDUCE THROTTLE -> REAR LATERAL RESERVE RECOVERS -> DRIVER STEERING CAN SETTLE/STRAIGHTEN`

No scripted `speed > 160 => drift` switch is allowed.

## 4. Closed Tatra technical choices

The previous 50 owner questions are retired. Remaining exact low-level values are FGear/VDS **calibration**, not owner-design unknowns.

### Geometry / CG

1. Static balance target: **45/55 F/R at 1657 kg**; only small packaging-driven correction allowed.
2. CG XYZ is derived from final packaging and the balance target; no unrelated invented fixed point.
3. Fuel/occupant sockets physically shift CG once implemented.
4. Chassis baseline: **2750 mm wheelbase / 1485 mm front track / 1400 mm rear track**; bespoke rear overhang remains `CD-561` model-sheet work.
5. Tire visual family: **185R15-like tall sidewall**; force curve is tuned separately.
6. Sidewall visual compliance is presentation-only; no tire soft-body solver.

### Steering / grip / drift

7. Inner-wheel lock target: **41° ±2°** unless final geometry invalidates it.
8. Physical steering-wheel animation target: **~2.5 turns lock-to-lock**.
9. Road-wheel mapping near-linear/mechanical; mouse gain speed-sensitive and smooth.
10. Gain baseline: **1.00 <=40 / ~0.72 @100 / ~0.52 @160 / ~0.42 @195**.
11. Mouse center dead-zone: **none**; tiny jitter filtering only.
12. Dry/wet/storm grip seeds **~1.02 / ~0.86 / ~0.80** are tuning seeds, not immutable law.
13. Post-peak grip is progressive; retain ~75–85% peak around 20° slip as seed.
14. Useful sustained sideslip: **18–40°**.
15. Priority: **hold/recovery/readability > effortless initiation**.

### Suspension / expression

16. Physical suspension is tuned for comfortable old luxury-sedan compliance; E34 is not a spring-rate donor.
17. Starting travel: **160 mm front / 180 mm rear total usable**; final values follow clearance/tuning.
18. Progressive bump-stop region: final **~25%** of compression.
19. Acceleration squat is physically modest and visually clear.
20. Physical roll target: **~4.5–5.5° @ ~0.8 g**.
21. Visible roll amplification baseline: **1.35×**, calibration range roughly 1.20–1.50×.
22. Separate expression gains: roll ~1.35×, pitch ~1.25×, heave ~1.20× starting direction.
23. Lane-change visual settle: **~0.9–1.2 s**, no growing oscillation.
24. Camera shake is bounded/accessibility-scalable and never the main physics cue.
25. Ruts exert moderate physical steering influence plus stronger body/wheel tremor.
26. Large potholes may physically perturb wheel load/yaw; ordinary defects mostly communicate through suspension/body/steering/audio.

### Brakes / handbrake

27. Brake-bias starting target: **60/40 front/rear**.
28. Full/near-full dry brake demand may lock fronts; no ABS rescue.
29. Old 38–42 m dry / 48–55 m wet stopping bands are calibration references; final measurement is repeated at declared load fixtures.
30. Handbrake curve is progressive: soft beginning, near-linear middle, stronger end.
31. Handbrake starting torque: **~1700 Nm per rear wheel**, modest retune allowed to meet acceptance.

### Engine / drivetrain / clutch

32. **180 hp / 240 Nm remains product authority**; torque-curve shape may be tuned without changing headline peaks.
33. Engine braking is moderate; enough for load transfer, not enough for ordinary lift-off snap.
34. Differential: **mild LSD**; modest power locking, lighter coast locking.
35. Current PINK CAB five-speed ratios remain baseline; BMW ratios are not imported.
36. Clutch-release-speed envelope: **0.20 s fastest -> 1.20 s slowest**.
37. Release-speed command is continuous/smooth; target **>=64 addressable steps** from mouse-wheel interaction.
38. Stall is forgiving but real: very low throttle + rapid clutch engagement may stall; no hidden auto-throttle.
39. Air-cooled V8 vibration is readable but comfortable; cabin drama is mostly expression-layer output.

### Aero / load / wallride

40. No scripted float mode; natural aero/steering/suspension tuning makes 140 busy, 170 clearly old-chassis, 195 demanding but controllable.
41. Crosswind/freight wake: **enabled, bounded, deterministic**.
42. Mass effect on acceleration/braking is smooth/near-linear; roll/settling expression may become mildly convex near 1900–2107 kg.
43. Heavier wallride contact gets only a **modest single-digit-percent** reduction in abrupt-separation tendency.
44. Level-1 residual magnet interpolation is **linear by total mass** between **5.0 s @1657 kg** and **4.0 s @2107 kg**; lighter states cap at 5.0 s.

### Mechanical health / benchmark

45. Air-cooled thermal model uses oil/head temperature + fan/oil-cooler/airflow health; no coolant subsystem.
46. Brake-fluid/hydraulic degradation = softer/longer effective response + reduced available braking; asymmetric pull only from asymmetric damage.
47. Cabin rattles become clearly audible from 140+ and over defects but never mask engine/tire/safety cues.
48. Separate shake targets may include rear-view mirror, steering wheel/column, taximeter/receipt hardware, passenger-door hardware, seats, daughter work surface and selected trim; gauges stay legible.
49. Standard benchmark fixture: **1657 kg full-fuel crew-only**; 2107 kg and one intermediate load are mandatory secondary fixtures.
50. If tuning tradeoffs appear, priority is **calm straight-line placement + predictable rear-breakaway/recovery**, then controllable drift, braking confidence, then aggressive turn-in.

## 5. FIRST EURO damage breadth

### ON

- permanent VDS body deformation;
- glass cracked/broken state;
- exterior lamp damage;
- passenger-door latch/jam/failure;
- tire/wheel condition and puncture path where exposed through FGear/adapter;
- steering/alignment offset from authored wheel/front-corner hit zones;
- suspension degradation from authored hit zones;
- brake heat/fade and hydraulic/brake-fluid health;
- air-cooled V8 oil/head/fan/oil-cooler/airflow thermal health;
- explicit repair/service and persistence.

### OFF BY DEFAULT / LATER

- full soft-body structural chassis fracture;
- detailed individual engine internals;
- CFD/fluid volumes;
- detailed tire carcass thermodynamics;
- catastrophic fuel-fire/explosion chain;
- centimeter-level brake-line plumbing;
- occupant injury simulation.

## 6. Runtime architecture

`PLAYER INPUT -> PINK CAB INPUT ADAPTER -> FGEAR -> AUTHORITATIVE VEHICLE STATE`

`AUTHORITATIVE VEHICLE STATE -> TATRA EXPRESSION LAYER -> BODY/CABIN/CAMERA/AUDIO/VFX`

`COLLISION -> VDS DEFORMATION + HIT ZONE -> OPTIONAL VEHICLE HEALTH CONSEQUENCE -> FGEAR PARAMETER/STATE ADAPTER`

The expression/deformation layers may never silently write steering, throttle, brake or tire force.

## 7. Delivery map

- `CD-731` plugin pin/version integration;
- `CD-732` authoritative FGear Tatra profile;
- `CD-733` analog control adapter;
- `CD-734` wet/high-speed tire/surface calibration;
- `CD-735` Tatra Expression Layer;
- `CD-736` VDS deformation integration;
- `CD-737` functional damage bridge;
- `CD-738` integrated vehicle acceptance;
- `CD-740` damage state/save schema.

Existing `CD-649..659` remain subsystem calibration/acceptance owners implemented through this stack.

## 8. Truth rule

This document locks technology and calibration intent. It does **not** prove plugin integration or runtime handling. `CD-738` executable evidence is required for VERIFIED.
