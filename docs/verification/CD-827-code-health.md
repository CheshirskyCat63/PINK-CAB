# CD-827 Code Health Baseline and Decomposition Evidence

**Baseline head:** `ac62ecb`  
**Branch:** `feat/CD-823-cockpit-playable-code-health`  
**Analyzer:** UnrealBuildTool Visual C++ static analyzer, project modules only.

## Analyzer baseline

`Build.bat PinkCabEditor Win64 Development <PinkCab.uproject> -WaitMutex -NoHotReloadFromIDE -StaticAnalyzer=VisualCpp -StaticAnalyzerProjectOnly`

Baseline result on exact pre-gate `ac62ecb`: **Succeeded**, **15 analyzer translation units**, **0 compiler/analyzer diagnostics** matching project `warning/error C####` patterns. Raw local evidence: `Artifacts/Analysis/baseline-visualcpp.txt`.

## Baseline hotspot classification

| File | Baseline LOC | Disposition |
| --- | ---: | --- |
| `PinkCabPersistenceService.h` | 376 | **split now** — binary codec/checkpoint ring/commit-recovery orchestration mixed |
| `PinkCabGameSnapshotArchive.h` | 362 | **split now** — passenger/economy/fare/vehicle/service/world serializers mixed |
| `PinkCabMetroTransitRuntime.h` | 325 | **split now** — network definition mutation and live transit state mixed |
| `PinkCabPassengerRecord.h` | 298 | **split now** — passenger record model and bounded registry mixed |
| `PinkCabL1TraversalState.h` | 294 | **retain — cohesive** — one bounded L1 traversal state machine |
| `PinkCabFareRuntimeSnapshot.h` | 281 | **split now** — DTO/schema and codec policy mixed |
| `PinkCabServiceSnapshot.h` | 250 | **split now** — DTO/schema and codec policy mixed |
| `PinkCabSuspendedBusRuntime.h` | 234 | **retain — cohesive** — one bounded route/contact state machine |
| `PinkCabFareLoopCoordinator.h` | 233 | **retain — cohesive** — lifecycle orchestration is the class responsibility |

## Task 7 decomposition result

| Original ownership | After split | Evidence commit |
| --- | --- | --- |
| `PinkCabPassengerRecord.h` 298 LOC | façade 4; record types 181; registry 152 | `a0f6ddd` |
| `PinkCabFareRuntimeSnapshot.h` 281 LOC | façade 4; DTO/types 55; codec 250 | `826b308` |
| `PinkCabServiceSnapshot.h` 250 LOC | façade 4; DTO/types 70; codec 204 | `d308886` |
| `PinkCabGameSnapshotArchive.h` 362 LOC | orchestrator 27; focused passenger/economy/fare/vehicle/service/world serializers 94/42/72/37/90/57 + shared primitives | `9514ab7` |
| `PinkCabPersistenceService.h` 376 LOC | orchestration façade 128; binary codec 241; checkpoint ring 40; shared persistence types | `4dca3de` |
| `PinkCabMetroTransitRuntime.h` 325 LOC | live runtime 182; network definition/catalog 204 | `3e54195` |

The retained files remain intentionally cohesive; no split was performed merely to reduce LOC. Public include paths are preserved as façades where existing callers depend on them. Persistence schemas and behavior remain unchanged.

## Verification policy

Every extraction is followed by the owning regression suite before commit. After the final extraction, the complete touched-subsystem regression set is rerun. The final Visual C++ analyzer belongs to Task 9 and will be recorded separately as exact-head evidence.

## Cockpit constraint

`APinkCabChaosTatraPawn` remains a composition root. Cockpit geometry, interaction, presentation, prototype assets and service presentation are owned by focused units under `Source/PinkCab/{Public,Private}/Cockpit`; authoritative taxi/vehicle/world state remains outside the visual layer.

## Task 7 final regression

After the final split, the touched-subsystem exact-branch regression completed with no failed automation results and process exit `0` for: `PinkCab.Persistence`, `PinkCab.Taxi`, `PinkCab.Service`, `PinkCab.World`, `PinkCab.Vertical`, `PinkCab.Core.CodeComplete`, and `PinkCab.Core`.

Task 7 therefore closes with every baseline `split now` hotspot separated by responsibility while all baseline `retain — cohesive` units remain intact.

## Task 9 final analyzer

Final exact-branch analyzer command:
`Build.bat PinkCabEditor Win64 Development <PinkCab.uproject> -WaitMutex -NoHotReloadFromIDE -StaticAnalyzer=VisualCpp -StaticAnalyzerProjectOnly`

Result: **Succeeded**, **19 analyzer actions**, **0 project `warning C####` / `error C####` diagnostics**. Raw local evidence: `Artifacts/Analysis/final-visualcpp.txt`.

The final analyzer finding set is therefore clean; no project defect or accepted-with-reason diagnostic remains open for this gate.
