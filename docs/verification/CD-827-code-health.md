# CD-827 Code Health Baseline

**Baseline head:** `ac62ecb`  
**Branch:** `feat/CD-823-cockpit-playable-code-health`  
**Analyzer:** UnrealBuildTool Visual C++ static analyzer, project modules only.

## Analyzer command

`Build.bat PinkCabEditor Win64 Development <PinkCab.uproject> -WaitMutex -NoHotReloadFromIDE -StaticAnalyzer=VisualCpp -StaticAnalyzerProjectOnly`

Baseline result: **Succeeded**, 17 analysis actions, **0 compiler/analyzer diagnostics** matching project warning/error patterns. Raw local evidence: `Artifacts/Analysis/baseline-visualcpp.txt` (not committed build artifact).

## Production hotspot inventory

| File | Approx LOC | Initial disposition |
| --- | ---: | --- |
| `PinkCabPersistenceService.h` | 376 | review for mixed persistence orchestration/validation/replay responsibility |
| `PinkCabGameSnapshotArchive.h` | 362 | review for archive/schema/validation responsibility split |
| `PinkCabMetroTransitRuntime.h` | 325 | review runtime state vs schedule/boarding policy |
| `PinkCabPassengerRecord.h` | 298 | review identity/relationship/history responsibility |
| `PinkCabL1TraversalState.h` | 294 | review state vs transition policy |
| `PinkCabFareRuntimeSnapshot.h` | 281 | review schema DTO vs capture/restore helpers |
| `PinkCabServiceSnapshot.h` | 250 | review schema DTO vs restore policy |
| `PinkCabSuspendedBusRuntime.h` | 234 | review runtime state vs route/boarding policy |
| `PinkCabFareLoopCoordinator.h` | 233 | review coordinator vs settlement/replay policy |

## Policy

Line count alone is not a defect. Each hotspot is inspected for responsibility mixing. Outcome is one of: `split now`, `retain — cohesive`, or `follow-up — split would be unrelated/risky for current gate`. Public behavior and persistence schemas are frozen by existing regression tests before extraction.

## New cockpit constraint

`APinkCabChaosTatraPawn` is a composition root only. New cockpit geometry, interaction policy and visual presentation live in focused units under `Source/PinkCab/{Public,Private}/Cockpit`.
