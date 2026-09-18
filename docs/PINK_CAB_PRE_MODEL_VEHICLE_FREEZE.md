# PINK CAB В· PRE-MODEL VEHICLE FREEZE

Date: 2026-09-17
Gate: `CD-856` — PRE-MODEL VEHICLE FREEZE / asset handoff
Broader Mechanics Freeze: `CD-848` — independent, not a hero-model blocker after this gate
Next asset task: `CD-855`

## Post-handoff authority amendment · 2026-09-18

This file remains **historical CD-856 pre-model evidence**. Current control/mechanics authority is [`PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`](PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md) / Confluence [16744449](https://cheshire-division.atlassian.net/wiki/spaces/TWC/pages/16744449/47+PINK+CAB+Control+Vehicle+Mechanics+Release+Contract). The later owner correction supersedes this file's fixed 0.20–1.20 clutch-range wording, old neutral-gate clutch behavior as a final target, and any conflicting control/mechanics semantics. **CD-856 remains DONE**; historical build/package evidence is not retroactively changed and does not prove the newer QA-01..QA-20 contract.

## Result

The code/interaction side of the hero vehicle is frozen sufficiently for final-model substitution. Final art may replace presentation assets; it must not redefine gameplay, controls, physics ownership, taxi state, economy or persistence.

This is deliberately **not** a claim that all later world, character, Neural or service design is complete. Those surfaces are not allowed to block the model-integration lane after the owner narrowed FIRST EURO priority to a complete, comfortable hero vehicle.

## Locked pre-model contracts

### Production vehicle authority

- Native Unreal Chaos Vehicles is the sole production road-dynamics solver.
- PINK CAB C++ owns input shaping, cockpit state, calibration profile, normalized telemetry and adapters.
- No FGear/VDS dependency is required for the model-integration lane.
- Presentation meshes never own authoritative trajectory, grip, fare, money or persistence state.

### Steering

- Mouse owns steering whenever gaze is not held.
- Positive mouse X remains semantic right-positive throughout cockpit/input code; the Chaos provider alone converts that command to the engine-specific opposite steering sign, fixing the previous inversion without leaking engine convention into gameplay semantics.
- Steering remains continuous with no gameplay center dead-zone.
- High-speed input gain is reduced, while full mechanical steering authority remains available.
- Holding Space preserves the current steering command and diverts mouse motion to gaze; release restores steering ownership.

### Pedals and clutch

- `Q` clutch, `W` brake, `E` throttle; brake + throttle may coexist.
- Keyboard input is converted into continuous internal pedal values rather than teleported 0/1 actuator state.
- Press/release response times are independently tuneable in the pawn.
- Clutch release time is a player-adjustable deterministic parameter from **0.20 s to 1.20 s** with **64 intervals / 65 addressable values**.
- Mouse wheel on the clutch pedal adjusts that release-time parameter.
- Stock Chaos does not expose a native mechanical-clutch axis, so FIRST EURO uses an explicit neutral-gate adapter: while clutch pressure is non-zero the driveline target is neutral; once release reaches zero, the selected cockpit gear reconnects. The adapter is named and tested rather than pretending native clutch support exists.

### Gearbox and handbrake

- Gearbox is manual: reverse, neutral, five forward gears (`R/N/1..5`).
- Automatic gears and automatic reverse are disabled in the canonical Chaos profile.
- Cockpit `SelectedGear` is forwarded to live Chaos through `SetTargetGear`.
- Handbrake remains an independent continuous cockpit amount and reaches the live movement component.

### Suspension baseline

Nominal pre-model calibration seed:

- front total suspension travel: **160 mm** (`8 cm raise + 8 cm drop`);
- rear total suspension travel: **180 mm** (`9 cm raise + 9 cm drop`);
- nominal spring rate: **170**;
- damping ratio: **0.38**;
- wheel-load ratio: **0.38**;
- rollbar scaling: **0.08**;
- spring preload: **32**;
- low/nominal/high spring variants remain data/calibration values, not hard-coded art assumptions.

These are calibration seeds, not mesh dimensions. Final model geometry must not silently rewrite them.

### Cockpit geometry/interaction seam

The runtime already owns **22 stable semantic slots** and their anchors:

`DriverCamera`, `SteeringWheel`, `ClutchPedal`, `BrakePedal`, `ThrottlePedal`, `Gearbox`, `Handbrake`, `Ignition`, `TurnSignals`, `Horn`, `Lights`, `Wipers`, `Washer`, `Taximeter`, `PassengerDoor`, `Dashboard`, `Warnings`, `Navigation`, `Radio`, `RearViewMirror`, `LeftMirror`, `RightMirror`.

Each slot may supply `LocalTransform`, `MeshOverride`, `MaterialOverride`, `MotionAxis`, `MotionPivot` and `MotionRange`. Final cockpit art binds to these stable slots; input semantics do not bind to arbitrary mesh names.

## Verified evidence on the pre-model branch

- UE 5.8.2 `PinkCabEditor Win64 Development`: PASS.
- `PinkCab.Vehicle`: PASS / exit code 0.
- PIE drive smoke: PASS; live diagnostics reached first gear with engine RPM and four wheels active.
- New tests cover steering sign/high-speed gain, manual gearbox propagation, clutch neutral-gate behavior, continuous pedal response, clutch release envelope and 160/180 mm suspension travel.
- Existing cockpit runtime contract proves all 22 slots have anchors and the driver camera is the active gameplay camera.
- Canonical Development Win64 `BuildCookRun -build -cook -stage -pak -archive`: PASS; AutomationTool ExitCode=0.
- Import boundary is specified in `docs/PINK_CAB_TATRA_ASSET_IMPORT_CONTRACT.md`.

## Model handoff rule

`CD-855` may consume this contract. It may add/import/replace visual assets, collision authoring and presentation bindings, but must not reopen the above contracts merely to accommodate a badly prepared mesh. If an imported model violates scale, axes, handedness, pivots or decomposition rules, fix the asset/import profile rather than compensating by reversing gameplay logic.
