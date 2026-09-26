# PINK CAB — Vehicle Physics Parameter Ledger — 2026-09-26

Baseline audited: `main@7df0fc546e36141c2866a5f5028599eedd37c4a2`.

| Parameter | Current executable/code | Other authority | Program treatment |
|---|---:|---|---|
| Warm idle | 750 RPM | Confluence 27: 850 RPM; owner request: 900–950 | Move to per-model profile; A/B around 925; accept by runtime/human gate |
| Engine max RPM | 8500 | Confluence 27: 6000 nominal / 6200 limiter | Do not silently downgrade; split red-zone/limiter/damage threshold in profile |
| Gearbox safe RPM | 6500 | Physical profile max 8500 | Remove independent contradiction; validate dangerous connected downshift against profile |
| Max torque | 260 Nm | Confluence 27: 240 Nm | Keep as current executable design target until explicit profile decision |
| Max power | ~250 hp asserted by test at redline | Confluence 27: 180 hp | Record as gameplay-tune conflict, not historical claim; resolve at profile gate |
| Final drive | 3.2 | Confluence 27: 4.25 | Benchmark executable first; reconcile with target acceleration/top-speed |
| Forward ratios | 4.6 / 2.2 / 1.5 / 1.1 / 0.85 | Confluence 27: 3.636 / 1.950 / 1.357 / 0.941 / 0.784 | Generate RPM↔speed table and lock per-model/profile |
| Reverse ratio | 4.6 | Confluence 27: 3.5 | Calibrate reverse traction under same engine/clutch rules |
| Reference service mass | 1657 kg | Product contract: 1450 base + 100 fuel + 58 heroine + 49 daughter | Keep fixture; audit one-time accumulation + CG |
| Declared loaded fixture | 2107 kg | Confluence current | Preserve as test fixture, not a permanent car mass |
| Wheelbase | 2980 mm | Current page 28: donor 613 source; hero 603-family needs own | Keep only in 613 profile |
| Front/rear track | 1520 / 1520 mm | Current page 28: donor 613 source | Keep only in 613 profile |
| Tire | 205/70 R14, radius 32.13 cm, width 20.5 cm | donor-source provenance | Per-model explicit wheel data |
| Front steer lock | 41° seed | calibration | Preserve semantics, tune effective high-speed gain separately |
| Nominal front/rear friction multiplier | 2.00 / 0.50 | current executable calibration | Treat as high-risk tune: validate dry/wet combined grip before changing |
| Spring rate | 170 nominal | calibration seed | Derive from sag/load target; retain low/nominal/high variants for experiments |
| Damping ratio | 0.38 | calibration seed | Tune decay after bump, not camera-only softness |
| Travel | front 8+8 cm; rear 9+9 cm | current tests | Validate against model geometry and full-load reserve |
| Steering base counts | 1400 | executable | A/B only; mechanics unchanged |
| Steering stationary travel scale | 3.60 | executable | Maintains heavy standstill feel |
| Moving travel scale | 1.35 → 2.20 | executable | Calibrate low-speed response vs high-speed stability |
| Steering response | 2.5/s stationary; 10.5→6.0/s moving | executable | Compare at 30/60/120 FPS |
| High-speed target gain | 0.55 at 120 km/h | executable | Ensure countersteer authority remains |
| Throttle response | `pow(driver, 0.55)` | executable | Tune only after engine-state/torque-path correctness |
| Clutch release range | runtime cockpit currently 0.20–1.20 s | current owner docs say numeric envelope is not necessarily final owner-lock | Preserve mechanic; calibrate after engine/RPM consistency |
| Stall RPM | 850 | current drivetrain condition | Recalibrate with 900–950 idle so idle/stall bands do not collide |
| Lug start / lug stall | 1000 / 800 | executable | Profileize and test coupled low-RPM behavior |

## Authority rule

A value is not “correct” merely because it appears in Jira, Confluence or a test. For executable behavior, exact source/profile + runtime evidence wins. Historical values remain traceable and are explicitly superseded only after the owner accepts the replacement profile.
