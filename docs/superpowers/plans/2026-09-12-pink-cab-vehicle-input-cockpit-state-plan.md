# PINK CAB Vehicle / Input / Cockpit State Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete `CD-795` so the playable native-Chaos taxi uses one canonical input contract, one cockpit gameplay state, one provenance-tagged physical profile, and production telemetry without depending on final art.

**Architecture:** Keep Chaos as sole road-dynamics owner. `FPinkCabSemanticInputRouter` owns keyboard/mouse semantics; `APinkCabChaosTatraPawn` translates only resolved semantics into `FPinkCabVehicleControlState` and cockpit commands. `FPinkCabCockpitState` is pure gameplay state, while `FPinkCabChaosPhysicalProfile` owns vehicle numbers and provenance.

**Tech Stack:** Unreal Engine 5.8.2, C++20, Chaos Vehicles, Unreal Automation Framework.

**Spec:** `docs/superpowers/specs/2026-09-12-pink-cab-core-gameplay-code-complete-design.md`

## Global Constraints
- Mouse steering remains default; Space transfers mouse ownership to gaze and release returns steering.
- Canonical pedals: `Q clutch intent`, `W brake`, `E throttle`.
- `1–4` recall targets only; they never actuate a control by themselves.
- LMB = momentary press/hold; RMB = grip where a target requires grip.
- Chaos remains sole tire/suspension/road-dynamics owner; all built-in arcade assists remain OFF.
- Do not fake a mechanical clutch. UE 5.8.2 ChaosVehiclesPlugin exposes no clutch input; capability must be explicit until a narrow native drivetrain extension exists.
- Final meshes, animations, sounds and UI art are outside this gate.

---
### Task 1: Canonical production input ownership

**Files:**
- Modify: `Source/PinkCab/Public/Interaction/PinkCabSemanticInputRouter.h`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleInputFrame.h`
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabChaosTatraPawn.h`
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp`
- Test: `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleInputContractTests.cpp`

**Interfaces:**
- `FPinkCabSemanticInputRouter::GetKeyForAction(EPinkCabSemanticAction) -> FKey`
- `FPinkCabVehicleInputFrame::FromDigital(...) -> FPinkCabVehicleInputFrame`
- `APinkCabChaosTatraPawn::ApplyInputFrame(const FPinkCabVehicleInputFrame&)`

- [ ] Write RED tests for `Q/W/E`, gaze ownership and no direct `W throttle / S brake` legacy mapping.
- [ ] Add explicit `EPinkCabMechanicalClutchCapability` to input/provider contract; stock Chaos reports `Unsupported`.
- [ ] Implement minimal input frame and reverse action lookup.
- [ ] Replace direct pedal key literals in Pawn Tick with router-resolved actions.
- [ ] Run input + Chaos baseline tests GREEN and commit.

### Task 2: Cockpit gameplay state on placeholders

**Files:**
- Create: `Source/PinkCab/Public/Vehicle/PinkCabCockpitState.h`
- Test: `Source/PinkCabTests/Private/Vehicle/PinkCabCockpitStateTests.cpp`

**Interfaces:**
- ignition: `Off / Running / Stalled`
- gearbox: integer selected gear with bounded shift requests
- handbrake: latched boolean
- passenger door: open/closed boolean
- meter: `Off / Running / Stopped`

- [ ] RED tests for ignition/stall/restart, gear bounds, handbrake, passenger door and meter state transitions.
- [ ] Implement pure deterministic state with idempotent setters/toggles.
- [ ] Map interaction target IDs `Gearbox`, `Handbrake`, `Ignition`, `PassengerDoor`, `Meter` into cockpit commands.
- [ ] Run cockpit/interaction tests GREEN and commit.
### Task 3: Provenance physical profile

**Files:**
- Reuse plan: `docs/superpowers/plans/2026-09-12-pink-cab-tatra-chaos-calibration-plan.md`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabChaosPhysicalProfile.h`
- Create: `Source/PinkCab/Private/Vehicle/PinkCabChaosPhysicalProfile.cpp`
- Modify: Pawn and front/rear wheel constructors.
- Test: `Source/PinkCabTests/Private/Vehicle/PinkCabChaosPhysicalProfileTests.cpp`

- [ ] Implement `SOURCE / DESIGN_TARGET / CALIBRATION` tagging exactly as the calibration sub-plan specifies.
- [ ] Move drivetrain, steering, wheel, tire, brake and suspension literals out of Pawn/Wheel classes.
- [ ] Keep Low/Nominal/High variants restricted to CALIBRATION fields.
- [ ] Run profile tests and existing runtime `DriveSmoke` GREEN.

### Task 4: Production vehicle telemetry + final runtime gate

**Files:**
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabVehicleTelemetry.h`
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosVehicleDynamicsProvider.cpp`
- Test: provider + runtime smoke tests.

**Interfaces:**
- vehicle: speed, engine RPM, current gear and normalized commands.
- per wheel (4): contact, suspension length, spring force, slip angle/magnitude, skid magnitude, drive torque, brake torque, ABS state.
- clutch capability is separate from normalized clutch intent.

- [ ] RED provider tests require four populated wheel telemetry slots from real `FWheelStatus` fields.
- [ ] Implement bounded wheel-state copy using `GetNumWheels()` / `GetWheelState()`.
- [ ] Extend PIE DriveSmoke to require gear/RPM/wheel-contact telemetry and canonical live input mapping.
- [ ] Run `PinkCab.Vehicle`, `PinkCab.Interaction`, fresh full build and `git diff --check`.
- [ ] Record exact-head evidence in Jira `CD-795`; move DONE only with fresh green evidence.
