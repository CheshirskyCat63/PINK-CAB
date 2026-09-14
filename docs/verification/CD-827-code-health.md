# CD-827 Code Health Baseline

**Baseline head:** `ac62ecb`  
**Branch:** `feat/CD-823-cockpit-playable-code-health`  
**Analyzer:** UnrealBuildTool Visual C++ static analyzer, project modules only.

## Analyzer command

`Build.bat PinkCabEditor Win64 Development <PinkCab.uproject> -WaitMutex -NoHotReloadFromIDE -StaticAnalyzer=VisualCpp -StaticAnalyzerProjectOnly`

Baseline result on exact pre-gate `ac62ecb`: **Succeeded**, **15 analyzer translation units**, **0 compiler/analyzer diagnostics** matching `warning/error C####` project patterns. Raw local evidence: `Artifacts/Analysis/baseline-visualcpp.txt` (not committed build artifact).

## Production hotspot inventory

| File | Approx LOC | Initial disposition |
| --- | ---: | --- |
| `PinkCabPersistenceService.h` | 376 | **split now** — binary codec/checkpoint ring/commit-recovery orchestration share one header |
| `PinkCabGameSnapshotArchive.h` | 362 | **split now** — one archive owns passenger/economy/fare/vehicle/service/world domain serializers |
| `PinkCabMetroTransitRuntime.h` | 325 | **split now** — topology/catalog mutation and live Tatra transit state machine are separate responsibilities |
| `PinkCabPassengerRecord.h` | 298 | **split now** — passenger record model and bounded passenger registry share one header |
| `PinkCabL1TraversalState.h` | 294 | **retain — cohesive** — one bounded L1 traversal state machine with phase-specific private handlers |
| `PinkCabFareRuntimeSnapshot.h` | 281 | **split now** — snapshot DTOs are mixed with capture/restore/validation codec policy |
| `PinkCabServiceSnapshot.h` | 250 | **split now** — service snapshot DTOs are mixed with capture/restore/validation codec policy |
| `PinkCabSuspendedBusRuntime.h` | 234 | **retain — cohesive** — one bounded route runtime with its own obstacle/contact phase state |
| `PinkCabFareLoopCoordinator.h` | 233 | **retain — cohesive** — lifecycle orchestration is the class responsibility; settlement/load services remain external |

## Policy

Line count alone is not a defect. Each hotspot is inspected for responsibility mixing. Outcome is one of: `split now`, `retain вЂ” cohesive`, or `follow-up вЂ” split would be unrelated/risky for current gate`. Public behavior and persistence schemas are frozen by existing regression tests before extraction.

## New cockpit constraint

`APinkCabChaosTatraPawn` is a composition root only. New cockpit geometry, interaction policy and visual presentation live in focused units under `Source/PinkCab/{Public,Private}/Cockpit`.
