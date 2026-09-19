# PINK CAB — Engineering Start Here

This page is the five-minute entry point for changing PINK CAB code. It describes the current exact repository shape; `Config/ArchitectureOwnership.json` is the machine-readable ownership source and `scripts/code-health.ps1` rejects stale paths or missing rows.

## 1. First five minutes

1. Run `git rev-parse --show-toplevel`, `git branch --show-current`, `git status` and `git log -5 --oneline` before editing.
2. Project file is `PinkCab.uproject`; production engine line is Unreal Engine 5.8, currently verified on 5.8.2.
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

Full automation uses the project runner pattern with `-Multiprocess`:

```powershell
$UE = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
& $UE "$PWD\PinkCab.uproject" -Multiprocess -unattended -NullRHI -nosplash -nopause -NoSound -stdout `
  '-ExecCmds=Automation RunTests PinkCab' '-TestExit=Automation Test Queue Empty'
```

Do not infer gameplay correctness from compilation alone. Parse the automation log for the completed queue and failure/fatal/assert/ensure signals.

## 3. Current architecture

The repository currently still builds production code through the `PinkCab` runtime module. Domain folders already define ownership; the approved normalization program will convert asset-safe boundaries into physical Unreal modules only after the dependency graph is acyclic.

```text
Interaction -> semantic input / device normalization
Vehicle     -> controls / drivetrain / Chaos adapter / vehicle state
Taxi        -> fare / passenger product logic
Economy     -> money / transaction ownership
World       -> CityCode / graph / route / materialization
Traffic     -> bounded traffic over World contracts
Persistence -> snapshots / codecs / migrations / checkpoints
Cockpit     -> interaction/presentation consumers, never vehicle/taxi truth
PinkCab     -> current runtime composition / reflected compatibility shell
```

Hard rules:

- raw keyboard/mouse/device interpretation belongs in Interaction;
- player-space `+X` means right; gearbox `+Y` means top/forward row 1/3/5;
- Chaos control writes belong in the Chaos vehicle dynamics adapter;
- gameplay domains do not depend on Persistence implementation internals;
- presentation consumes authoritative state and does not create a second source of truth;
- no new dependency cycle or larger code-health violation may be added over the committed baseline.

## 4. I want to change X — go here

| Concern ID | Change | Current owner | Public contract | Test prefix |
| --- | --- | --- | --- | --- |
| `vehicle_control_orchestration` | per-frame vehicle control orchestration | Vehicle | `Source/PinkCab/Public/Vehicle/PinkCabVehicleControlRuntime.h` | `PinkCab.Vehicle.ControlRuntime.Runtime` |
| `steering` | steering response/feel logic | Vehicle | `Source/PinkCab/Public/Vehicle/PinkCabSteeringController.h` | `PinkCab.Vehicle.ControlRuntime.Steering` |
| `gearbox` | H-gate/requested/engaged behavior | Vehicle | `Source/PinkCab/Public/Vehicle/PinkCabGearboxController.h` | `PinkCab.Vehicle.ControlRuntime.Gearbox` |
| `clutch` | clutch coupling, heat/wear, stall coupling | Vehicle | `Source/PinkCab/Public/Vehicle/PinkCabDrivetrainCondition.h` | `PinkCab.Vehicle.ControlRuntime` |
| `throttle` | launch redose/target behavior | Vehicle | `Source/PinkCab/Public/Vehicle/PinkCabLaunchController.h` | `PinkCab.Vehicle.ControlRuntime` |
| `brake` | brake dosing target | Vehicle | `Source/PinkCab/Public/Vehicle/PinkCabPedalDosingController.h` | `PinkCab.Vehicle.ControlRuntime` |
| `handbrake` | analog parking/hydraulic behavior | Vehicle | `Source/PinkCab/Public/Vehicle/PinkCabHandbrakeActuator.h` | `PinkCab.Vehicle.ControlRuntime.Handbrake` |
| `device_input_sign` | OS/UE mouse axis -> driver-space sign | Interaction | `Source/PinkCab/Public/Interaction/PinkCabPhysicalInputConvention.h` | `PinkCab.Vehicle.ControlRuntime` |
| `player_input_capture` | PlayerController/raw device capture -> semantic sample | Interaction | `Source/PinkCab/Public/Interaction/PinkCabPlayerInputAdapter.h` | `PinkCab.Interaction.PlayerInput` |
| `chaos_translation` | semantic controls -> Chaos API | Vehicle | `Source/PinkCab/Public/Vehicle/PinkCabVehicleDynamicsProvider.h` | `PinkCab.Vehicle.ChaosBaseline` |
| `fare` | fare loop/taximeter lifecycle | Taxi | `Source/PinkCab/Public/Taxi/PinkCabFareLoopCoordinator.h` | `PinkCab.Taxi` |
| `passenger` | persistent passenger record behavior | Taxi | `Source/PinkCab/Public/Taxi/PinkCabPassengerRecord.h` | `PinkCab.Taxi.Passenger` |
| `economy` | balance/transactions/exactly-once ledger | Economy | `Source/PinkCab/Public/Economy/PinkCabEconomyLedger.h` | `PinkCab.Economy` |
| `vehicle_snapshot` | current vehicle save schema/capture codec | Persistence | `Source/PinkCab/Public/Persistence/PinkCabVehicleSnapshot.h` | `PinkCab.Persistence.VehicleSnapshot` |
| `game_persistence` | save/checkpoint/restore orchestration | Persistence | `Source/PinkCab/Public/Persistence/PinkCabGamePersistenceCoordinator.h` | `PinkCab.Persistence` |
| `world_route` | route search/road graph routing | World | `Source/PinkCab/Public/World/PinkCabRouteService.h` | `PinkCab.World.Routing` |
| `traffic` | logical traffic flow/gaps | Traffic | `Source/PinkCab/Public/Traffic/PinkCabTrafficFlow.h` | `PinkCab.Traffic` |
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
