# PINK CAB Control Runtime Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the approved PINK CAB steering, launch, pedals, clutch, H-gate, handbrake, drivetrain wear and persistence logic code-complete on native UE 5.8 Chaos without touching the final vehicle model.

**Architecture:** Keep Chaos as the sole tire/suspension/contact solver. Add focused deterministic PINK CAB controllers between input/cockpit state and the existing Chaos adapter; the pawn only orchestrates controller inputs/outputs and telemetry.

**Tech Stack:** Unreal Engine 5.8.2, C++20/UE C++, Chaos Vehicles, UE Automation Tests, Git.

**Spec:** `docs/superpowers/specs/2026-09-18-control-runtime-design.md`

## Global constraints

- Do not modify Tatra model/material/presentation assets.
- Preserve dirty `CD-855` worktree untouched.
- TDD: every behavior change starts with a failing automation test.
- No ABS/ESP/autothrottle/rev-match/countersteer/yaw rescue.
- No second road-dynamics solver.
- Manual feel/model tests are deferred and remain NOT RUN.
- Use existing Jira owners; do not create duplicate tickets.
## File structure

New focused runtime units:
- `Public/Vehicle/PinkCabVehicleMotionClassifier.h`
- `Public/Vehicle/PinkCabSteeringController.h`
- `Public/Vehicle/PinkCabLaunchController.h`
- `Public/Vehicle/PinkCabHandbrakeActuator.h`
- `Public/Vehicle/PinkCabGearboxController.h`
- `Public/Vehicle/PinkCabDrivetrainCondition.h`

Modify integration/state:
- `Public/Vehicle/PinkCabCockpitState.h`
- `Public/Vehicle/PinkCabCockpitInteractionRouter.h`
- `Private/Vehicle/PinkCabChaosCockpitBridge.cpp`
- `Public/Vehicle/PinkCabChaosCockpitBridge.h`
- `Private/Vehicle/PinkCabChaosVehicleDynamicsProvider.cpp`
- `Public/Vehicle/PinkCabChaosVehicleDynamicsProvider.h`
- `Private/Vehicle/PinkCabChaosTatraPawn.cpp`
- `Public/Vehicle/PinkCabChaosTatraPawn.h`
- `Public/Vehicle/PinkCabVehicleHealthState.h`
- `Public/Vehicle/PinkCabVehicleHealthService.h`
- `Public/Persistence/PinkCabVehicleSnapshot.h`
- `Public/Persistence/PinkCabVehicleSnapshotArchive.h`

Tests stay in existing PinkCabTests module plus small focused new test files if needed.
## Task 1 — Motion classifier and launch edge

- [ ] Add failing tests for STATIONARY/MOVING hysteresis, no speed-jitter chatter and deterministic transition.
- [ ] Add failing tests for exactly one LAUNCH start per departure attempt and reset after return/stall.
- [ ] Run only the new tests and confirm expected RED failures.
- [ ] Implement `FPinkCabVehicleMotionClassifier`.
- [ ] Implement `FPinkCabLaunchController` with launch serial and throttle-dose-required state.
- [ ] Re-run focused tests to GREEN.
- [ ] Run adjacent vehicle input tests.
- [ ] Commit as one independently reviewable change.

## Task 2 — Mouse steering transfer

- [ ] Add failing tests for bounded normalized virtual cursor.
- [ ] Add failing tests for odd-symmetric nonlinear target curve and fine center control.
- [ ] Add failing tests proving STATIONARY response is slower/heavier than MOVING.
- [ ] Add failing tests proving higher speed increases response rate while usable cursor span remains bounded/slightly wider.
- [ ] Add failing tests proving Space gaze freezes steering ownership and no yaw/slip input changes steering.
- [ ] Verify RED.
- [ ] Implement `FPinkCabSteeringController` with configurable counts-per-full-scale and speed/state response.
- [ ] Keep logical steering right-positive; keep Chaos sign conversion isolated in provider.
- [ ] Integrate controller into pawn without model/presentation changes.
- [ ] Re-run focused and existing steering tests to GREEN.
- [ ] Commit.
## Task 3 — Pedal dosing and per-launch throttle reset

- [ ] Add failing tests for E+wheel throttle target, W+wheel brake target and Q+wheel clutch release-time adjustment.
- [ ] Add failing test for E -> W -> Q single-recipient wheel priority when keys overlap.
- [ ] Add failing test proving W+E coexist.
- [ ] Add failing test proving every new LAUNCH requires fresh E+wheel dosing.
- [ ] Add failing test proving speed jitter/held E cannot repeatedly reset throttle within one launch.
- [ ] Verify RED.
- [ ] Implement deterministic pedal target/dosing state in `FPinkCabLaunchController` or a focused pedal state owned by it.
- [ ] Route wheel before cockpit wheel actuation so one detent has one recipient.
- [ ] Preserve continuous pedal ramping; do not add autothrottle.
- [ ] Re-run focused and existing input tests to GREEN.
- [ ] Commit.

## Task 4 — Dual-behavior analog handbrake

- [ ] Add failing tests for STATIONARY analog pull + RMB-release latch.
- [ ] Add failing tests for MOVING nonlinear pull + RMB-release auto-return.
- [ ] Add failing test for latched parking brake transitioning to MOVING and releasing into auto-return.
- [ ] Add failing tests for 25/50/100% distinct output torque.
- [ ] Verify RED.
- [ ] Implement `FPinkCabHandbrakeActuator` driven by motion mode, grip ownership and mouse pull delta.
- [ ] Change Chaos adapter to keep bool handbrake false for the analog authority path.
- [ ] Apply additive external rear-wheel brake torque using UE 5.8 `SetTorqueCombineMethod(Additive)` + `SetBrakeTorque`.
- [ ] Re-run handbrake/provider tests to GREEN.
- [ ] Commit.
## Task 5 — H-gate, requested gear and common engagement validator

- [ ] Add failing tests for 1/3/5 top, N corridor, 2/4/R bottom topology.
- [ ] Add failing tests that diagonal wall crossing cannot teleport directly between slots.
- [ ] Add failing tests separating requested gear from engaged gear.
- [ ] Add failing tests that legacy ShiftBy/wheel request still passes through the same validator.
- [ ] Add failing tests for mismatch refusal/grind, matched low-load clutchless engagement and dangerous connected downshift classification.
- [ ] Verify RED.
- [ ] Implement `FPinkCabGearboxController` with H-gate selector, request state, engaged state and deterministic validation result.
- [ ] Make `FPinkCabCockpitState` expose requested lever gear; remove unconditional direct gear authority.
- [ ] Route all gear requests through the controller before Chaos `SetTargetGear`.
- [ ] Re-run gearbox/cockpit tests to GREEN.
- [ ] Commit.

## Task 6 — Continuous clutch transfer and stall

- [ ] Add failing tests for monotonic clutch torque capacity across 0/25/50/75/100% engagement.
- [ ] Add failing test proving half clutch does not collapse to neutral/fully-engaged binary behavior.
- [ ] Add failing tests for launch stall, free rev with clutch disengaged and braking-to-stop while coupled.
- [ ] Add failing tests for harsh engagement shock metric and slip-energy accumulation.
- [ ] Verify RED.
- [ ] Extend gearbox/drivetrain controller with continuous coupling output and engine/load mismatch state.
- [ ] Use Chaos internal gear when fully coupled/matched; use neutral + additive rear-wheel drive torque during partial coupling so tire/contact physics remains Chaos-owned.
- [ ] Keep no automatic rev matching.
- [ ] Re-run clutch/gearbox/provider tests to GREEN.
- [ ] Commit.
## Task 7 — Causal drivetrain/brake condition

- [ ] Add failing tests for clutch heat from slip and no clutch heat when fully disengaged.
- [ ] Add failing tests for brake heat only when rotating wheels perform braking work.
- [ ] Add failing test: free-rev engine + held handbrake + disengaged clutch does not heat rear brakes.
- [ ] Add failing tests for reversible brake fade and permanent wear separation.
- [ ] Add failing tests for gearbox wear from repeated conflict and no arbitrary wear from clean shifts.
- [ ] Verify RED.
- [ ] Add clutch and gearbox health channels without changing existing channel meanings.
- [ ] Implement `FPinkCabDrivetrainCondition` thermal/wear accumulator.
- [ ] Bridge health multipliers into torque/engagement capability only through authored consequences.
- [ ] Re-run vehicle health tests to GREEN.
- [ ] Commit.

## Task 8 — Persistence schema/migration

- [ ] Add failing tests for new drivetrain health/thermal round-trip.
- [ ] Add failing test that load is not a free repair.
- [ ] Add failing test that transient grip/handbrake pull/active throttle command is not restored as a held control.
- [ ] Add failing test that a loaded stationary vehicle requires fresh launch throttle dosing.
- [ ] Add migration test for previous vehicle snapshot schema defaults.
- [ ] Verify RED.
- [ ] Bump vehicle snapshot schema and explicitly migrate previous channel count/state.
- [ ] Persist permanent wear and chosen thermal state needed to prevent save/load exploit.
- [ ] Re-run all persistence tests to GREEN.
- [ ] Commit.
## Task 9 — Orchestration and exact-head automated gate

- [ ] Integrate classifier, launch, steering, handbrake and gearbox/drivetrain controllers into `APinkCabChaosTatraPawn`.
- [ ] Keep pawn orchestration thin; move rule logic out if a method becomes multi-rule.
- [ ] Add/extend telemetry for motion mode, launch serial, requested/engaged gear, clutch coupling, handbrake command, heat/wear and zero-assist flags.
- [ ] Add exact-head automation tests covering QA-01..QA-19 wherever technically automatable without a human/model.
- [ ] Run PinkCab.Vehicle, PinkCab.Cockpit and PinkCab.Persistence automation suites.
- [ ] Run `scripts/check-authority.sh`.
- [ ] Run canonical `scripts/build.ps1`.
- [ ] Scan produced logs for fatal/assert/ensure/error relevant to the test run.
- [ ] Fix regressions and repeat until exact HEAD is green.
- [ ] Commit.

## Task 10 — Authority, review and handoff

- [ ] Update release contract with STATIONARY/MOVING/LAUNCH classification and accepted dual handbrake semantics.
- [ ] Update handling QA with heavy stationary steering, bounded virtual cursor and moving handbrake auto-return checks.
- [ ] Update Jira CD-611/CD-649/CD-653/CD-659/CD-643/CD-644/CD-740/CD-848 evidence/status without claiming manual verification.
- [ ] Update Confluence page 16744449 with the approved runtime semantics and exact implementation SHA.
- [ ] Run `git diff --check`, authority guard, relevant automation and canonical build again on final HEAD.
- [ ] Request independent code review before merge.
- [ ] Open/update PR with exact automated evidence and explicit manual/model gates NOT RUN.
- [ ] Leave owner handling/model tests as the next phase.
