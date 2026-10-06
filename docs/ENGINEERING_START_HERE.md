# PINK CAB вЂ” Engineering Start Here

This page is the five-minute entry point for changing PINK CAB code. It describes the current exact repository shape; `Config/ArchitectureOwnership.json` is the machine-readable ownership source and `scripts/code-health.ps1` rejects stale paths or missing rows.

## Accepted vehicle checkpoint В· CD-952

Accepted P04 measured gearing runtime: **52239b61bc80e5a63716b9b09b87c520fa09fd05**, delivery **37149462470**, attempt 1; explicitly owner accepted on 2026-10-03. PR #62 integrated it as **4a313d38f0674a3e5048f832a07428defa31ab62**. Read GitHub for later main commits; integration/admin commits never rename accepted executable bytes.

Previous accepted fallbacks: P03/V2 `edf75e1b` / delivery `37117735294`, and P02 `8d68e456` / delivery `36868646970`. Road R1-R5 and no-assist input grammar remain frozen. Sole root desktop game entry: `PINCKCAB`; studio entries are Editor and explicitly named Rollback_V2.

Coordinator `37148042881`, gated delivery `37149462470` and installed audit `37151172013` passed: 79 script tests, complete ControlRuntime, 63 physics tests, 240 D3 cases, five slope repeats and 53 installed payload files. Additive owner decision is `OWNER_ACCEPTANCE.json`, retained by run `37152180059`; original HUMAN_PENDING handoff receipts remain unchanged historical evidence. Administrative closeout: CD-952.

CD-648 remains the single vehicle umbrella. CD-641 owns remaining P04 performance calibration and the non-monotonic intermediate-input observation; P05-P11 and full FIRST EURO remain unfinished. CD-559 retains clean-source/full-project/packaged-input engineering, not an absent P03 acceptance. PR #47/CD-650 remains diagnostic only. CD-855 placeholder polish/replacement is deferred until vehicle calibration and does not block internal physics/admin; public asset rights stay separate.

## 1. First five minutes

1. Run `git rev-parse --show-toplevel`, `git branch --show-current`, `git status` and `git log -5 --oneline` before editing.
2. Project file is `PinkCab.uproject`; production engine line is Unreal Engine 5.8, currently installed and locally built on 5.8.3.
3. Read `docs/AUTHORITY.yaml` for source-of-truth pointers and `docs/PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md` before changing controls/vehicle mechanics.
4. Find the concern in the table below. Change the listed owner, not a convenient caller.
5. Run the concern's test prefix, then `scripts/code-health.ps1`, then the affected domain suite.

## 2. Canonical commands

Build on this Windows node:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1
```

Architecture/code-health gate:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\code-health.ps1
```

Full automation uses the project runner pattern with `-Multiprocess`. Run it only in an isolated disposable checkout: authoring tests can rewrite tracked maps and road assets. The accepted vehicle suite is not the full PinkCab product suite; full-product regression remains separately scoped under CD-559.

```powershell
$UE = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
& $UE "$PWD\PinkCab.uproject" -Multiprocess -unattended -NullRHI -nosplash -nopause -NoSound -stdout `
  '-ExecCmds=Automation RunTests PinkCab' '-TestExit=Automation Test Queue Empty'
```

Do not infer gameplay correctness from compilation alone. Parse the automation log for the completed queue and failure/fatal/assert/ensure signals.

## 3. Current architecture

Production code now has eight asset-safe runtime modules plus the reflected compatibility/composition module. Pure contracts live in their domain module; reflected types whose serialized identity is still `/Script/PinkCab.*` remain in `PinkCab` until an asset-safe migration is independently proven.

```text
PinkCabCore        -> stable IDs / result / schema / state-kernel contracts
PinkCabInteraction -> semantic input / device normalization
PinkCabVehicle     -> controls / drivetrain / Chaos adapter / vehicle state
PinkCabEconomy     -> money / exactly-once transaction ownership
PinkCabWorld       -> CityCode / graph / route / materialization contracts
PinkCabTraffic     -> bounded traffic over World contracts
PinkCabTaxi        -> fare / passenger product logic
PinkCabPersistence -> asset-safe save DTOs / codecs / migrations / checkpoints
PinkCab            -> reflected compatibility, Runtime/Cockpit/Service composition
```

The module dependency graph and the folder-domain dependency graph are both required to remain acyclic. `Config/CodeHealthPolicy.json` is the executable module contract.

Hard rules:

- raw keyboard/mouse/device interpretation belongs in Interaction;
- player-space `+X` means right; gearbox `+Y` means top/forward row 1/3/5;
- game-thread semantic control writes belong in `PinkCabChaosVehicleDynamicsProvider`; physics-thread engine/wheel torque writes belong only in `PinkCabChaosVehicleSimulation`; `PinkCabChaosVehicleMovementComponent` is the command/evidence bridge; no third Chaos actuation writer is allowed;
- gameplay domains do not depend on Persistence implementation internals;
- presentation consumes authoritative state and does not create a second source of truth;
- no new dependency cycle or larger code-health violation may be added over the committed baseline.

## 4. I want to change X вЂ” go here

| Concern ID | Change | Current owner | Public contract | Test prefix |
| --- | --- | --- | --- | --- |
| `passenger_snapshot_state` | passenger snapshot data; persistence codec is external | Taxi | `Source/PinkCabTaxi/Public/Taxi/PinkCabPassengerSnapshot.h` | `PinkCab.Taxi.PassengerSnapshot` |
| `vehicle_state_snapshot` | current vehicle health/load runtime snapshot without schema | Vehicle | `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleStateSnapshot.h` | `PinkCab.Vehicle.LiveState` |
| `fare_runtime_state_snapshot` | fare/session/meter/manifest snapshot data without persistence schema | Taxi | `Source/PinkCabTaxi/Public/Taxi/PinkCabFareRuntimeStateSnapshot.h` | `PinkCab.Persistence.FareRuntimeSnapshot` |
| `service_snapshot_state` | service/inventory/build/fuel snapshot data; persistence codec is external | Service | `Source/PinkCab/Public/Service/PinkCabServiceSnapshotTypes.h` | `PinkCab.Service.Snapshot` |
| `runtime_composition` | possession/runtime wiring and compatibility shell | Runtime | `Source/PinkCab/Public/Runtime/PinkCabChaosTatraPawn.h` | `PinkCab.Runtime.Composition` |
| `vehicle_visual_presentation` | vehicle exterior/cabin/presentation profile and visual shell | Runtime | `Source/PinkCab/Public/Runtime/PinkCabVehicleVisualProfile.h` | `PinkCab.Vehicle.Visual` |
| `driver_ui` | system menu, HUD, cursor/capture and input-mode presentation | Runtime | `Source/PinkCab/Public/Runtime/PinkCabDriverUiComponent.h` | `PinkCab.UI.SystemMenu` |
| `vehicle_control_orchestration` | per-frame vehicle control orchestration | Vehicle | `Source/PinkCab/Public/Vehicle/PinkCabVehicleControlRuntime.h` | `PinkCab.Vehicle.ControlRuntime.Runtime` |
| `steering` | steering response/feel logic | Vehicle | `Source/PinkCabVehicle/Public/Vehicle/PinkCabSteeringController.h` | `PinkCab.Vehicle.ControlRuntime.Steering` |
| `gearbox` | H-gate/requested/engaged behavior | Vehicle | `Source/PinkCabVehicle/Public/Vehicle/PinkCabGearboxController.h` | `PinkCab.Vehicle.ControlRuntime.Gearbox` |
| `clutch` | clutch torque transfer, slip, lock and bidirectional engineв†”shaft reaction | Vehicle | `Source/PinkCabVehicle/Public/Vehicle/PinkCabClutchDrivelineModel.h` | `PinkCab.Vehicle.Physics.P02.ClutchModel` |
| `throttle` | launch redose/target behavior | Vehicle | `Source/PinkCabVehicle/Public/Vehicle/PinkCabLaunchController.h` | `PinkCab.Vehicle.ControlRuntime` |
| `brake` | brake dosing target | Vehicle | `Source/PinkCabVehicle/Public/Vehicle/PinkCabPedalDosingController.h` | `PinkCab.Vehicle.ControlRuntime` |
| `handbrake` | analog parking/hydraulic behavior | Vehicle | `Source/PinkCabVehicle/Public/Vehicle/PinkCabHandbrakeActuator.h` | `PinkCab.Vehicle.ControlRuntime.Handbrake` |
| `device_input_sign` | OS/UE mouse axis -> driver-space sign | Interaction | `Source/PinkCabInteraction/Public/Interaction/PinkCabPhysicalInputConvention.h` | `PinkCab.Vehicle.ControlRuntime` |
| `player_input_capture` | PlayerController/raw device capture -> semantic sample | Interaction | `Source/PinkCabInteraction/Public/Interaction/PinkCabPlayerInputAdapter.h` | `PinkCab.Interaction.PlayerInput` |
| `chaos_translation` | semantic controls -> authoritative Chaos command | Vehicle | `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleDynamicsProvider.h` | `PinkCab.Vehicle.ChaosBaseline` |
| `chaos_physics_actuation` | physics-thread engine/clutch/wheel torque integration | Vehicle | `Source/PinkCabVehicle/Public/Vehicle/PinkCabChaosVehicleMovementComponent.h` | `PinkCab.Vehicle.Physics.P02` |
| `fare` | fare loop/taximeter lifecycle | Taxi | `Source/PinkCabTaxi/Public/Taxi/PinkCabFareLoopCoordinator.h` | `PinkCab.Taxi` |
| `passenger` | persistent passenger record behavior | Taxi | `Source/PinkCabTaxi/Public/Taxi/PinkCabPassengerRecord.h` | `PinkCab.Taxi.Passenger` |
| `economy` | balance/transactions/exactly-once ledger | Economy | `Source/PinkCabEconomy/Public/Economy/PinkCabEconomyLedger.h` | `PinkCab.Economy` |
| `vehicle_snapshot_archive` | versioned vehicle save schema + legacy migration adapter | Persistence | `Source/PinkCabPersistence/Public/Persistence/PinkCabVehicleSnapshot.h` | `PinkCab.Persistence.VehicleSnapshot` |
| `game_persistence` | save/checkpoint/restore orchestration | Persistence | `Source/PinkCab/Public/Persistence/PinkCabGamePersistenceCoordinator.h` | `PinkCab.Persistence` |
| `world_route` | route search/road graph routing | World | `Source/PinkCabWorld/Public/World/PinkCabRouteService.h` | `PinkCab.World.Routing` |
| `traffic` | logical traffic flow/gaps | Traffic | `Source/PinkCabTraffic/Public/Traffic/PinkCabTrafficFlow.h` | `PinkCab.Traffic` |
| `cockpit_presentation` | instrument/lever/visual projection | Cockpit | `Source/PinkCab/Public/Cockpit/PinkCabCockpitPresentationState.h` | `PinkCab.Cockpit.VisualDriver` |

The implementation path for each row is in `Config/ArchitectureOwnership.json`; do not duplicate that information into a second hand-maintained table.

## 5. Control/mechanics locks that must survive refactors

- Mouse steers by default; user-facing steering direction is not inverted.
- `Q` clutch, `W` brake, `E` throttle; wheel recipient priority is E -> W -> Q.
- Every genuine new launch requires `E + wheel` throttle redosing. Do not restore automatic launch throttle.
- H-gate is 1/3/5 top, 2/4/R bottom, reverse far-right bottom, with a real neutral corridor.
- No ABS, ESP, auto-throttle, auto-rev-match, auto-countersteer or yaw-rescue assist.
- Engine/chassis sign or unit adaptation may happen once inside a technology adapter; feature/controller layers stay in semantic driver space.

## 6. Before you commit

1. Run the focused tests for the concern.
2. Run the affected domain prefix.
3. Run `git diff --check`.
4. Run `scripts/code-health.ps1`; new debt must fail closed.
5. For a behavior change, keep RED -> GREEN evidence. For a pure extraction, identify the characterization test that proves preserved behavior.
6. Do not mark Jira/Confluence VERIFIED from docs or compile output; exact executable evidence is required.

Architecture normalization design: `docs/superpowers/specs/2026-09-19-pink-cab-code-health-architecture-normalization-design.md`.
Implementation plan: `docs/superpowers/plans/2026-09-19-pink-cab-code-health-architecture-normalization.md`.

<!-- P4_ACTIVE_OWNERSHIP_START -->
## Active P4 ownership

The active baseline keeps ownership intentionally broad: `runtime`, `vehicle`, `cockpit`, `interaction`, `world`, `taxi`, `economy`, `traffic`, `persistence`.
Detailed clutch, pedal, telemetry, evidence, and road-authoring research is historical and must not become a required production dependency.
<!-- P4_ACTIVE_OWNERSHIP_END -->
