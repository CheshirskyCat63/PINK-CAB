# PHY-003 — Causal Drivetrain Telemetry Evidence — 2026-09-26

Status: **DONE / exact canonical main verified**

- runtime PR: #33 → `07b5726ecd352c742875966fb19598577ce2d826`
- exact-main verification infrastructure PR: #34 → `5ab428724e5fa0bdedf471312d40f774beb809d4`
- exact-main Windows run: **36233455129**
- repo/code-health: **32/32 PASS**
- zero-debt: **0**
- writer guard: **68 groups / 80 occurrences PASS**
- `PinkCab.Vehicle.Physics`: **9/9 PASS**
- Editor/Game builds: **PASS**
- baseline validation: **PASS**

## Diagnostic contract

The trace separates:
- raw throttle/brake/clutch;
- prepared/dosed/smoothed input;
- post-drivetrain condition input;
- health-clamped control throttle;
- mechanical throttle response;
- final rev-limiter throttle sent to Chaos;
- requested engine torque after limiter/health;
- ignition combustion permission and permission-gated available torque;
- requested / engaged / Chaos current / Chaos target gears;
- clutch coupling, expected coupled RPM, clutch slip and drivetrain torque capacity;
- external partial-clutch torque vs Chaos mechanical path;
- wheel contact, suspension length/spring force, slip, drive/brake torque, wheel RPM/steer angle;
- ABS/TC configuration and Chaos torque/target-rotation/stabilize assist flags;
- vehicle speed, mass, translational energy;
- model/profile/schema/calibration/hash;
- raw steering mouse delta, virtual cursor, shaped target and final command.

UE 5.8 public `FWheelStatus` does not expose exact tire normal load, slip ratio, or longitudinal/lateral tire force. These fields use explicit availability=false instead of fabricated numbers.

Runtime capture is opt-in via `-PinkCabCausalTelemetry`. It writes:
- `Saved/GitHubGate/VEHICLE_PHYSICS_CAUSAL_TRACE.csv`
- `Saved/GitHubGate/VEHICLE_PHYSICS_CAUSAL_WHEELS.csv`
- `Saved/GitHubGate/VEHICLE_PHYSICS_CAUSAL_TRACE.txt`

The buffer is append-only. Once full it preserves all earliest frames and counts subsequent dropped frames.

## CI correction

After PHY-001, immutable baseline creation is now manual-only through workflow_dispatch `freeze_baseline=true`. Every physics/runtime merge to main is verified automatically without creating a new baseline snapshot.

No vehicle tuning, accepted controls, city or MetaRoad behavior changed.

Next: **PHY-004 · Repeatable calibration fixtures**.
