# PINK CAB Modular Cockpit, Code Health & Playable Build Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship an exact-head Win64 build that starts in a modular primitive driver cockpit and visibly drives the taxi, while resolving actionable static-analysis findings and decomposing proven production monoliths.

**Architecture:** Keep vehicle/taxi/world state authoritative. `APinkCabChaosTatraPawn` becomes a composition root around focused cockpit assembly, interaction and presentation units; replaceable visual slots translate authoritative state into primitive/final visuals without owning gameplay truth.

**Tech Stack:** Unreal Engine 5.8.2, C++20/UBT, Chaos Vehicles current placeholder dynamics, Unreal Automation Tests, Visual C++ static analyzer, BuildCookRun Win64 Development.

**Spec:** `docs/superpowers/specs/2026-09-14-pink-cab-modular-cockpit-code-health-design.md`

## Global Constraints

- Canonical input: mouse steering; Space GAZE; 1 signals / 2 horn / 3 gearbox / 4 handbrake; LMB ATTENTION; RMB GO; Q clutch / W brake / E throttle.
- Primitive visuals never own authoritative vehicle/taxi/world state.
- Final asset replacement must be slot/config assignment, not gameplay rewrite.
- New cockpit classes must be focused; the pawn must not become a cockpit monolith.
- TDD for behavior changes: failing test, verify RED, minimal implementation, verify GREEN, commit.
- Exact-head acceptance requires build, automation, static analysis, package and packaged smoke evidence.

---

### Task 1: Establish code-health baseline and branch evidence

**Files:**
- Create: `docs/verification/CD-827-code-health.md`
- No production code changes.

**Interfaces:**
- Consumes: exact pre-gate head `ac62ecb`.
- Produces: analyzer baseline and production hotspot classification used by later refactor tasks.

- [ ] Run `Build.bat PinkCabEditor Win64 Development <uproject> -WaitMutex -NoHotReloadFromIDE -StaticAnalyzer=VisualCpp -StaticAnalyzerProjectOnly` and save stdout under `Artifacts/Analysis/baseline-visualcpp.txt`.
- [ ] Inventory production `.h/.cpp` LOC and classify each hotspot >=230 LOC by responsibility, not size alone.
- [ ] Record every analyzer diagnostic as project defect, engine/toolchain noise or accepted-with-reason.
- [ ] Commit the baseline report as `docs(CD-827): record cockpit gate code-health baseline`.

### Task 2: Define stable cockpit slot and presentation contracts

**Files:**
- Create: `Source/PinkCab/Public/Cockpit/PinkCabCockpitSlot.h`
- Create: `Source/PinkCab/Public/Cockpit/PinkCabCockpitPresentationState.h`
- Test: `Source/PinkCabTests/Private/Cockpit/PinkCabCockpitContractTests.cpp`

**Interfaces:**
- Produces: `EPinkCabCockpitSlot`, `FPinkCabCockpitSlotDefinition`, `FPinkCabCockpitPresentationState`.
- Presentation state contains normalized steering/pedals, speed/RPM/gear, ignition/handbrake/door/meter and simple auxiliary-control state; it does not mutate gameplay.

- [ ] Write a failing automation test asserting every required stable slot ID exists and definitions reject duplicate IDs.
- [ ] Run `PinkCab.Cockpit.Contract` and verify RED because the contract types do not exist.
- [ ] Implement the slot enum/definition and read-only presentation state with no UObject ownership.
- [ ] Run the contract test and existing `PinkCab.Vehicle` tests; verify GREEN.
- [ ] Commit `feat(CD-824): define replaceable cockpit slot contract`.
### Task 3: Build cockpit assembly, driver camera and primitive shell

**Files:**
- Create: `Source/PinkCab/Public/Cockpit/PinkCabCockpitAssemblyComponent.h`
- Create: `Source/PinkCab/Private/Cockpit/PinkCabCockpitAssemblyComponent.cpp`
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabChaosTatraPawn.h`
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp`
- Test: `Source/PinkCabTests/Private/Cockpit/PinkCabCockpitRuntimeContractTests.cpp`

**Interfaces:**
- `UPinkCabCockpitAssemblyComponent::GetSlotComponent(EPinkCabCockpitSlot)` returns the replaceable visual/anchor component for a semantic slot.
- `GetDriverCamera()` returns the only active first-person camera for normal play.

- [ ] Write a failing runtime contract test asserting a spawned Tatra pawn owns the assembly, has all required anchors, and driver camera is active while the old chase camera is inactive.
- [ ] Verify RED on `PinkCab.Cockpit.Runtime.Contract`.
- [ ] Implement the assembly component with Engine primitive fallbacks for floor, dash, A-pillars/roof header, steering wheel, pedals, gear lever, handbrake, meter and switch/instrument surfaces.
- [ ] Attach the assembly to the vehicle mesh/root with collision disabled for interior presentation parts so the camera is never occluded by cabin primitives.
- [ ] Move driver head/look camera ownership from chase-boom logic into the assembly; keep only a temporary debug chase camera if needed and ensure it is inactive by default.
- [ ] Run cockpit contract/runtime tests plus `PinkCab.Vehicle.ChaosBaseline`; verify GREEN.
- [ ] Commit `feat(CD-824): add modular primitive driver cockpit`.

### Task 4: Canonical input ownership and functional vehicle controls

**Files:**
- Create: `Source/PinkCab/Public/Cockpit/PinkCabCockpitInteractionComponent.h`
- Create: `Source/PinkCab/Private/Cockpit/PinkCabCockpitInteractionComponent.cpp`
- Modify: `Source/PinkCab/Public/Interaction/PinkCabSemanticInputRouter.h`
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabCockpitState.h`
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabCockpitInteractionRouter.h`
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp`
- Test: `Source/PinkCabTests/Private/Cockpit/PinkCabCockpitInputTests.cpp`

**Interfaces:**
- `docs/PROJECT_SETUP.md`, `docs/PINK_CAB_TATRA_DAUGHTER_CONDUCTOR_CANON.md` and `docs/VERIFICATION_MATRIX.md` `PC-T-INP-001..008` are the authority for this task.
- Interaction component owns one bounded current target, quick recall, optional grip, momentary press/hold, wheel command emission and transient ownership cleanup only; command application stays in existing state/router paths.
- Existing `FPinkCabVehicleInputFrame` remains the continuous pedal/control source.

- [x] Write failing compliance tests for all `PC-T-INP-001..008`, including focus-loss cleanup and absence of unauthorized shortcuts.
- [x] Verify RED before routing changes.
- [x] Bind `Space=GAZE`, `1..4=quick recall`, `RMB=grip`, `LMB=momentary press/hold`, wheel=contextual signed adjustment, `Q/W/E=clutch/brake/throttle`; remove `ATTENTION/GO` production semantics.
- [x] Implement bounded gaze target selection from registered cockpit interaction slots only; no world Actor scan.
- [x] Add per-target interaction metadata (`supports grip`, `supports momentary`, `supports wheel`) and reject unsupported gestures before command emission.
- [x] Quick recall selects/restores target/hand pose only and never actuates.
- [x] Implement grip begin/end without implicit actuation and LMB press/release with correct momentary semantics; horn must expose short/long hold duration/state.
- [x] Implement focus-loss/input-recovery cleanup so gaze/grip/momentary/wheel ownership cannot remain stuck.
- [x] Remove all direct player-facing physical-control shortcuts not listed in authority, including the `R` pawn-reset path.
- [x] Extend cockpit state/router with minimal authoritative auxiliary states for signals, horn actuation, lights, wipers and washer while preserving ignition/gear/handbrake/door/meter behavior.
- [x] Refactor pawn tick so it gathers raw input and delegates target search/interaction/continuous-control work instead of containing interaction policy inline.
- [x] Run `PinkCab.Cockpit.Input`, `PinkCab.Interaction`, `PinkCab.Vehicle` and exact `PC-T-INP-001..008` acceptance; verify GREEN.
- [x] Commit `fix(CD-825): conform cockpit input to canonical authority`.
### Task 5: Drive cockpit visuals from authoritative telemetry/state

**Files:**
- Create: `Source/PinkCab/Public/Cockpit/PinkCabCockpitVisualDriverComponent.h`
- Create: `Source/PinkCab/Private/Cockpit/PinkCabCockpitVisualDriverComponent.cpp`
- Test: `Source/PinkCabTests/Private/Cockpit/PinkCabCockpitVisualDriverTests.cpp`

**Interfaces:**
- Consumes `FPinkCabCockpitPresentationState`.
- Produces only local visual transforms/visibility; no gameplay mutation.

- [ ] Write failing tests for steering-wheel angle, pedal travel, gear-lever pose, handbrake pose and ignition/meter/door indicator visibility from presentation values.
- [ ] Verify RED.
- [ ] Implement deterministic mapping functions and a component that applies them to semantic slots from the assembly.
- [ ] Populate presentation state each frame from existing vehicle telemetry and cockpit state; do not duplicate calculations already owned by dynamics or taxi systems.
- [ ] Run visual-driver tests and vehicle regression; verify GREEN.
- [ ] Commit `feat(CD-824): drive cockpit visuals from authoritative state`.

### Task 6: Integrate taxi meter, passenger door, navigation/radio and mirror contracts

**Files:**
- Create: `Source/PinkCab/Public/Cockpit/PinkCabCockpitServiceBridge.h`
- Create: `Source/PinkCab/Private/Cockpit/PinkCabCockpitServiceBridge.cpp`
- Modify only the smallest existing taxi/world interfaces needed to expose read-only presentation values.
- Test: `Source/PinkCabTests/Private/Cockpit/PinkCabCockpitServiceBridgeTests.cpp`

**Interfaces:**
- Service bridge reads authoritative fare/taximeter, passenger-door, route/progress and radio/mirror capability state.
- It returns presentation DTOs only; it cannot settle fares, mutate routes or own media playback truth.

- [ ] Write failing tests proving meter display follows existing authoritative meter/fare state, door display follows cockpit state, and route/radio/mirror surfaces fail closed when no service source is present.
- [ ] Verify RED.
- [ ] Implement read-only bridge/adapters with explicit unavailable states instead of invented values.
- [ ] Connect primitive taximeter, door lever, navigation/radio surfaces and mirror anchors to the bridge.
- [ ] Run `PinkCab.Cockpit.ServiceBridge`, `PinkCab.Taxi` and relevant world tests; verify GREEN.
- [ ] Commit `feat(CD-826): connect cockpit taxi and service surfaces`.

### Task 7: Decompose proven production monoliths

**Files:**
- Modify only hotspots classified `split now` in `docs/verification/CD-827-code-health.md`.
- Add focused headers/helpers adjacent to their owning subsystem; preserve public entry points where possible.
- Tests: existing subsystem suites plus targeted extraction tests where behavior boundaries need explicit coverage.

**Interfaces:**
- Public behavior and snapshot/schema semantics remain unchanged unless a separately tested defect is found.

- [ ] For each `split now` hotspot, write/identify a regression test that freezes current public behavior before extraction.
- [ ] Extract one responsibility at a time: serialization/validation/replay/orchestration/policy are separate candidates, never mixed in one mega-helper.
- [ ] Run the owning subsystem tests after each extraction and commit each coherent split separately.
- [ ] Recompute production LOC/responsibility inventory and update the report with before/after ownership.
- [ ] Run full relevant regression prefixes after the final split.
### Task 8: Exact-head runtime movement acceptance

**Files:**
- Create: `Source/PinkCabTests/Private/Cockpit/PinkCabCockpitPlayableRuntimeTests.cpp`
- Modify: playable dev-map generation only if a deterministic cockpit-ready pawn/course is not already persisted.

**Interfaces:**
- Test observes real possessed pawn, active driver camera, live dynamics and measurable displacement.

- [x] Write the runtime test first and verify it fails on missing cockpit/camera/canonical-input assertions before implementation is complete.
- [x] Prime ignition and release handbrake through the same physical cockpit command path used by the player; no hidden keyboard control path.
- [x] Apply non-zero throttle through the canonical `E`/vehicle-input path for a bounded duration and assert forward displacement plus valid telemetry.
- [x] Exercise Space gaze→steering return and transient input cleanup during the live drive; camera and steering ownership must remain valid.
- [x] Assert no player-facing free-reset/teleport shortcut exists. Terminal recovery remains owned by the authoritative RecoveryPolicy/Workday/Repair flow and is not faked by this movement test.
- [x] Run `PinkCab.Cockpit.Playable.Runtime`, exact `PC-T-INP-001..008` acceptance and existing `PinkCab.Core.CodeComplete.Runtime`; verify GREEN.
- [x] Commit `test(CD-828): prove canonical playable cockpit taxi movement`.

### Task 9: Final analyzer, clean build and package

**Files:**
- Update: `docs/verification/CD-827-code-health.md`
- No speculative production changes after verification begins.

- [x] Run `git diff --check` and verify clean whitespace.
- [x] Run final UBT VisualCpp project-only static analysis and save `Artifacts/Analysis/final-visualcpp.txt`.
- [x] Resolve any new actionable project diagnostic, then repeat the analyzer until the project finding set is clean/accepted-with-reason.
- [x] Force a clean `PinkCabEditor Win64 Development` build.
- [x] Run `PinkCab.Cockpit`, `PinkCab.Interaction`, `PinkCab.Vehicle`, `PinkCab.Taxi`, `PinkCab.Persistence`, `PinkCab.Core.CodeComplete` and any subsystem suites touched by monolith splits.
- [x] Run `scripts/build.ps1 -Package`; require `BUILD SUCCESSFUL` and ExitCode 0.
- [x] Launch the packaged executable against the intended playable map, keep it alive long enough for map load/engine init, and reject any fatal/assert/crash.
- [x] Update desktop shortcut `PinkCab.lnk` only after the accepted package exists and target verification succeeds.
- [x] Commit `chore(CD-828): close playable cockpit build evidence`.

### Task 10: Synchronize Jira, Confluence and Git closure evidence

**Files:**
- Update local spec/plan verification references only if exact implementation differs from the approved contract.

- [ ] Add exact branch/head, analyzer counts/classification, test counts, package path and packaged-smoke evidence to CD-823..CD-829.
- [ ] Update Confluence page 41 with final implementation/file ownership and evidence; do not rewrite higher-level authority pages unless a contradiction was explicitly resolved.
- [ ] Transition completed execution units and CD-823 only after exact-head verification.
- [ ] Verify local `main` integration status separately from remote origin; never claim remote publication until push is verified.

## Self-review

- Spec coverage: all slot/replaceability, input, functional cabin, taxi-service, code-health, movement and package requirements map to Tasks 1–10.
- Placeholder scan: no TBD/TODO/"implement later" steps are accepted.
- Type consistency: new cockpit units consume authoritative state and never own fare/route/dynamics truth.
- Execution mode: inline execution in this session, because the user explicitly requested implementation/admin/build rather than a planning-only handoff.
