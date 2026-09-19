# PINK CAB Control Runtime Design

**Date:** 2026-09-18
**Project:** PINK CAB
**Umbrella:** CD-848 Mechanics Freeze
**Authority:** `docs/PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`
**Owner approval:** chat approval on 2026-09-18
**Runtime:** Unreal Engine 5.8.2 / native Chaos Vehicles

## Goal

Implement the approved PINK CAB vehicle-control mechanics as code-complete, automated-testable runtime logic without touching the final car model or relying on manual feel testing.

The implementation keeps native Chaos Vehicles as the sole road-dynamics solver. PINK CAB owns input transfer, state classification, gearbox/clutch validation, analog handbrake actuation, mechanical wear/damage and persistence.

## Non-goals

- no final Tatra model/material/cockpit art work;
- no manual handling acceptance in this phase;
- no ABS, ESP, launch control, autothrottle, auto-rev-match, auto-countersteer or yaw rescue;
- no second tire/suspension solver;
- no model mirroring to fix steering sign;
- no silent replacement of already-approved input grammar.
## Vehicle motion classification

The runtime exposes two stable vehicle modes plus one transient launch phase.

### STATIONARY

The vehicle is considered stationary below a calibrated enter threshold after hysteresis/debounce.

Stationary behavior owns:
- parking-handbrake latch behavior;
- heavy steering response representing no power steering;
- launch preparation and per-launch throttle-dose reset;
- normal cockpit manipulation while stopped.

### MOVING

The vehicle is considered moving above a calibrated exit threshold after hysteresis/debounce.

Moving behavior owns:
- hydraulic-style momentary handbrake with auto-return;
- lighter/faster steering response;
- speed-dependent steering target/response calibration;
- normal road-driving control semantics.

### LAUNCH

LAUNCH is a transient transition state, not a third persistent driving mode.

It starts when a new departure attempt begins from STATIONARY and ends once the vehicle is stably MOVING or the attempt is cancelled/stalled back to STATIONARY.

Each new LAUNCH requires E+wheel throttle dosing again exactly once.
Speed jitter inside one launch must never retrigger the reset.
## Steering model

Mouse steering remains the default input. Holding Space transfers the mouse to gaze; releasing Space returns it to steering.

The steering pipeline is:

`raw mouse delta -> virtual steering cursor -> nonlinear target curve -> speed/state response rate -> logical steering -> Chaos sign adapter`.

### Virtual steering circle

The player should not sweep the mouse across the desk. Steering uses a bounded accumulated virtual cursor with a comfortable full useful span calibrated to roughly +/-5 cm of physical mouse travel on the owner's current setup.

The implementation stores dimensionless normalized cursor displacement, not literal centimeters. A sensitivity/calibration scalar maps device counts to the virtual circle.

### Nonlinear response

Near center, steering is deliberately fine and calm. Larger cursor displacement increases steering command progressively so countersteer can be fast without making small corrections nervous.

The target curve is monotonic and odd-symmetric around zero.

### Speed/state response

At STATIONARY the steering wheel is heavy because the car has no power steering: target authority remains available, but the rate at which physical steering approaches the target is deliberately low.

As road speed increases:
- steering response rate increases;
- the useful target response becomes sharper;
- the mouse still stays inside the same bounded virtual circle;
- small corrections remain controllable.

This is player-input transfer only. No yaw/slip state is allowed to steer the car for the player.
## Handbrake model

Quick key 4 selects the handbrake. RMB acquires/holds it with the right hand.

The same physical lever has two automatic behaviors selected only by vehicle mode.

### STATIONARY parking behavior

Pulling the lever while stationary changes an analog 0..1 handbrake command.

On RMB release the current position latches and remains as a parking brake.

Reacquiring the lever allows the player to reduce or increase the latched amount.

### MOVING hydraulic behavior

While moving, pulling the lever applies analog rear braking only while RMB owns the lever.

Lever displacement is mapped through a progressive nonlinear curve: small pull gives fine control; larger pull ramps strongly toward full rear-brake torque.

On RMB release the lever auto-returns toward zero quickly but continuously. It never toggles or latches while MOVING.

If a latched parking brake exists and the vehicle transitions to MOVING without the lever being held, the latch releases into the moving auto-return path rather than remaining an accidental permanent drag.

Chaos receives rear-wheel handbrake/brake torque magnitude, not a bool collapse.
## Pedal and launch controls

The existing owner grammar remains unchanged:
- Q = clutch;
- Q+wheel = clutch release-time adjustment;
- W = brake;
- W+wheel = brake target/dosing;
- E = throttle;
- E+wheel = throttle target/dosing;
- W+E may coexist;
- simultaneous Q/W/E wheel ownership priority is E -> W -> Q.

Throttle target must reset for every new LAUNCH from standstill and require new E+wheel dosing.

The reset occurs once per launch event, not every frame, not from low-speed oscillation, and not repeatedly during one held E event.

Pedal application stays continuous and deterministic.

## Clutch and driveline

Selected/requested gear and actually engaged gear are separate states.

Clutch is modeled as continuous torque-transfer capacity rather than a binary neutral gate.

The engagement validator consumes:
- requested gear;
- current engaged gear;
- clutch transfer amount;
- engine RPM;
- road/wheel speed estimate;
- transmitted load;
- gearbox health.

Matched low-load clutchless engagement may succeed as a difficult skill. Loaded or badly mismatched engagement may refuse/grind and accumulate wear.
Dangerous connected downshift may overspeed/damage the drivetrain. There is no automatic rev matching.

All shift paths, including physical H-gate and any retained ShiftBy/wheel path, route through the same validator.

The H-gate topology remains:
- top: 1 / 3 / 5;
- center corridor: N;
- bottom: 2 / 4 / R.

No diagonal wall teleport is accepted.

## Mechanical consequences

Vehicle Health gains dedicated drivetrain channels/state for:
- clutch condition;
- clutch temperature;
- gearbox/transmission condition;
- brake temperature/fade where required by existing health architecture.

Required causal behavior includes:
- clutch slip generates clutch heat and wear;
- harsh clutch engagement can shock/wear drivetrain;
- coupled braking to stall can stall engine;
- brake work generates brake heat;
- handbrake drag on rotating rear wheels generates rear-brake heat/wear;
- held handbrake with clutch disengaged and free-revving engine does not heat rear brakes;
- persistent damage survives save/load;
- loading never repairs the vehicle.

Healthy normal urban driving must not force service inside the 30-minute owner acceptance target.
## Chaos integration

Native Chaos Vehicles remains the only road-dynamics solver.

PINK CAB may use public UE 5.8 Chaos APIs for:
- steering/throttle/brake inputs;
- target gear;
- per-wheel brake torque;
- per-wheel drive torque;
- wheel telemetry.

The adapter boundary owns the coordinate/sign conversion. Logical steering stays right-positive in PINK CAB space; only the final Chaos adapter may invert if UE's coordinate convention requires it.

Analog handbrake must use per-wheel torque magnitude rather than `SetHandbrakeInput(bool)` as the authoritative analog path.

Continuous clutch transfer may scale/override delivered rear-wheel drive torque through the adapter when needed, while preserving Chaos tire/suspension/contact simulation.

## State ownership and isolation

New runtime logic is split into focused units rather than expanding the pawn into one giant state machine:
- vehicle motion classifier;
- steering controller;
- launch/pedal controller;
- handbrake actuator;
- gearbox/clutch controller;
- drivetrain wear model.

`APinkCabChaosTatraPawn` orchestrates these units and supplies frame input/telemetry. It does not own their detailed rules.
## Persistence

Persistence schema is versioned.

At minimum save/load preserves:
- drivetrain health/wear;
- clutch wear;
- gearbox wear;
- permanent brake damage;
- other existing Vehicle Health channels.

Transient interaction ownership, moving handbrake pull, and an active launch throttle command are not restored as if the player were still holding controls.

A loaded stationary car must require the normal per-launch E+wheel throttle dosing before a new departure.

## Automated verification

Implementation follows TDD. Each behavior is introduced by a failing automation test, then minimal production code, then refactor.

Required automated coverage includes:
- state hysteresis and launch edge detection;
- one throttle reset per launch;
- Q/W/E wheel routing priority;
- stationary heavy steering vs faster moving response;
- bounded virtual steering cursor and nonlinear curve;
- no steering change while Space gaze owns the mouse;
- steering sign adapter boundary;
- moving handbrake nonlinear pull and auto-return;
- stationary handbrake latch;
- analog rear torque at 25/50/100%;
- requested vs engaged gear separation;
- H-gate neutral traversal;
- continuous clutch transfer;
- mismatch refusal/grind/damage;
- clutch/brake heat causal cases;
- persistence round-trip/version handling;
- zero ABS/ESP/yaw-rescue configuration.

Manual feel tests and final Tatra model integration remain a later gate.
## Completion definition

This implementation phase is code-complete only when:
1. relevant automation tests pass on the exact feature HEAD;
2. canonical PinkCabEditor build passes;
3. no model/material/presentation assets are required for correctness;
4. docs/authority mirror the approved runtime semantics;
5. code review is completed before merge;
6. manual owner handling/model acceptance is explicitly left NOT RUN.

Code-complete does not mean handling VERIFIED.
