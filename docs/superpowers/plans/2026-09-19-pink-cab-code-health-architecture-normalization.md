# PINK CAB Code Health & Architecture Normalization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Convert PINK CAB from a folder-organized monolith into an acyclic, machine-enforced architecture where a new Unreal/C++ developer can find the owner, contract and tests for a change within five minutes without changing accepted gameplay behavior.

**Architecture:** Stabilize the current CD-848 real-input repair first, then install a repository-owned architecture analyzer and ownership manifest before moving code. Decompose `APinkCabChaosTatraPawn` into input, vehicle-control, Chaos-adapter and presentation boundaries, break cross-domain cycles, normalize persistence DTO ownership, and only then create asset-safe physical UE modules.

**Tech Stack:** Unreal Engine 5.8.2, C++20/UBT, native Chaos Vehicles, Unreal Automation Tests, Python 3 standard library, PowerShell, Visual C++ project-only static analysis.

**Spec:** `docs/superpowers/specs/2026-09-19-pink-cab-code-health-architecture-normalization-design.md`

## Global Constraints

- No behavior drift: steering, clutch, throttle dosing, brake dosing, H-gate, handbrake and current taxi/persistence semantics remain unchanged unless a separate approved gameplay task changes them.
- Canonical driver-space axes: `+X = right`, gearbox `+Y = forward/top 1/3/5`, gearbox `-Y = rear/bottom 2/4/R`.
- New launch still requires `E + wheel` throttle dosing; do not reintroduce automatic 45% throttle.
- Raw keyboard/mouse/device coordinate interpretation must end in Interaction/Input only.
- Direct Chaos control writes must end in the Vehicle Chaos adapter only.
- Domain/module dependency SCCs larger than one must be zero at final acceptance.
- Production `.cpp` target <=350 non-comment LOC; `.h` <=250; ordinary functions <=80 LOC; cyclomatic complexity <=15 unless explicitly allowlisted with rationale.
- Preserve `/Script/PinkCab.*` reflected compatibility while asset-safe migration is performed.
- TDD/characterization-first for behavior and refactor seams: RED -> minimal GREEN -> refactor -> affected suite -> commit.
- No architecture phase may start on top of an unexplained RED test from the previous phase.
- Exact-head completion requires build, analyzer, full automation, real-input runtime acceptance, package smoke and independent review.

## Review Focus

1. Real PlayerController input timing: key state is asserted after a frame boundary, not immediately after `InputKey`.
2. Launch sequencing: throttle is dosed while clutch is held before progressive clutch release, so the test does not create an artificial zero-throttle stall.
3. Reflected UE type movement: moving source files must not silently change script/module identity or break asset references.
4. Analyzer false positives: generated/declarative data and compatibility shims require explicit narrow allowlist entries, never blanket directory exclusions.
5. Dependency regression: indirect include back-edges must be detected from the full graph, not only direct forbidden strings.

---
### Task 1: Stabilize the current CD-848 real-input repair

**Files:**
- Modify: `Source/PinkCab/Public/Interaction/PinkCabPhysicalInputConvention.h`
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabGearboxController.h`
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabSteeringController.h`
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp`
- Modify: `Source/PinkCabTests/Private/Vehicle/PinkCabPlayerInputRuntimeTests.cpp`
- Modify only as required by the existing sign contract: current cockpit/core/Chaos/control tests already dirty in the worktree.

**Interfaces:**
- Produces: `FPinkCabPhysicalInputConvention::ResolveActiveDeviceAxis(float Processed, float Raw)`.
- Produces: `FPinkCabPhysicalInputConvention::SteeringRight(float DeviceX)`.
- Produces: `FPinkCabPhysicalInputConvention::GearboxForward(float DeviceY)`.
- Produces: `FPinkCabPhysicalInputConvention::HandbrakePull(float DeviceY)`.
- Produces: `FPinkCabGearboxController::ApplyLeverDriverDelta(float DriverRightCounts, float DriverForwardCounts)`.

- [ ] **Step 1: Correct the existing RED E2E fixture before any new production change.** Keep clutch fully depressed while engaging first, press E through `APlayerController::InputKey`, dose five wheel steps over separate frames, verify target throttle reaches 0.25 while engine remains running, then release clutch progressively.
- [ ] **Step 2: Move immediate key assertions one frame later.** The test must assert `PC->IsInputKeyDown(EKeys::E)` only after Unreal has processed a frame, and likewise assert release after the next frame.
- [ ] **Step 3: Run the real-input test and verify the failure is now a real gameplay-path mismatch, not fixture-induced stall.**

Run:
```powershell
UnrealEditor-Cmd.exe PinkCab.uproject -unattended -nop4 -NullRHI -ExecCmds="Automation RunTests PinkCab.Vehicle.PlayerInput.ControllerThrottleDrive;Quit"
```

Expected before production fix: RED only if the real controller/Tick path still fails to deliver throttle/motion.
- [ ] **Step 4: If RED remains, instrument only read-only test observability.** Add narrow getters for launch throttle target / dose-required / engine-running state only when the failing assertion cannot otherwise identify the first mismatch; do not add new gameplay state.
- [ ] **Step 5: Make the minimum production correction at the first proven mismatch.** Device sign remains owned by `FPinkCabPhysicalInputConvention`; backend Chaos steering conversion may remain in the Chaos provider because it is engine-space, not player-space.
- [ ] **Step 6: Verify targeted GREEN.**

Run:
```powershell
Automation RunTests PinkCab.Vehicle.PlayerInput.ControllerThrottleDrive
Automation RunTests PinkCab.Vehicle.ControlRuntime
Automation RunTests PinkCab.Cockpit.Input.Compliance
Automation RunTests PinkCab.Cockpit.Playable.Runtime
```

Expected: all targeted tests PASS; gearbox top/bottom row direction and previously accepted steering direction both remain correct.

- [ ] **Step 7: Build and commit only the stabilized CD-848 repair.**

Run:
```powershell
.\scripts\build.ps1
git diff --check
git add Source/PinkCab Source/PinkCabTests
git commit -m "fix(CD-848): stabilize physical input convention"
```

Commit boundary: no architecture extraction in this commit.

### Task 2: Install the repository code-health analyzer and regression ratchet

**Files:**
- Create: `scripts/code_health.py`
- Create: `scripts/tests/test_code_health.py`
- Create: `scripts/code-health.ps1`
- Create: `Config/CodeHealthPolicy.json`
- Create: `Config/CodeHealthBaseline.json`
- Create: `Saved/Reports/.gitkeep` only if the reports directory otherwise cannot be represented; generated report files remain ignored.

**Interfaces:**
- Produces: `analyze_repository(root: pathlib.Path, policy: dict) -> dict`.
- Produces CLI: `python scripts/code_health.py --root . --policy Config/CodeHealthPolicy.json --baseline Config/CodeHealthBaseline.json --report Saved/Reports/code-health.json --check`.
- Exit 0 = no regression versus baseline; exit 2 = architecture/policy violation; exit 1 = analyzer/tool error.
- [ ] **Step 1: Write analyzer unit tests first.** Tests must create temporary C++ trees and assert detection of: file LOC limit, function length, complexity, non-trivial business logic in Public headers, forbidden raw-input API usage, forbidden direct Chaos writes, forbidden Persistence dependency, strict allowlist schema/broad-exclusion rejection, and a three-node include cycle.

Example test contract:
```python
def test_detects_domain_cycle(tmp_path):
    write(tmp_path, "Source/PinkCab/Public/Vehicle/A.h", '#include "Taxi/B.h"\n')
    write(tmp_path, "Source/PinkCab/Public/Taxi/B.h", '#include "Persistence/C.h"\n')
    write(tmp_path, "Source/PinkCab/Public/Persistence/C.h", '#include "Vehicle/A.h"\n')
    report = analyze_repository(tmp_path, policy())
    assert ["Persistence", "Taxi", "Vehicle"] in report["dependency_sccs"]
```

- [ ] **Step 2: Run analyzer tests and verify RED because `code_health.py` does not exist.**

Run:
```powershell
python -m unittest scripts.tests.test_code_health -v
```

Expected: import/file failure.

- [ ] **Step 3: Implement the minimum analyzer using Python stdlib only.** Parse `#include "Domain/..."`, non-comment LOC, brace-delimited function bodies, branch tokens, and Tarjan/Kosaraju SCC. Report exact file/line evidence for every violation.
- [ ] **Step 4: Add policy JSON with explicit domain mapping and forbidden ownership patterns.** Do not use broad wildcard exclusions. Allowlist entries require `path`, `rule`, `reason`, and `expires_when`.
- [ ] **Step 5: Run unit tests GREEN, then run analyzer against current source in baseline-generation mode.**
- [ ] **Step 6: Commit the exact observed debt to `Config/CodeHealthBaseline.json`.** Baseline records counts/identities, not a pass flag. Future `--check` fails if any count or violating file worsens.
- [ ] **Step 7: Wrap local/CI invocation in `scripts/code-health.ps1` and verify exit codes.**
- [ ] **Step 8: Commit.**

```powershell
git add scripts/code_health.py scripts/tests/test_code_health.py scripts/code-health.ps1 Config/CodeHealthPolicy.json Config/CodeHealthBaseline.json
git commit -m "build: add architecture code-health ratchet"
```

### Task 3: Publish the five-minute ownership map

**Files:**
- Create: `Config/ArchitectureOwnership.json`
- Create: `docs/ENGINEERING_START_HERE.md`
- Modify: `scripts/code_health.py`
- Modify: `scripts/tests/test_code_health.py`
**Interfaces:**
- `ArchitectureOwnership.json` maps each concern to `owner_domain`, `public_contract`, `implementation_root`, and `test_prefix`.
- Analyzer verifies every configured path exists and no concern has multiple authoritative owners.

Required concern rows:
`steering`, `gearbox`, `clutch`, `throttle`, `brake`, `handbrake`, `device_input_sign`, `chaos_translation`, `fare`, `passenger`, `economy`, `vehicle_snapshot`, `game_persistence`, `world_route`, `traffic`, `cockpit_presentation`.

- [ ] **Step 1: Add failing analyzer tests for missing owner paths and duplicate owners.**
- [ ] **Step 2: Verify RED.**
- [ ] **Step 3: Implement manifest validation and write the exact current ownership manifest.**
- [ ] **Step 4: Write `ENGINEERING_START_HERE.md` from the accepted spec.** Include project path, UE version, canonical build/test commands, dependency rules, and an explicit “I want to change X -> go here” table sourced from the manifest.
- [ ] **Step 5: Add a doc-sync check:** every concern ID in the manifest must appear in the onboarding table; stale/missing rows fail code-health.
- [ ] **Step 6: Run analyzer/tests GREEN and commit.**

```powershell
python -m unittest scripts.tests.test_code_health -v
.\scripts\code-health.ps1
git add Config/ArchitectureOwnership.json docs/ENGINEERING_START_HERE.md scripts
git commit -m "docs: add five-minute engineering ownership map"
```

### Task 4: Extract real player-input capture from the Pawn

**Files:**
- Create: `Source/PinkCab/Public/Interaction/PinkCabPlayerInputSample.h`
- Create: `Source/PinkCab/Public/Interaction/PinkCabPlayerInputAdapter.h`
- Create: `Source/PinkCab/Private/Interaction/PinkCabPlayerInputAdapter.cpp`
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp`
- Modify: `Source/PinkCabTests/Private/Vehicle/PinkCabPlayerInputRuntimeTests.cpp`
- Create: `Source/PinkCabTests/Private/Interaction/PinkCabPlayerInputAdapterTests.cpp`

**Interfaces:**
```cpp
struct FPinkCabPlayerInputSample
{
    FPinkCabVehicleInputFrame VehicleFrame;
    float DeviceX = 0.0f;
    float DeviceY = 0.0f;
    int32 WheelSteps = 0;
};

class FPinkCabPlayerInputAdapter
{
public:
    FPinkCabPlayerInputSample Capture(
        APlayerController& Controller,
        const FPinkCabSemanticInputRouter& Router) const;
};
```
- [ ] **Step 1: Write adapter tests first.** Cover E/W/Q coexistence, wheel-step sign, stale raw-axis rejection, processed-zero/raw-stale suppression, and canonical `+X/+Y` driver-space directions.
- [ ] **Step 2: Verify RED because the adapter does not exist.**
- [ ] **Step 3: Implement capture by moving only PlayerController/raw-device reading out of Pawn Tick.** Use `FPinkCabPhysicalInputConvention` for device-axis normalization; no gameplay policy in the adapter.
- [ ] **Step 4: Replace Pawn raw key/mouse reads with one `Capture()` call.**
- [ ] **Step 5: Run `PinkCab.Interaction`, `PinkCab.Vehicle.PlayerInput`, `PinkCab.Cockpit.Input.Compliance` and code-health.**
- [ ] **Step 6: Verify analyzer reports zero raw-input ownership violations outside Interaction.**
- [ ] **Step 7: Commit.**

```powershell
git add Source/PinkCab/Public/Interaction Source/PinkCab/Private/Interaction Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp Source/PinkCabTests
git commit -m "refactor: isolate player input capture"
```

### Task 5: Extract vehicle control orchestration from `APinkCabChaosTatraPawn`

**Files:**
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleControlRuntime.h`
- Create: `Source/PinkCab/Private/Vehicle/PinkCabVehicleControlRuntime.cpp`
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabChaosTatraPawn.h`
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp`
- Create: `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleControlRuntimeTests.cpp`

**Interfaces:**
```cpp
struct FPinkCabVehicleControlInput
{
    FPinkCabVehicleInputFrame Frame;
    float DriverMouseX = 0.0f;
    float DriverMouseY = 0.0f;
    int32 WheelSteps = 0;
    float DeltaSeconds = 0.0f;
};

struct FPinkCabVehicleControlOutput
{
    FPinkCabVehicleControlState Controls;
    FVector2D GearLeverCursor = FVector2D::ZeroVector;
    int32 RequestedGear = 0;
    int32 EngagedGear = 0;
};

class FPinkCabVehicleControlRuntime
{
public:
    FPinkCabVehicleControlOutput Update(
        const FPinkCabVehicleControlInput& Input,
        const FPinkCabVehicleTelemetry& Telemetry,
        FPinkCabCockpitState& Cockpit,
        FPinkCabVehicleHealthState& Health);
};
```
- [ ] **Step 1: Write characterization tests against the current Pawn control behavior before moving it.** Cover launch reset exactly once, 5% throttle dose increments, E>W>Q wheel priority, steering sign, H-gate requested/engaged separation, analog handbrake, and low-RPM stall.
- [ ] **Step 2: Verify the new runtime tests RED because `FPinkCabVehicleControlRuntime` does not exist while the existing Pawn tests remain GREEN.**
- [ ] **Step 3: Move controller ownership into the runtime:** `MotionClassifier`, `SteeringController`, `LaunchController`, `PedalDosingController`, `HandbrakeActuator`, `GearboxController`, `DrivetrainCondition` and the associated smoothing state.
- [ ] **Step 4: Keep Pawn public compatibility getters as thin delegates while tests migrate.** Example: `GetEngagedGear() { return VehicleControlRuntime.GetEngagedGear(); }`.
- [ ] **Step 5: Make `ApplyVehicleInputFrame` a compatibility delegation only; no duplicated control calculations remain in Pawn.**
- [ ] **Step 6: Run control runtime, drivetrain, cockpit playable and real PlayerController suites GREEN.**
- [ ] **Step 7: Run code-health and verify Pawn LOC/complexity decreases without a new oversized runtime function.**
- [ ] **Step 8: Commit.**

```powershell
git add Source/PinkCab/Public/Vehicle/PinkCabVehicleControlRuntime.h Source/PinkCab/Private/Vehicle/PinkCabVehicleControlRuntime.cpp Source/PinkCab/Public/Vehicle/PinkCabChaosTatraPawn.h Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp Source/PinkCabTests
git commit -m "refactor: extract vehicle control runtime"
```

### Task 6: Make the existing DynamicsProvider the sole Chaos write boundary

**Files:**
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabVehicleDynamicsProvider.h`
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabChaosVehicleDynamicsProvider.h`
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosVehicleDynamicsProvider.cpp`
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp`
- Modify: `scripts/code_health.py`
- Modify: `scripts/tests/test_code_health.py`
- Test: existing `PinkCab.Vehicle.ChaosBaseline`, `PinkCab.Vehicle.ChaosRuntime`, `PinkCab.Vehicle.PlayerInput`.

**Interfaces:**
- Existing `IPinkCabVehicleDynamicsProvider::ApplyControls(const FPinkCabVehicleControlState&)` remains the single control write API.
- Existing `ReadTelemetry(FPinkCabVehicleTelemetry&)` remains the read API.
- Pawn may obtain the movement component for construction/setup only; gameplay-frame calls to `SetSteeringInput`, `SetThrottleInput`, `SetBrakeInput`, `SetHandbrakeInput`, `SetDriveTorque`, `SetBrakeTorque` and `SetTorqueCombineMethod` are forbidden outside the Chaos provider.
- [ ] **Step 1: Add failing analyzer tests for every forbidden Chaos write API outside `PinkCabChaosVehicleDynamicsProvider.cpp`.**
- [ ] **Step 2: Run analyzer unit tests RED.**
- [ ] **Step 3: Extend analyzer rule and run it on production; record each real offender.**
- [ ] **Step 4: Route each gameplay-frame write through `ApplyControls`; do not create a second Chaos bridge.**
- [ ] **Step 5: Keep the semantic steering sign positive-right; the only chassis-space inversion remains inside `FPinkCabChaosVehicleDynamicsProvider::ApplyControls`.**
- [ ] **Step 6: Run Chaos baseline/runtime plus real-input tests and verify wheel contact, forward motion, reverse motion and normalized telemetry remain GREEN.**
- [ ] **Step 7: Run code-health and require zero direct Chaos-write violations.**
- [ ] **Step 8: Commit.**

```powershell
git add Source/PinkCab/Public/Vehicle Source/PinkCab/Private/Vehicle scripts
git commit -m "refactor: enforce single Chaos dynamics adapter"
```

### Task 7: Move UI/input-mode and runtime composition out of Vehicle ownership

**Files:**
- Create: `Source/PinkCab/Public/Runtime/PinkCabDriverUiComponent.h`
- Create: `Source/PinkCab/Private/Runtime/PinkCabDriverUiComponent.cpp`
- Move primary class file: `Source/PinkCab/Public/Vehicle/PinkCabChaosTatraPawn.h` -> `Source/PinkCab/Public/Runtime/PinkCabChaosTatraPawn.h`
- Move implementation: `Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp` -> `Source/PinkCab/Private/Runtime/PinkCabChaosTatraPawn.cpp`
- Update internal includes in production/tests to `Runtime/PinkCabChaosTatraPawn.h`.
- Do not change the C++ class name or UE module, so the reflected class remains `/Script/PinkCab.PinkCabChaosTatraPawn`.

**Interfaces:**
```cpp
class UPinkCabDriverUiComponent : public UActorComponent
{
public:
    void SetSystemMenuOpen(APlayerController& PC, bool bOpen);
    void ResetTransientInput(APlayerController& PC);
    bool IsSystemMenuOpen() const;
};
```
- [ ] **Step 1: Add a failing runtime test asserting the pawn's reflected class path before/after source relocation is exactly `/Script/PinkCab.PinkCabChaosTatraPawn`.**
- [ ] **Step 2: Add focused UI component tests for menu open/close, GameOnly vs menu input mode, cursor/capture restoration and focus-loss cleanup.**
- [ ] **Step 3: Verify RED because the component/runtime path does not exist.**
- [ ] **Step 4: Extract `MountPlayableHud`, `UnmountPlayableHud`, `MountSystemMenu`, `UnmountSystemMenu`, `ApplyGameplayInputMode`, `ApplySystemMenuInputMode`, pointer-capture and application-deactivation handling into the UI component.**
- [ ] **Step 5: Move Pawn source/header to `Runtime` without renaming the UCLASS or module.** Update includes and UHT generated include remains `PinkCabChaosTatraPawn.generated.h`.
- [ ] **Step 6: Pawn Tick is reduced to capture -> control update -> dynamics apply -> presentation sync plus thin lifecycle delegation.**
- [ ] **Step 7: Run cockpit, vehicle, real-input, PIE and code-health suites. Verify the Vehicle domain no longer depends on Cockpit merely because the composition root lived there.**
- [ ] **Step 8: Commit.**

```powershell
git add Source/PinkCab/Public/Runtime Source/PinkCab/Private/Runtime Source/PinkCab Source/PinkCabTests
git commit -m "refactor: move taxi pawn to runtime composition root"
```

### Task 8: Move persistence DTO ownership to domains and keep codecs in Persistence

**Files:**
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleStateSnapshot.h`
- Create: `Source/PinkCab/Public/Taxi/PinkCabFareRuntimeStateSnapshot.h`
- Create: `Source/PinkCab/Public/Service/PinkCabServiceStateSnapshot.h`
- Modify: `Source/PinkCab/Public/Persistence/PinkCabVehicleSnapshot.h`
- Modify: `Source/PinkCab/Public/Persistence/PinkCabFareRuntimeSnapshotTypes.h`
- Modify: `Source/PinkCab/Public/Persistence/PinkCabFareRuntimeSnapshotCodec.h`
- Modify: service snapshot/codec files under `Source/PinkCab/Public/Persistence` or `Public/Service` according to current exact paths.
- Modify: `Source/PinkCab/Public/Persistence/PinkCabGameSnapshot.h`
- Test: existing Persistence Vehicle/Fare/Service/GameSnapshot suites.

**Interfaces:**
- Domain snapshot structs are data-only and version-neutral where possible.
- Persistence codecs own schema versions, archive representation and migrations.
- Gameplay domains never include a Persistence implementation header.
- [ ] **Step 1: Write persistence regression tests that prove the existing binary/logical round trips before type movement.** Existing VehicleSnapshot legacy migration, FareRuntime active/awaiting-payment, Passenger snapshot and Service snapshot tests are mandatory.
- [ ] **Step 2: Split passenger snapshot data from its codec.** Move `FPinkCabPassengerSnapshot` data to Taxi ownership; move `FPinkCabPassengerSnapshotCodec` and `ToPersistedIdentity` conversion into Persistence so `Taxi/PinkCabPassengerSnapshot.h` no longer includes `Persistence/PinkCabPersistedLogicalState.h`.
- [ ] **Step 3: Introduce `FPinkCabVehicleStateSnapshot` as the Vehicle-owned capture/restore DTO.** Pawn/runtime compatibility API uses this type; Persistence maps it to schema-versioned `FPinkCabVehicleSnapshot`.
- [ ] **Step 4: Keep service state data under Service ownership.** If the current `Service/PinkCabServiceSnapshot.h` is already data-only, retain it; move only codec/archive logic that is persistence-specific.
- [ ] **Step 5: Update `FPinkCabGameSnapshot` aggregation to consume domain-owned snapshot data through Persistence codecs without adding reverse includes.**
- [ ] **Step 6: Run all Persistence, Taxi, Service and Vehicle snapshot suites GREEN, including legacy schema migration and atomic-invalid rejection.**
- [ ] **Step 7: Run code-health and require zero `Taxi -> Persistence` and zero `Vehicle -> Persistence` implementation edges.**
- [ ] **Step 8: Commit.**

```powershell
git add Source/PinkCab/Public/Taxi Source/PinkCab/Public/Vehicle Source/PinkCab/Public/Service Source/PinkCab/Public/Persistence Source/PinkCab/Private/Persistence Source/PinkCabTests
git commit -m "refactor: normalize domain snapshot ownership"
```

### Task 9: Break the current five-domain SCC

**Current back-edges to eliminate:**
- `Taxi/PinkCabPassengerSnapshot.h -> Persistence/PinkCabPersistedLogicalState.h`
- runtime Pawn Vehicle folder -> Cockpit presentation/components
- runtime Pawn -> Persistence vehicle snapshot
- `Vehicle/PinkCabVehicleVisualProfile.h -> Cockpit/PinkCabCockpitVisualBinding.h`
- any resulting indirect `Vehicle -> Cockpit -> Taxi -> Vehicle` path.

**Files:**
- Move presentation ownership from `Vehicle/PinkCabVehicleVisualProfile.*` and `Vehicle/PinkCabVehicleVisualShellComponent.*` to the Cockpit/Runtime presentation package while preserving USTRUCT/UCLASS names/module identity.
- Modify includes discovered by analyzer after Tasks 7-8.
- Modify `Config/CodeHealthPolicy.json` only to tighten allowed edges; do not suppress a cycle.

**Interfaces:**
- Runtime composition may depend on Cockpit + Vehicle + Taxi.
- Cockpit may consume read-only Vehicle/Taxi contracts.
- Taxi/Service may consume Vehicle domain contracts.
- Vehicle must not depend on Cockpit, Taxi, Service or Persistence implementation.
- [ ] **Step 1: Add an analyzer fixture reproducing the real five-domain SCC and assert `--check` fails.**
- [ ] **Step 2: Verify RED.**
- [ ] **Step 3: Move visual profile/shell ownership out of Vehicle without changing reflected type names.** Runtime composition owns application; Vehicle exposes only telemetry/control/health/load.
- [ ] **Step 4: Remove all remaining Vehicle -> Cockpit/Persistence implementation includes.**
- [ ] **Step 5: Re-run dependency graph after every back-edge removal; never remove a forward read-contract merely to hide the graph.**
- [ ] **Step 6: Require final SCC report to contain only singleton components.**
- [ ] **Step 7: Run full affected domain suites and UBT build GREEN.**
- [ ] **Step 8: Commit the cycle break separately from physical module migration.**

```powershell
.\scripts\code-health.ps1
.\scripts\build.ps1
git add Source/PinkCab Config/CodeHealthPolicy.json scripts
git commit -m "refactor: make runtime domain graph acyclic"
```

### Task 10: Create the locked asset-safe Unreal module boundaries

**Files:**
- Create runtime modules: `Source/PinkCabCore`, `Source/PinkCabInteraction`, `Source/PinkCabVehicle`, `Source/PinkCabTaxi`, `Source/PinkCabEconomy`, `Source/PinkCabWorld`, `Source/PinkCabTraffic`, `Source/PinkCabPersistence`.
- Each module gets `<Module>.Build.cs`, `Public/`, `Private/<Module>.cpp` with `IMPLEMENT_MODULE(FDefaultModuleImpl, <Module>)`.
- Modify: `PinkCab.uproject`
- Modify: `Source/PinkCab/PinkCab.Build.cs`
- Modify: `Source/PinkCab.Target.cs`
- Modify: `Source/PinkCabEditor.Target.cs`
- Modify: `Source/PinkCabTests/PinkCabTests.Build.cs`
- Move only asset-safe non-reflected or explicitly verified types on this pass.

**Module dependency contract:**
```text
PinkCabCore        -> Engine basics only
PinkCabInteraction -> PinkCabCore + InputCore/EnhancedInput
PinkCabVehicle     -> PinkCabCore + ChaosVehicles/PhysicsCore
PinkCabEconomy     -> PinkCabCore
PinkCabWorld       -> PinkCabCore
PinkCabTraffic     -> PinkCabCore + PinkCabWorld
PinkCabTaxi        -> PinkCabCore + PinkCabEconomy + PinkCabVehicle + PinkCabWorld
PinkCabPersistence -> PinkCabCore + domain public snapshot contracts
PinkCab            -> all required domain modules + reflected compatibility/composition
```
- [ ] **Step 1: Add a code-health test that parses `*.Build.cs` dependencies and fails on a module cycle or dependency outside the contract.**
- [ ] **Step 2: Verify RED against the current single-module layout because the required module map is absent.**
- [ ] **Step 3: Create empty buildable modules first and add them to the project/targets.** Build must pass before moving any code.
- [ ] **Step 4: Move pure asset-safe contracts/controllers domain by domain, preserving logical include prefixes such as `Vehicle/PinkCabGearboxController.h` inside the new module Public tree.**
- [ ] **Step 5: Do not move reflected types whose serialized `/Script/PinkCab.*` identity is not proven asset-safe.** Keep thin compatibility shells in `PinkCab` rather than introducing redirects casually.
- [ ] **Step 6: After each domain move, build and run that domain's automation prefix.** Commit each module migration independently if the diff becomes large.
- [ ] **Step 7: Run analyzer and require module SCCs = 0 and folder-domain SCCs = 0.**
- [ ] **Step 8: Commit final module wiring.**

```powershell
.\scripts\build.ps1
.\scripts\code-health.ps1
git add PinkCab.uproject Source
git commit -m "refactor: enforce PinkCab runtime module boundaries"
```

### Task 11: Close known God Objects and policy violations to zero debt

**Mandatory known splits:**

1. `PinkCabGearboxController.h`
   - Create `Vehicle/PinkCabHGateGeometry.h/.cpp` for gate topology/cursor mapping only.
   - Create `Vehicle/PinkCabGearEngagementValidator.h/.cpp` for requested-vs-engaged mechanical validation only.
   - Keep `FPinkCabGearboxController` as orchestration/state owner; implementation moves to `.cpp`.

2. `PinkCabL1TraversalState.h`
   - Create `Vehicle/PinkCabL1TraversalConfig.h` for immutable/tunable values.
   - Create `Vehicle/PinkCabL1TraversalTransitionPolicy.h/.cpp` for transition predicates.
   - Keep state data and small accessors in `PinkCabL1TraversalState.h`.

3. Runtime Pawn
   - Final `.cpp` must be <=350 non-comment LOC and contain composition/lifecycle/thin delegation only.
   - No raw input policy, drivetrain calculations, persistence codec logic or UI construction remains inline.

4. Any additional baseline violation emitted by Task 2 remains fail-closed: it may be allowlisted only when it is generated/declarative/cohesive immutable data with a concrete rationale and expiry condition; gameplay orchestration may not be allowlisted.
- [ ] **Step 1: Write focused characterization tests for H-gate topology/engagement and L1 traversal before splitting either header.**
- [ ] **Step 2: Verify existing behavior GREEN and add one intentionally failing seam-level test for each new extracted unit.**
- [ ] **Step 3: Extract H-gate geometry; run gearbox tests GREEN; commit.**
- [ ] **Step 4: Extract engagement validator; run gearbox/drivetrain tests GREEN; commit.**
- [ ] **Step 5: Extract L1 traversal config/policy; run L1/vertical/world acceptance GREEN; commit.**
- [ ] **Step 6: Run code-health in final-zero mode:** no grandfathered gameplay debt, no SCC >1, no forbidden ownership edges, no unallowlisted LOC/function/complexity violation.
- [ ] **Step 7: If any unexpected gameplay orchestration violation remains, do not suppress it. Split the exact reported responsibility behind an existing behavior test, rerun the owning suite, and remove that baseline entry before proceeding.**
- [ ] **Step 8: Commit the zero-debt baseline.**

```powershell
python scripts/code_health.py --root . --policy Config/CodeHealthPolicy.json --baseline Config/CodeHealthBaseline.json --report Saved/Reports/code-health-final.json --check --require-zero-debt
git add Config/CodeHealthBaseline.json Source scripts
git commit -m "refactor: close code-health debt"
```

### Task 12: Exact-head verification, package, review and administration

**Files:**
- Update: `docs/ENGINEERING_START_HERE.md`
- Create: `docs/verification/PINK_CAB_CODE_HEALTH_2026-09-19.md`
- Update exact ownership paths in `Config/ArchitectureOwnership.json`.
- No speculative gameplay changes after final verification starts.

**Interfaces:**
- Produces exact-head evidence for Git/Jira/Confluence.
- Human CD-848 feel gate remains open unless the owner explicitly accepts it; architecture completion must not silently close gameplay feel acceptance.

- [ ] **Step 1: Fresh preflight on final candidate.** Record repository root, branch, HEAD, upstream, status, last commits and open PR relationship.
- [ ] **Step 2: Run whitespace and architecture gates.**

```powershell
git diff --check
python -m unittest scripts.tests.test_code_health -v
.\scripts\code-health.ps1 -RequireZeroDebt
```

Expected: zero failures; domain/module SCCs >1 = 0; forbidden edges = 0; unallowlisted size/complexity debt = 0.
- [ ] **Step 3: Run UBT project-only Visual C++ static analysis.**

```powershell
$dotnet='C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\ThirdParty\DotNet\10.0\win-x64\dotnet.exe'
$ubt='C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll'
& $dotnet $ubt PinkCabEditor Win64 Development -Project="$PWD\PinkCab.uproject" -NoUBTMakefiles -WaitMutex -NoHotReloadFromIDE -StaticAnalyzer=VisualCpp -StaticAnalyzerProjectOnly
```

Expected: UBT succeeds with zero actionable project diagnostics.

- [ ] **Step 4: Run canonical clean build.**

```powershell
.\scripts\build.ps1
```

Expected: exit 0.

- [ ] **Step 5: Run the full automation suite, not only prefixes.**

```powershell
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\PinkCab.uproject" -unattended -nop4 -NullRHI -ExecCmds="Automation RunTests PinkCab;Quit" -log
```

Expected: every discovered `PinkCab.*` test PASS; report the exact count rather than assuming the historical 337 count.

- [ ] **Step 6: Re-run the critical real-input runtime acceptance with rendering-capable runtime when available.** Require canonical steering/H-gate direction, E+wheel launch dosing and measurable motion through the PlayerController/Tick path.
- [ ] **Step 7: Scan final logs before packaging.** `Fatal`, `Assertion failed`, `ensure condition failed`, `Unhandled Exception` and automation failures must all be zero.
- [ ] **Step 8: Package through the canonical repository wrapper.**

```powershell
.\scripts\build.ps1 -Package
```

Expected: BuildCookRun/packaging exit 0.

- [ ] **Step 9: Smoke the packaged executable against the intended default map.** Confirm process survives initialization/map load and packaged log has zero fatal/assert/ensure/unhandled.
- [ ] **Step 10: Update `docs/ENGINEERING_START_HERE.md` only with paths verified on the exact accepted HEAD; run code-health doc-sync again.**
- [ ] **Step 11: Write the exact-head verification report.** `docs/verification/PINK_CAB_CODE_HEALTH_2026-09-19.md` records HEAD, before/after Pawn LOC, module graph, SCC count, analyzer result, static-analysis result, automation count, package path/hashes and smoke result.
- [ ] **Step 12: Request independent code review against the approved spec and this plan.** Critical/Important findings are fixed with tests and the full affected verification repeated before integration.
- [ ] **Step 13: Commit only final evidence/documentation after verification remains GREEN.**

```powershell
git add docs/ENGINEERING_START_HERE.md docs/verification Config/ArchitectureOwnership.json
git commit -m "docs: record verified PinkCab architecture normalization"
```

- [ ] **Step 14: Synchronize Jira and Confluence.** Create/use one current architecture-normalization execution issue linked to BASE-100/CD-746 and CD-848; do not reopen/overwrite closed CD-823 historical evidence. Record exact branch/SHA/tests/analyzers/package evidence.
- [ ] **Step 15: Update the durable architecture authority with the final module/ownership map and mark implementation VERIFIED only for evidence actually run.**
- [ ] **Step 16: Leave CD-848 human feel gate open unless the owner separately accepts steering/launch/H-gate/handbrake feel.**
- [ ] **Step 17: Keep release gates closed until evidence is complete.** Do not merge/push/close release gates until the independent review and any required human gates are satisfied.

## Execution order and commit discipline

Tasks are sequential. Do not begin Task N+1 with an unexplained RED from Task N.

Expected commit families:
```text
fix(CD-848): stabilize physical input convention
build: add architecture code-health ratchet
docs: add five-minute engineering ownership map
refactor: isolate player input capture
refactor: extract vehicle control runtime
refactor: enforce single Chaos dynamics adapter
refactor: move taxi pawn to runtime composition root
refactor: normalize domain snapshot ownership
refactor: make runtime domain graph acyclic
refactor: enforce PinkCab runtime module boundaries
refactor: close code-health debt
docs: record verified PinkCab architecture normalization
```

Do not squash away the extraction history before review; the small commits are evidence that behavior was kept green step-by-step.
## Self-review

- Spec coverage: Phase 0 current CD-848 stabilization = Task 1; observability/ratchet = Tasks 2-3; input ownership = Task 4; Pawn decomposition = Tasks 5-7; persistence boundary + SCC removal = Tasks 8-9; physical modules = Task 10; hotspot/zero-debt closure = Task 11; exact-head verification/onboarding/admin = Task 12.
- Placeholder scan: clean; no deferred-marker or generic-test-step language is allowed in executable tasks.
- Type consistency: `FPinkCabPlayerInputSample` feeds `FPinkCabVehicleControlInput`; `FPinkCabVehicleControlRuntime` produces `FPinkCabVehicleControlState`; the existing `IPinkCabVehicleDynamicsProvider::ApplyControls` is the sole physics-write boundary.
- Review Focus coverage: PlayerController timing and launch sequence are Task 1 tests; reflected class identity is Task 7; allowlist strictness is Task 2; indirect cycle detection is Task 2/9.
- Behavior preservation: every extraction is preceded by existing or newly explicit characterization coverage and followed by the owning prefix plus periodic full-suite verification.
- Architecture safety: physical module migration is deliberately after the domain graph is already acyclic; no UCLASS module/package move is required to achieve the first clean dependency graph.
- Failure policy: any unexplained RED stops progression; any unexpected gameplay code-health violation is fixed through a focused characterization/extraction cycle rather than suppressed.
