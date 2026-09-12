# PINK CAB · Native Unreal Chaos Arcade-Sim Program

Date: 2026-09-12
Program: `CD-785`
State: `NATIVE_UNREAL_PRODUCTION_PATH`
Spend target for vehicle physics/destruction dependencies: `EUR 0`
Supersedes production dependency on: FGear / VDS

## Canonical decision

PINK CAB will build hero-Tatra road dynamics and PS2-style destruction on Unreal Engine 5.8 built-in systems.

- Production road-dynamics backend: Chaos Vehicles.
- Baseline/reference: UE Advanced Vehicle template / double-wishbone suspension sample pattern.
- Game-facing abstraction remains `IPinkCabVehicleDynamicsProvider`.
- Production provider becomes Chaos-backed.
- FGear and VDS research stays archived as fallback evidence only; neither is a required dependency or purchase target.

## Physics rule: baseline first, corrections second

1. Build a stock Chaos Tatra baseline with no custom force assists.
2. Enter sourced/locked physical parameters before tuning values.
3. Record handling telemetry on the canonical acceptance course.
4. Identify specific defects or desired authored behavior.
5. Only then enable bounded built-in arcade controls or a narrowly scoped native correction.
6. Every correction requires a named purpose, activation envelope, maximum authority, telemetry channel, data-driven configuration, independent kill switch and before/after evidence.

Forbidden shortcuts:

- hidden velocity overwrites;
- unconditional yaw snaps;
- duplicate vehicle solvers;
- magic constants outside configuration/data assets;
- assists that mask broken collision/suspension;
- blanket third-party procurement because one subsystem needs work.

## Advanced Vehicle baseline

Use the official Advanced Vehicle pattern as the starting laboratory:

- Chaos simulated wheels own physical wheel contact and suspension forces;
- visible wheel/suspension geometry follows the simulated wheel through the animation rig;
- visible mechanical suspension is presentation, not a second physics solver;
- Tatra-specific visual suspension may diverge from the sample geometry while preserving the same simulated/visible separation.

The first implementation target is `CD-786`.

## Tatra calibration

`CD-787` owns the native Chaos parameter matrix.

Parameter groups:

- mass, center of mass and inertia;
- rear-engine RWD drivetrain;
- torque curve, RPM range, engine inertia and braking;
- gearbox and final drive;
- wheel radius/width/mass/inertia;
- steering;
- spring rate, preload, damping, travel and anti-roll;
- tire friction, cornering, slip and load response;
- brake and handbrake torque.

Every value is tagged as `SOURCE`, `DESIGN TARGET`, or `CALIBRATION`.

## Handling telemetry and acceptance course

`CD-788` establishes the baseline and regression course:

- 0–100 / 100–0;
- 40/80 km/h slalom;
- 120/160 km/h emergency lane change;
- constant-radius circle;
- lift-off oversteer;
- power oversteer;
- handbrake initiation;
- Scandinavian flick;
- curb / one-wheel bump / crest / compression;
- 180 km/h traffic weave;
- spin recovery;
- wall touch.

Telemetry includes speed/RPM/gear, steering, per-wheel RPM/travel/load/slip/forces, body yaw/roll/pitch, longitudinal/lateral acceleration and later any arcade-control contribution.

## Bounded arcade-sim layer

`CD-789` may use native Chaos arcade controls such as StabilizeControl, TargetRotationControl, TorqueControl and input-rate shaping only after the stock baseline is captured.

The goal is not to make the car magnetized to the road. The goal is a physically causal car with authored recoverability, speed-sensitive control and progressive drift behavior.

Turning all assists OFF must reproduce the baseline.

## PS2 destruction architecture

`CD-790` owns destruction.

Principle: maximum perceived destruction per millisecond.

Use:

- authored damage-state mesh swaps;
- detachable doors, bumpers, wheels, lamps, glass and trim;
- pooled low-poly rigid-body debris;
- sparks, dust, glass, smoke, fire, decals, skid marks, sound and camera response;
- selective Chaos rigid-body / Geometry Collection events for hero moments or small local sets only.

Do not use expensive continuous deformation across all traffic vehicles.
Do not require VDS.

Damage severity must deterministically map to authored states. Debris has strict lifetime, collision, sleep and pool budgets.

## Traffic / physics interaction bubble

`CD-791` defines physics LOD:

- far traffic: lane/spline agents;
- near traffic: simplified collision/reaction proxies;
- interaction bubble: full Chaos only for hero taxi and vehicles that can materially interact;
- destruction debris: pooled, short-lived, aggressively sleeping.

Promotion/demotion requires hysteresis and no visible driving-line pop.
Hard caps are required for fully simulated traffic vehicles and awake debris bodies.

## Jira execution order

- `CD-785` Native Unreal Chaos Arcade-Sim Program — IN PROGRESS
- `CD-786` Advanced Vehicle baseline + Tatra prototype — IN PROGRESS
- `CD-787` Tatra physical calibration + parameter matrix — BACKLOG; blocked by CD-786
- `CD-788` Vehicle telemetry + handling acceptance course — BACKLOG; blocked by CD-786
- `CD-789` Bounded arcade-assist layer — BACKLOG; blocked by CD-787 + CD-788
- `CD-790` PS2 destruction system — BACKLOG; parallel after baseline architecture is stable
- `CD-791` Physics/traffic interaction bubble + CPU budget — BACKLOG; parallel
- `CD-792` Native Unreal production acceptance gate — blocked by CD-789 + CD-790 + CD-791

## Superseded external-plugin track

`CD-777..CD-784` are complete as historical evaluation work. Their result is retained as fallback research, but the owner decision on 2026-09-12 removes FGear and VDS from the production critical path.

No vendor email needs to be sent for production progress. No plugin purchase is authorized or required.

## Production acceptance

`CD-792` closes only when:

- hero Tatra reaches target handling with Chaos as sole road-dynamics owner;
- baseline and assisted profiles are reproducible;
- no required third-party vehicle/damage plugin is present;
- PS2 destruction is deterministic and bounded;
- build, automation and package pass;
- the stress scene meets the agreed CPU/frame budget.

If a real Chaos limitation is proven, open a narrow native-extension task for that subsystem only. Do not reopen blanket third-party procurement automatically.
