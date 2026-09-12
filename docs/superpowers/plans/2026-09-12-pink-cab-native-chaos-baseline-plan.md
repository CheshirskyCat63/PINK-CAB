# PINK CAB Native Chaos Baseline Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver `CD-786` as a playable UE 5.8 Chaos vehicle baseline using PINK CAB's provider/control contract, mouse steering, and a deterministic weave course made from vehicle-sized blocks.

**Architecture:** Keep `IPinkCabVehicleDynamicsProvider` as the game-facing boundary. A Chaos-backed provider writes normalized controls into `UChaosWheeledVehicleMovementComponent` and reads authoritative speed/RPM/gear telemetry; `APinkCabChaosTatraPawn` owns the stock Chaos vehicle and temporary Epic Advanced Vehicle sample chassis assets. No arcade force assists are enabled in this task.

**Tech Stack:** Unreal Engine 5.8.2 C++, ChaosVehicles, PhysicsCore, Enhanced Input/legacy axis bridge only for baseline control capture, Unreal Automation Tests, Epic-installed Advanced Vehicle template assets.

**Spec:** `docs/qa/PINK_CAB_NATIVE_UNREAL_CHAOS_ARCADE_SIM_PLAN.md`

## Global Constraints

- UE line is exactly 5.8; current workstation evidence is UE 5.8.2.
- Vehicle-physics dependency spend remains EUR 0.
- Chaos Vehicles is the sole road-dynamics backend.
- Existing `IPinkCabVehicleDynamicsProvider` stays the PINK CAB-facing boundary.
- Stock baseline first: no StabilizeControl, TargetRotationControl, TorqueControl, velocity overwrite, yaw snap, or custom force assist.
- Mouse steering has no authored center dead-zone; Space remains reserved for gaze/free-look and must not become handbrake.
- Preserve deliberate-spin capability; do not add ABS/ESP rescue in CD-786.
- Temporary chassis resources may come only from the locally installed official UE 5.8 Advanced Vehicle template and are replaceable by the Tatra art later.
- Do not fake clutch support. Record stock Chaos clutch as unsupported at this integration boundary until proven otherwise.

---### Task 1: Chaos provider contract

**Files:**
- Modify: `PinkCab.uproject`
- Modify: `Source/PinkCab/PinkCab.Build.cs`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabChaosVehicleDynamicsProvider.h`
- Create: `Source/PinkCab/Private/Vehicle/PinkCabChaosVehicleDynamicsProvider.cpp`
- Create: `Source/PinkCabTests/Private/Vehicle/PinkCabChaosVehicleProviderTests.cpp`

**Interfaces:**
- Consumes: `FPinkCabVehicleControlState`, `FPinkCabVehicleTelemetry`, `IPinkCabVehicleDynamicsProvider`.
- Produces: `FPinkCabChaosVehicleDynamicsProvider(UChaosWheeledVehicleMovementComponent*)`, `ApplyControls`, `ReadTelemetry`.

- [ ] **Step 1:** Add automation tests that construct a transient `UChaosWheeledVehicleMovementComponent`, bind the provider, apply steering/throttle/brake/handbrake, and assert Chaos raw inputs mirror the PINK CAB normalized contract.
- [ ] **Step 2:** Build before implementation and verify RED because `PinkCabChaosVehicleDynamicsProvider.h` does not exist.
- [ ] **Step 3:** Enable the built-in `ChaosVehicles` plugin/module plus `PhysicsCore`, then implement the minimal provider. Handbrake maps to boolean using `>= 0.5f`; clutch is preserved only in returned normalized telemetry and does not claim physical actuation.
- [ ] **Step 4:** Build and run `PinkCab.Vehicle.ChaosBaseline.Provider*`; expect PASS.
- [ ] **Step 5:** Commit `feat(CD-786): add native Chaos vehicle provider`.

### Task 2: Stock Chaos Tatra-baseline pawn and wheels

**Files:**
- Create: `Source/PinkCab/Public/Vehicle/PinkCabChaosTatraPawn.h`
- Create: `Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabChaosWheelFront.h`
- Create: `Source/PinkCab/Private/Vehicle/PinkCabChaosWheelFront.cpp`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabChaosWheelRear.h`
- Create: `Source/PinkCab/Private/Vehicle/PinkCabChaosWheelRear.cpp`
- Extend: `Source/PinkCabTests/Private/Vehicle/PinkCabChaosVehicleProviderTests.cpp`
**Interfaces:**
- Consumes: official Chaos `AWheeledVehiclePawn`, `UChaosWheeledVehicleMovementComponent`, provider from Task 1.
- Produces: spawnable `APinkCabChaosTatraPawn` with `GetPinkCabDynamicsProvider()` and `GetChaosMovement()` accessors.

- [ ] **Step 1:** Add tests asserting the pawn uses a `UChaosWheeledVehicleMovementComponent`, has four wheel setups, RWD rear wheels, front steering, stock arcade controls disabled, and mass initialized from `FPinkCabTatraProfile::Canonical()` reference fixture.
- [ ] **Step 2:** Build and verify RED because pawn/wheel classes do not exist.
- [ ] **Step 3:** Implement the minimal pawn and wheel classes using Epic Advanced Vehicle SportsCar constructor setup as a reference, but keep all PINK CAB tuning values explicit and tagged baseline/calibration in code comments.
- [ ] **Step 4:** Keep automatic gears for this first executable baseline; configure Ackermann steering; do not enable auto stabilization or custom forces.
- [ ] **Step 5:** Build and run `PinkCab.Vehicle.ChaosBaseline.Pawn*`; expect PASS.
- [ ] **Step 6:** Commit `feat(CD-786): add stock Chaos Tatra baseline pawn`.

### Task 3: PINK CAB baseline driving input

**Files:**
- Create: `Source/PinkCab/Public/Vehicle/PinkCabDrivingInputComponent.h`
- Create: `Source/PinkCab/Private/Vehicle/PinkCabDrivingInputComponent.cpp`
- Modify: `Config/DefaultInput.ini`
- Extend: `Source/PinkCabTests/Private/Vehicle/PinkCabChaosVehicleProviderTests.cpp`

**Interfaces:**
- Consumes: mouse delta, W/S throttle-brake baseline keys, provider handle.
- Produces: one `FPinkCabVehicleControlState` per frame; Space suppresses steering input so gaze can own mouse later without changing vehicle state unexpectedly.

- [ ] **Step 1:** Add pure input-state tests for continuous steering accumulation/return behavior, throttle/brake exclusivity, handbrake command, and Space gaze hold preserving the last steering state without mapping Space to handbrake.
- [ ] **Step 2:** Build and verify RED because `PinkCabDrivingInputComponent` does not exist.
- [ ] **Step 3:** Implement a small actor component with testable input methods (`AddMouseSteering`, `SetThrottle`, `SetBrake`, `SetHandbrake`, `SetGazeHeld`, `TickControl`).
- [ ] **Step 4:** Bind minimal project input axes/actions to the pawn; default baseline uses mouse steering, W throttle, S brake/reverse behavior through Chaos, and a non-Space handbrake key.
- [ ] **Step 5:** Build and run `PinkCab.Vehicle.ChaosBaseline.Input*`; expect PASS.
- [ ] **Step 6:** Commit `feat(CD-786): route PinkCab driving input into Chaos`.
### Task 4: Official UE chassis assets + weave course

**Files:**
- Copy official UE 5.8 Advanced Vehicle assets into `Content/VehicleTemplate/Meshes` and `Content/VehicleTemplate/Materials`, preserving `/Game/VehicleTemplate/...` package paths.
- Create: `Source/PinkCab/Public/World/PinkCabChaosWeaveCourse.h`
- Create: `Source/PinkCab/Private/World/PinkCabChaosWeaveCourse.cpp`
- Create: `Source/PinkCabTests/Private/World/PinkCabChaosWeaveCourseTests.cpp`
- Modify: `Config/DefaultEngine.ini`

**Interfaces:**
- Consumes: spawnable pawn and engine basic shapes/materials.
- Produces: deterministic test world actor that creates road floor, spawn point, and vehicle-sized obstacle blocks in alternating lanes.

- [ ] **Step 1:** Add tests for deterministic obstacle count, lane alternation, minimum clear gap, and course bounds; build to verify RED because the course class does not exist.
- [ ] **Step 2:** Implement the course as C++ spawned static-mesh components using engine basic Cube/Plane assets so it is source-controlled and does not require hand-authored map editing.
- [ ] **Step 3:** Copy only required official Advanced Vehicle mesh/physics/material assets; verify no TP_VehicleAdv native Blueprint dependency is imported.
- [ ] **Step 4:** Add a development game mode/map bootstrap path that spawns `APinkCabChaosTatraPawn` and `APinkCabChaosWeaveCourse` when launching the dedicated Chaos baseline map/URL.
- [ ] **Step 5:** Build and run `PinkCab.World.ChaosWeaveCourse*`; expect PASS.
- [ ] **Step 6:** Commit `feat(CD-786): add Chaos weave driving course`.

### Task 5: Runtime smoke, evidence, and admin closure

**Files:**
- Create: `docs/qa/CD786_CHAOS_BASELINE_EVIDENCE_20260912.md`
- Modify: `docs/VERIFICATION_MATRIX.md` only for verified CD-786 rows.
- Update Jira `CD-786` and parent `CD-785` with exact evidence.

- [ ] **Step 1:** Run `scripts/build.ps1`; require exit 0.
- [ ] **Step 2:** Run targeted `PinkCab.Vehicle.ChaosBaseline.*` and `PinkCab.World.ChaosWeaveCourse.*`; require all discovered tests PASS.
- [ ] **Step 3:** Launch UnrealEditor/PIE or standalone on the baseline course and verify the pawn accelerates, brakes, steers continuously with mouse, and can weave between blocks without any arcade force assist enabled.
- [ ] **Step 4:** Record exact UE version, Git SHA, tests, runtime observations, known limitation that stock clutch is not yet integrated, and whether Tatra art is still placeholder.
- [ ] **Step 5:** Run authority guard and `git diff --check`.
- [ ] **Step 6:** Push feature branch and update Jira. Mark `CD-786` DONE only if build + automation + runtime smoke are green; otherwise leave IN PROGRESS with the concrete blocker.
