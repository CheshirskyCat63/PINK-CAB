# PINK CAB · Vehicle Feel 90

**Status:** CURRENT EXECUTION PRIORITY
**Accepted fallback:** `accepted/p4-rig06-20261007` → `6edea7747d3a8433188c9fb394b98ae9c320d49b`
**Owner lane:** CD-848 / CD-648 / CD-641
**Rule:** finish the hero-car driving behavior to roughly 80–90% of intended FIRST EURO feel before broad world/taxi/content implementation.

## Baseline rule

The accepted P4 + RIG06 build is immutable fallback evidence. Vehicle Feel 90 is a new calibration lane; it does **not** rewrite the fact that P4 was owner accepted.

Every candidate starts from the accepted tag and must preserve:

- Native Unreal Chaos Vehicles as the sole road-dynamics solver;
- mouse steering / Space gaze;
- Q clutch, W brake, E throttle and E+wheel launch dosing;
- H-pattern topology and requested vs engaged gear separation;
- analog clutch / service brake / handbrake semantics;
- no ABS, ESP, traction control, yaw rescue, auto-countersteer, auto-rev-match, auto-throttle or target-rotation assists;
- presentation/model code cannot own physics or trajectory.

No world, taxi, economy, traffic, Neural or ServiceNode feature may be mixed into this lane.

## Definition of 80–90%

Vehicle Feel 90 means the car is already representative of how it will drive in the shipped FIRST EURO product. Remaining work after this gate may include final road-specific polish, sound, presentation vibration, camera polish and narrow content-specific calibration, but not basic corrections to steering, grip, suspension, mass balance, braking, gearing or engine behavior.

## Ordered gates

### VF0 · Measurements and evidence
- record accepted baseline tag/SHA/profile hash;
- measure RIG06 wheel centers, wheelbase, tracks, wheel radius and body bounds;
- identify which measured values are presentation-only and which may legitimately calibrate the physical profile;
- establish repeatable dry-road fixtures and telemetry before tuning.

### VF1 · Steering / input
- driver-space +right remains +right end-to-end;
- no stale raw delta, queued ghost steering or speed-triggered self-steering;
- standstill effort, urban response and high-speed sensitivity are progressive;
- 30/60/120 FPS input behavior is equivalent within declared tolerance.

### VF2 · Engine / clutch / gearbox
- no high-RPM torque chatter or repeated zero/full torque pulses under constant input;
- RPM, wheel speed and selected ratio remain physically consistent within slip tolerance;
- useful first/second/third gear behavior and engine braking are continuous;
- reverse remains controllable;
- no hidden speed governor or force boost.

### VF3 · Mass / CoM / inertia
- base vehicle mass, fuel, crew/passenger load and declared max fixture apply exactly once;
- rear-engine longitudinal CoM is explicit rather than accidental geometry origin;
- vertical/lateral CoM and inertia are versioned calibration values;
- empty/reference/max-load behavior stays stable and recognizably the same car.

### VF4 · Suspension / body control
- static ride height/sag and wheel contact are coherent;
- spring/preload/damping/travel/anti-roll do not create pogo, ice-like recovery or artificial flatness;
- bumps and lane changes settle without long oscillation or snap rotation.

### VF5 · Dry tyres / brakes
- dry grip is not manufactured by a permanent low-rear-grip shortcut;
- breakaway remains progressive and catchable, while full spin is still possible;
- service braking is analog and stable without ABS;
- handbrake remains a deliberate rear-axle action;
- stopping and lane-change fixtures are repeatable.

### VF6 · Wet / storm
- reduced combined grip is surface-driven, not a second steering model;
- excess throttle can consume rear lateral reserve;
- easing throttle restores reserve without ESP or hidden countersteer;
- wet/storm remain controllable enough for intended arcade-sim play.

### VF7 · Load envelope
- run empty, reference crew/fuel and declared max-load fixtures;
- steering, braking, suspension and breakaway change with load without becoming a different control system;
- no duplicated mass after save/load or passenger/fuel transitions.

### VF8 · Runtime robustness
- dry-road representative maneuvers at 30/60/120 FPS;
- 60/80/120/160/180 km/h bounded steering/settling checks where physically reachable;
- long enough drive to expose heat/state drift, stuck controls, contact loss and accumulation;
- code-health, build and packaged smoke remain green.

### VF9 · Owner acceptance
- one exact-head candidate;
- one delivered `PINCKCAB`;
- owner drives the candidate and accepts/rejects feel;
- accepted tag and rollback pointer are written before any broader gameplay lane starts.

## Candidate discipline

One hypothesis per calibration commit where practical. Change the physical owner, not presentation, to solve physical behavior.

Each feel-changing candidate records:
- parameter delta;
- reason/hypothesis;
- measured before/after;
- automated gate result;
- owner verdict if delivered.

A failed feel candidate is reverted or kept on an archive tag; it is not layered over with compensating hacks.

## Scope guard

During Vehicle Feel 90 the following domains are frozen unless a regression proves a direct dependency:

- `Source/PinkCabWorld`
- `Source/PinkCabTraffic`
- `Source/PinkCabTaxi`
- `Source/PinkCabEconomy`
- `Source/PinkCabPersistence`
- world/taxi/service gameplay content

The RIG06 presentation may be measured and visually corrected, but cannot become a second vehicle solver.

## Exit

Only after VF9 owner acceptance does execution return to FIRST EURO R01 (L1 → L2 → L1 route/streaming closure).
