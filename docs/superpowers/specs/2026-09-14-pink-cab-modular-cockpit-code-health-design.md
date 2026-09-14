# PINK CAB Modular Cockpit, Code Health & Playable Build Gate

**Jira:** CD-823, execution CD-824..CD-829  
**Confluence:** `41 · PINK CAB · Modular Cockpit, Code Health & Playable Build Gate`  
**Authority:** Confluence pages `03 · Tatra & Cockpit Interaction Contract` and `30 · Cabin/Input State Machine & Ownership`

## Goal

Ship a primitive-only but fully functional first-person cockpit whose visual parts can be replaced by final assets without rewriting gameplay, while also establishing project-wide static-analysis evidence and decomposing proven production monoliths without behavior changes.

The accepted result is a Development Win64 package that starts in the driver seat and visibly drives the taxi through the canonical input path.

## Non-goals

This gate does not produce final interior art, hands/IK, finger animation, final materials, final audio mix, production mirror quality, or polished UI. Those assets must plug into the stable contracts created here.

## Locked input grammar

- Mouse steers whenever mouse ownership is not diverted.
- Hold `Space` for GAZE; mouse then controls view instead of steering.
- `1–4 = START / quick reach`: `1 signals`, `2 horn`, `3 gearbox`, `4 handbrake`.
- `LMB = ATTENTION`: commit/retain right hand on a valid physical target.
- `RMB = GO`: perform contextual manipulation/action and own mouse where required.
- `Q = clutch`, `W = brake`, `E = throttle`; W+E may coexist.

Earlier temporary RMB-grab/LMB-manipulate behavior is not authoritative and must not be reintroduced.

## Core architecture

Gameplay truth remains in vehicle/taxi/world state. Cockpit presentation is a consumer and command adapter, never a second source of truth.

`authoritative state -> cockpit presentation adapters -> semantic slots/targets -> primitive or final visual asset`

`APinkCabChaosTatraPawn` becomes a composition root. It must not grow into the cockpit implementation monolith.
## Cockpit units

1. `UPinkCabCockpitAssemblyComponent` owns semantic attachment slots, primitive fallback visuals, camera/head roots and replaceable asset bindings.
2. `UPinkCabCockpitInteractionComponent` owns START/ATTENTION/GO target selection and command emission. It does not own vehicle/taxi state.
3. `FPinkCabCockpitPresentationState` is a read-only projection of vehicle telemetry, cockpit state and taxi state used to drive visuals.
4. `UPinkCabCockpitVisualDriverComponent` maps presentation values to local transforms/visibility/material parameters for replaceable slots.
5. Existing vehicle input/dynamics, fare/taximeter, persistence and world/navigation services remain authoritative.

## Stable semantic slots

At minimum: `DriverCamera`, `SteeringWheel`, `ClutchPedal`, `BrakePedal`, `ThrottlePedal`, `Gearbox`, `Handbrake`, `Ignition`, `TurnSignals`, `Horn`, `Lights`, `Wipers`, `Washer`, `Taximeter`, `PassengerDoor`, `Dashboard`, `Warnings`, `Navigation`, `Radio`, `RearViewMirror`, `LeftMirror`, `RightMirror`.

A slot definition carries a stable ID, local transform, optional mesh/material override, motion axis/pivot/range and interaction target metadata. Primitive fallbacks are implementation detail. Final asset replacement must be data/config assignment only.

## Primitive shell acceptance

The primitive shell provides floor, dash, windshield frame/A-pillars, roof header, side frames, seat reference and readable control silhouettes. It must not obstruct the forward road view. The active game camera is attached to the driver/head root, not the current chase boom.

## Functional cabin scope

The first accepted cockpit exposes steering, clutch/brake/throttle, gearbox, handbrake, ignition, turn signals, horn, lights, wipers/washer, speed/RPM/gear/engine/handbrake indicators, selected warning indicators, taximeter, passenger-door lever, mirror contract, navigation surface, radio surface and recovery/reset.

Taxi meter presentation reads the authoritative fare/taximeter runtime. Passenger-door presentation reads authoritative cockpit/taxi state. Navigation and radio surfaces are adapters to their service contracts; they must not create duplicate route/media truth.

## Movement acceptance

A runtime test must prove the spawned `APinkCabChaosTatraPawn` is possessed by a local controller, the driver camera is active, ignition and handbrake commands reach live vehicle dynamics, and non-zero throttle produces measurable forward displacement. Packaged smoke must load the intended playable map and remain alive without fatal/assert/crash.

## Code-health gate

Run UnrealBuildTool Visual C++ static analysis across project modules before and after refactoring:

`Build.bat PinkCabEditor Win64 Development <PinkCab.uproject> -WaitMutex -NoHotReloadFromIDE -StaticAnalyzer=VisualCpp -StaticAnalyzerProjectOnly`

Every finding is classified as project defect, engine/toolchain noise, or accepted-with-reason. Project defects are fixed before gate closure.
## Monolith policy

Monolith decomposition is responsibility-driven, not cosmetic line-count reduction. Review all production hotspots, especially current pre-gate files around 230+ LOC or files mixing serialization, validation, orchestration and state mutation.

Known hotspots at `ac62ecb` include `PinkCabPersistenceService.h` (~376 LOC), `PinkCabGameSnapshotArchive.h` (~362), `PinkCabMetroTransitRuntime.h` (~325), `PinkCabPassengerRecord.h` (~298), `PinkCabL1TraversalState.h` (~294), `PinkCabFareRuntimeSnapshot.h` (~281), `PinkCabServiceSnapshot.h` (~250), `PinkCabSuspendedBusRuntime.h` (~234) and `PinkCabFareLoopCoordinator.h` (~233).

Each hotspot gets one of three outcomes recorded in the code-health report: split now with preserved public behavior; retain with explicit single-responsibility rationale; or create a bounded follow-up when splitting would exceed the current gate and is not required for cockpit/playable correctness. New cockpit code may not add another mixed-responsibility hotspot.

## Verification matrix

Required exact-head evidence:

- clean `PinkCabEditor` Development build;
- `PinkCab.Vehicle` regression;
- `PinkCab.Interaction` regression;
- `PinkCab.Taxi` regression;
- `PinkCab.Persistence` regression where state contracts changed;
- new `PinkCab.Cockpit` contract tests;
- new cockpit runtime/PIE movement test;
- final VisualCpp project-only static analysis;
- `BuildCookRun` Win64 Development package;
- packaged executable smoke with intended map load and no fatal/assert/crash.

## Definition of Done

The gate is complete only when the package starts in a readable driver cockpit, the possessed taxi visibly moves through the canonical input path, cockpit controls and instruments consume one authoritative state path, primitive assets are replaceable by slot/config assignment, actionable analyzer findings are resolved, monolith decisions are documented and tested, and Git/Jira/Confluence evidence names the exact accepted head.
