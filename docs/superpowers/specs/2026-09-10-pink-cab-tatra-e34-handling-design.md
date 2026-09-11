# PINK CAB · Tatra E34-Reference Handling & Physical Feedback Design

**Status:** OWNER-APPROVED DESIGN — IMPLEMENTATION/ADMIN SWEEP AUTHORITY CANDIDATE
**Date:** 2026-09-10
**Product:** PINK CAB
**Primary Jira owners:** `CD-562`, `CD-648..659`; drivetrain `CD-641..647`; mass/contact `CD-592`; physical vehicle systems `CD-722`; Level 1 vertical QA `CD-701`.

## 1. Owner intent

The hero Tatra must **drive with the confidence, steering readability and controllable rear-drive balance associated with a well-sorted BMW E34**, while **looking, moving and communicating load like an old rear-engined 1970s Tatra barge**.

The approved external handling benchmark is the **BMW E34 530i V8 five-speed manual (1992–1995 family)**. This is a behavioral/chassis-response reference, not permission to transplant the donor car's front-engine packaging, ABS, engine output, final-drive geometry or body dimensions into PINK CAB.

The player should think:

> this old Tatra looks like its suspension was never meant to be driven this hard, but the steering itself is calm, predictable and genuinely good.

## 2. External donor provenance — BMW E34 530i V8 manual

Publicly documented donor-reference facts used only to identify the benchmark vehicle:

- BMW E34 530i V8, European manual model;
- 2997 cm³ M60B30 naturally aspirated V8;
- 218 PS / 160 kW at 5800 rpm;
- 290 Nm at 4500 rpm;
- five-speed manual, rear-wheel drive;
- curb weight about 1565 kg;
- wheelbase 2761 mm;
- front/rear track about 1470 / 1495 mm;
- 225/60 R15 tire fitment in the cited production specification;
- double-pivot MacPherson-strut front family and semi-trailing-arm rear family for E34;
- hydraulic recirculating-ball steering on ordinary rear-drive E34 models;
- ventilated front / solid rear disc brakes on the cited 530i V8 application;
- donor production car used ABS.

Sources consulted for donor identification/provenance: BMW technical training material for E34 rear axle family; contemporary/service reference material; Auto-Data; Automobile-Catalog; Brembo application catalog. The donor ABS is **explicitly rejected** for PINK CAB.

## 3. What is copied from the E34 reference

PINK CAB copies the **behavioral character**, not incompatible hardware:

- calm on-center steering;
- progressive steering response rather than nervous twitch response;
- accurate, analog relationship between commanded steering and road-wheel angle;
- predictable build-up and release of lateral grip;
- rear-drive oversteer that is readable and catchable rather than binary;
- good straight-line composure;
- braking that remains steerable and communicative;
- natural body settling after a maneuver without endless oscillation;
- no hidden `Drift Mode`;
- no arcade snap-to-angle behavior;
- no automatic countersteer.

The **target subjective control quality is E34-like even when the Tatra body is visibly moving much more**.

## 4. What remains Tatra and must not be copied from BMW

The playable car remains the bespoke PINK CAB early/Gen-1 Tatra 603-family interpretation:

- rear-mounted air-cooled V8 identity;
- rear-heavy mass distribution / rear-biased dynamic character;
- current locked vehicle mass model: 1450 kg base, 1550 kg full fuel, actual crew/passenger load;
- no ABS;
- no ESP;
- 195 km/h flat-road terminal target;
- 1970s old-car cabin and physical controls;
- visibly softer, more dramatic suspension/body motion than the E34 donor;
- wallride/magnetic behavior under PINK CAB-specific physics rather than donor road-car hardware.

The donor's 218 PS / 290 Nm, 235 km/h production top speed, front-engine layout, ABS, exact wheelbase/track, exact tire size, brake package and suspension hardware are **PROVENANCE ONLY** unless a separate owner lock explicitly adopts a value.

## 5. Powertrain feel

Power is not the limiting fantasy.

The Tatra must feel like a healthy, strong car whose engine has enough authority to accelerate briskly and sustain high-speed work. The driver should not feel that 140 km/h is difficult because the engine is weak.

Instead:

- 0–100 and overtaking should feel lively;
- throttle remains useful at speed and while sliding;
- reaching 140 km/h is easy for the powertrain;
- reaching 195 km/h is possible without a hard speed wall;
- near 140+ the **chassis/body/suspension presentation** begins to communicate that the old car is being pushed well outside comfortable design intent;
- near 195 the car may look and sound busy, floaty and physically loaded, but not power-starved.

Existing 180 hp / 240 Nm gameplay engine target remains current unless separately superseded. E34 engine output is not silently imported by this design.

## 6. Rear-heavy Tatra dynamics

The car's main static/dynamic mass character remains rear-biased because the V8 is behind the cabin.

Required consequences:

- rear axle carries visibly meaningful load;
- lift-off / braking transfers load forward enough to change balance;
- throttle can settle or rotate the car depending on existing slip state;
- oversteer is progressive and recoverable with competent steering/throttle;
- committed bad input can still produce a full spin;
- added passenger/fuel mass follows the separately locked mass/load contract and does not turn the car into a different handling preset.

Exact axle percentages and CG coordinates remain calibration/model-sheet values unless already locked elsewhere. Existing provisional 44/56 may be used only as an explicit calibration reference until final packaging measurement; it must not override the rear-heavy owner direction if final physical geometry requires more rear bias.

## 7. Steering contract — E34-like control, fully analog

Steering must be **precise, calm and non-fatiguing**.

Authoritative principles:

- steering command is continuous/analog;
- player can regulate steering to degree-level resolution;
- no binary steering steps;
- no auto-countersteer;
- no steering correction hidden as ESP;
- speed may reduce control sensitivity/gain, but must not remove the player's physical steering authority;
- large deliberate input can still reach the full mechanical steering envelope at high speed;
- straight-line driving at 140–195 km/h must not require constant nervous micro-correction;
- turn-in builds progressively and the rear follows with readable delay/load transfer rather than instantaneous rotation.

Exact wheel-angle lock, steering ratio/transfer curve, speed-gain curve and input filtering remain calibration items. Existing values in `CD-649` are to be reconciled against this donor-reference contract rather than treated as sacred if they produce a non-E34-like result.

## 8. Pedal and lever authority — continuous physical control

Every primary driving control is an analog state even if keyboard/mouse provides the physical interface:

- steering angle: continuous, degree-readable;
- throttle pressure: continuous 0–100%;
- brake pressure: continuous 0–100%;
- clutch state: continuous 0–100%;
- clutch release speed: continuously adjustable through the approved control grammar;
- handbrake: continuous pull / rear-brake command, not merely ON/OFF;
- vehicle speed is a physical outcome, never a selected mode.

The interaction system may quantize raw mouse-wheel events internally, but the resulting commanded control state must interpolate smoothly enough that the player can intentionally set fine intermediate values.

No ABS/ESP substitutes for poor pedal control.

## 9. Suspension: real physics layer vs visual-expression layer

The suspension is intentionally split conceptually into two coupled layers.

### 9.1 Physical handling layer

The underlying tire/contact/chassis physics remains controlled and readable:

- no endless body oscillation;
- no random instability;
- no fake loss of grip merely because the body animation looks dramatic;
- E34-like steering confidence and progressive grip remain the primary driver-control target.

### 9.2 Tatra expression layer

Visual, camera and audio feedback is **slightly exaggerated relative to the true underlying chassis displacement** so the player reads an old heavy taxi being driven too fast.

Approved cues:

- larger visible roll than the hidden tire model strictly requires;
- visible brake dive and acceleration squat;
- long but controlled body settling;
- body shudder after pothole/patch/rut strikes;
- cabin/trim vibration;
- steering-column/wheel tremor where readable;
- subtle camera vibration coupled to actual road/suspension state;
- panel/seat/trim movement and rattles;
- tire/suspension thumps synchronized with the same physical event.

The expression layer may amplify **presentation**, but it may not invent grip loss, change collision outcome or alter road-wheel forces independently of the physical state.

## 10. Speed character

### 0–100 km/h

Comfortable, lively, easy to place. The old body moves, but steering feels modern-good rather than antique-bad.

### 100–140 km/h

The car is still fully usable and confident. Body motion becomes more noticeable. Surface defects begin to matter visually and dynamically.

### 140–170 km/h

This is where the Tatra starts visually reading as a barge outside its comfort zone: more pitch/roll, light float, stronger rut/pothole reactions, trim vibration and slower body settling. Steering remains calm enough for skilled weaving.

### 170–195 km/h

The powertrain can continue pulling; the difficulty is chassis management. The car must communicate speed through body motion, aerodynamic/road noise, steering sensitivity and suspension workload rather than fake engine weakness. Competent driving remains possible.

## 11. Road defects and body communication

Potholes, patches, ruts, cracks, debris transition and uneven surfaces must use one physical event/state to drive multiple synchronized outputs:

`SURFACE INPUT → WHEEL/SUSPENSION RESPONSE → BODY ROLL/PITCH/HEAVE → STEERING TREMOR → CAMERA/COCKPIT SHAKE → AUDIO/VFX`

The same event may be visually amplified, but outputs must remain coherent.

Ruts are especially important: the physical model can guide the wheel/car along the rut, while the body visibly chatters, leans or heaves more than the minimum physics displacement needed to communicate the event.

## 12. Braking

The donor reference contributes **progressive, steerable braking feel**, not its ABS hardware.

PINK CAB requirements:

- no ABS;
- wheel lock must remain possible under excessive brake command;
- driver can modulate braking precisely;
- loaded car modestly worsens stopping performance under the mass/load contract;
- braking produces visible dive and rear-unloading body language;
- high-speed braking can make the old chassis look busy without adding hidden steering stabilization;
- existing stopping-distance targets remain calibration references until the E34-reference feel pass reconciles them with the new load fixtures.

## 13. Drift / oversteer

The car uses one continuous model from grip to slide to spin.

Target behavior:

- E34-like readability in when rear grip begins to release;
- Tatra rear-engine/rear-heavy character in how throttle/load affects the rear axle;
- throttle, brake, clutch, gear and handbrake all remain live during slide;
- no drift mode;
- no automatic countersteer;
- no artificial speed injection;
- recovery comes from player timing and the predictable tire/steering model;
- body animation may be more dramatic than the physical slip state, but the player must still be able to infer what the tires are doing.

## 14. Handbrake

The handbrake is a real analog physical control.

Requirements:

- partial pull produces partial rear-brake effect;
- full pull produces full calibrated rear-brake effect;
- release is continuous;
- no binary `drift button` interpretation;
- driver can hold an intermediate pull deliberately;
- handbrake state is visible through the physical lever and logged in telemetry.

Exact torque and rise/release curves remain calibration items and may be retuned if needed to preserve the E34-like tire/steering behavior plus Tatra body character.

## 15. Clutch

The clutch is a physical continuous state and remains skill-bearing without becoming a race-sim ritual.

Owner intent from current control system:

- Q owns clutch engagement/disengagement state through the approved interaction model;
- player can influence **release speed** finely;
- launch can stall, engage smoothly or produce a harsh/wheelspin start depending on throttle + release timing;
- no hidden launch control;
- no mandatory double-clutch/rev-match ritual for ordinary play;
- clutch/gear state never resets lateral slip merely because a shift occurs.

Any older document saying the player cannot finely regulate release speed is superseded by this design.

## 16. Mass/load integration

Locked mass authority remains:

- base vehicle 1450 kg;
- full-fuel vehicle 1550 kg;
- heroine 58 kg;
- daughter/conductor 49 kg;
- female passenger 55–70 kg;
- male passenger 65–90 kg;
- actual passenger masses contribute individually;
- 1657 kg full-fuel crew-only reference fixture;
- 2107 kg declared current maximum passenger-load fixture.

Higher mass:

- modestly worsens acceleration;
- modestly worsens braking;
- increases roll/pitch/settling and expression-layer shake;
- makes comparable wallride/fence contact somewhat less prone to abrupt separation;
- shrinks Level 1 residual magnet window smoothly from 5.0 s at 1657 kg to 4.0 s at 2107 kg.

## 17. Calibration philosophy

Do not tune by copying one donor number into Chaos and declaring victory.

The hierarchy is:

1. **driver-control feel:** E34 530i V8 manual benchmark;
2. **mass/layout behavior:** rear-engined PINK CAB Tatra;
3. **power envelope:** PINK CAB V8 / 195 km/h product target;
4. **body presentation:** exaggerated old-Tatra barge animation/audio/camera language;
5. **arcade-sim accessibility:** exact analog control without compulsory race-sim hardware.

When low-level values conflict, preserve this hierarchy and record the tuned profile/version.

## 18. Telemetry requirements

At minimum record:

- total mass and load fixture;
- speed;
- raw steering input and final wheel angle;
- steering gain/filter state;
- throttle/brake/clutch/handbrake commands;
- clutch release-speed setting;
- gear/rpm/delivered torque;
- per-wheel normal load;
- per-wheel slip angle/ratio;
- tire longitudinal/lateral force;
- yaw rate and sideslip;
- physical body roll/pitch/heave;
- visual-expression roll/pitch/shake separately;
- suspension travel/velocity;
- road-surface event id;
- any non-diegetic yaw-damping contribution as an explicit separate channel;
- ABS/ESP state must not exist except as an explicit `abs=false`, `esp=false` capability declaration.

## 19. Acceptance philosophy

A passing Tatra must satisfy both halves simultaneously:

**Control layer:** calm, progressive, E34-like confidence and recoverability.
**Presentation layer:** obviously old, soft, rear-heavy Tatra barge moving harder than its suspension seems comfortable with.

Reject either failure mode:

- `old barge visual + bad/unreadable steering`;
- `perfect modern sedan visual + no Tatra body drama`.

Dedicated deterministic comparison fixtures must cover low speed, 100 km/h, 140 km/h, 170 km/h, 195 km/h; lane change; long bend; rut; pothole/patch; hard brake; lift-off; throttle oversteer; handbrake initiation; recovery; full spin; low/high passenger load; wallride entry.

## 20. Remaining OPEN items

The owner has locked the behavior, but these exact implementation numerics still require calibration rather than invention:

- final physical axle distribution/CG from bespoke Tatra packaging;
- exact steering ratio/lock/filter/gain curve that best reproduces the E34-like target;
- exact tire size/compound and dry/wet force curves;
- exact spring/damper/ARB values;
- exact physical vs visual roll/pitch/heave amplification factors;
- camera-shake limits/accessibility;
- exact brake bias/torque and lockup progression;
- exact analog handbrake torque curve;
- exact clutch release-speed min/max and wheel-step/acceleration mapping;
- final 0–100 and braking benchmark fixture after mass/load reconciliation;
- whether current 180 hp / 240 Nm engine curve remains final or is later retuned while preserving the owner power-feel direction.

Temporary values are `EXPERIMENTAL / NON-AUTHORITY`.

## 21. Administrative consequences

Update/reconcile:

- Jira `CD-562`, `CD-641`, `CD-648..659`, `CD-592`, `CD-701`, `CD-722`;
- durable Confluence handling/Tatra/open-decision/roadmap authorities;
- Git active baseline, authority map, open decisions, mass/load, Level 1 vertical docs and QA matrix;
- replace old fixed-five-second roadmap/QA residue with load-sensitive 5.0→4.0 endpoints;
- do not import BMW ABS/front-engine layout/powertrain numbers as PINK CAB canon.

## 22. Maturity

Owner behavior/design: **APPROVED / LOCKED DIRECTION**.
Low-level calibration numerics: **PARTIAL / OPEN**.
Runtime implementation: **NOT PROVEN**.
Verification: **NOT RUNTIME VERIFIED**.
