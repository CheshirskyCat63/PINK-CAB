# PINK-CAB CD-869 · L1 Endless Straight Road Streaming Implementation Plan

> **Spec:** `docs/superpowers/specs/2026-09-24-l1-endless-road-streaming-design.md`
>
> **Execution base:** `main@ed969edac845248464f8442a78899d26e6dfecde`
>
> **Implementation branch:** `feat/CD-869-l1-endless-road`
>
> **Rule:** TDD, exact-head verification, no competing world/streaming system, no gameplay-control rewrites.

## Goal

Deliver the first real Level 1 road foundation: one pleasant 1000 m MetaRoad-authored straight road module that is materialized indefinitely ahead of the current Tatra, retired behind it with a fixed bounded pool, reconstructs the same logical chunks on return, and can be driven forward/reverse without visible road voids or progressive pool/memory growth.

This plan deliberately does **not** add buildings, signs, trash, traffic, parking gameplay, wallride, freight ceiling, curves, Level 2 or taxi objectives.

## Global constraints

1. `PinkCabWorld` remains the owner of CityCode, ChunkId, RoadGraph and materialization policy.
2. MetaRoad Free is editor/bake tooling only; no runtime authority moves into MetaRoad.
3. Native Chaos vehicle/input code is untouched unless an integration test proves a world-only compatibility defect.
4. Runtime representation count is hard-capped at seven chunks.
5. One chunk is exactly `100000 cm`.
6. Positive-X travel keeps 2 behind / current / 4 ahead; negative-X travel mirrors that window.
7. At standstill, last stable travel direction is retained.
8. Missing baked road assets fail visibly; no silent greybox substitution in the accepted candidate.
9. The accepted vehicle baseline `8168d724…` is not rewritten or reclassified by this work.
10. The implementation candidate is not merged until focused world tests, broader PINK-CAB regression, build/package gate appropriate to changed content, code review and human road-driving gate are satisfied.

## File map

### New pure/runtime-world files

- `Source/PinkCabWorld/Public/World/PinkCabL1EndlessRoadModel.h`
  - pure signed chunk-index math;
  - stable travel-direction hysteresis;
  - desired seven-chunk window;
  - 14-lane straight-chunk topology builder;
  - no Actor/UObject ownership.

- `Source/PinkCabWorld/Private/World/PinkCabL1EndlessRoadModel.cpp`
  - implementation of the pure model.

### New game-runtime files

- `Source/PinkCab/Public/World/PinkCabL1RoadChunkActor.h`
  - one reusable physical 1000 m chunk representation;
  - binds a logical `FPinkCabChunkId`;
  - owns the baked visual/collision component(s);
  - exposes diagnostic bound ID/index.

- `Source/PinkCab/Private/World/PinkCabL1RoadChunkActor.cpp`
  - road asset resolution;
  - logical rebind + world placement;
  - deterministic presentation parameters if supported by material instances;
  - fail-closed missing-asset behavior.

- `Source/PinkCab/Public/World/PinkCabL1EndlessRoadStreamer.h`
  - orchestration Actor;
  - fixed seven-slot pool;
  - tracked Tatra/player Actor;
  - chunk-boundary and direction-change updates;
  - diagnostic counters.

- `Source/PinkCab/Private/World/PinkCabL1EndlessRoadStreamer.cpp`
  - runtime pool/materialization implementation using `FPinkCabWorldMaterializationPolicy`.

### New/updated test files

- `Source/PinkCabTests/Private/World/PinkCabL1EndlessRoadModelTests.cpp`
  - pure RED→GREEN model/graph tests.

- `Source/PinkCabTests/Private/World/PinkCabL1EndlessRoadRuntimeTests.cpp`
  - Actor pool, rebinding, bounded count and missing-asset runtime tests.

- `Source/PinkCabTests/Private/World/PinkCabL1EndlessRoadMapGenerator.cpp`
  - editor-only generation/update of the test map after the MetaRoad asset is admitted.

- `Source/PinkCabTests/Private/World/PinkCabL1EndlessRoadMapTests.cpp`
  - baked asset existence/bounds/collision and saved-map composition tests.

- Existing `PinkCabWorldMaterializationTests.cpp`
  - remains unchanged unless a failing test demonstrates a required extension.

### Content produced during MetaRoad authoring/admission

Canonical target paths, subject to actual MetaRoad bake output discovered on the Windows runner:

- `/Game/World/L1/Road/SM_PC_L1_Straight_1000m`
- `/Game/World/L1/Road/Materials/M_PC_L1_Asphalt`
- `/Game/World/L1/Road/Materials/MI_PC_L1_Asphalt_Wet`
- `/Game/World/L1/Road/Materials/M_PC_L1_Marking`
- `/Game/World/L1/Road/Materials/M_PC_L1_GreenDivider`
- `/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight`

If MetaRoad bake necessarily emits multiple meshes, retain them under the same folder and bind them through one `APinkCabL1RoadChunkActor`; do not widen runtime ownership.

## Interfaces

### `FPinkCabL1EndlessRoadModel`

Planned public API:

```cpp
enum class EPinkCabLongitudinalTravelDirection : int8
{
    Negative = -1,
    Positive = 1
};

struct FPinkCabL1EndlessRoadWindow
{
    int32 CurrentChunkIndex = 0;
    EPinkCabLongitudinalTravelDirection Direction =
        EPinkCabLongitudinalTravelDirection::Positive;
    TArray<FPinkCabChunkCoord> DesiredCoords;
};

class PINKCABWORLD_API FPinkCabL1EndlessRoadModel
{
public:
    static constexpr double ChunkLengthCm = 100000.0;
    static constexpr int32 PoolSize = 7;
    static constexpr int32 BehindCount = 2;
    static constexpr int32 AheadCount = 4;
    static constexpr int32 GroundLaneCount = 14;

    static int32 ResolveChunkIndex(double LongitudinalCm);

    static EPinkCabLongitudinalTravelDirection ResolveTravelDirection(
        double LongitudinalVelocityCmPerSec,
        EPinkCabLongitudinalTravelDirection LastStableDirection,
        double DirectionThresholdCmPerSec);

    static FPinkCabL1EndlessRoadWindow BuildWindow(
        int32 CurrentChunkIndex,
        EPinkCabLongitudinalTravelDirection Direction);

    static bool AppendStraightChunkLanes(
        const FPinkCabCityIdentity& City,
        int32 ChunkIndex,
        FPinkCabRoadGraph& InOutGraph);
};
```

The final names may change only if existing naming conventions require it; behavior and ownership may not drift from the spec.

### `APinkCabL1RoadChunkActor`

Minimum behavior:

```cpp
bool BindChunk(
    const FPinkCabCityIdentity& City,
    int32 ChunkIndex,
    const FPinkCabChunkId& ChunkId);

void ClearBinding();
bool IsBound() const;
int32 GetBoundChunkIndex() const;
FPinkCabChunkId GetBoundChunkId() const;
```

A successful bind places the actor at `X = ChunkIndex * 100000 cm` for this v1 proof.

### `APinkCabL1EndlessRoadStreamer`

Minimum diagnostics:

```cpp
int32 GetCurrentChunkIndex() const;
int32 GetActiveChunkCount() const;
int32 GetPoolSize() const;
TArray<FPinkCabChunkId> GetActiveChunkIds() const;
EPinkCabLongitudinalTravelDirection GetStableTravelDirection() const;
```

A testable explicit update entrypoint is allowed if it is production-meaningful, e.g. `RefreshForTrackedActor()` or `RefreshForState(Location, Velocity)`; do not add test-only public methods.

---

## Task 0 — exact preflight + MetaRoad admission probe

**Purpose:** prove the installed editor/tooling state before any implementation depends on it.

### Steps

1. Fresh Git preflight on the Windows PINK-CAB workspace:
   - repo root;
   - branch;
   - HEAD;
   - upstream;
   - `git status --short`;
   - untracked/staged/user changes;
   - current PR state.
2. Verify UE 5.8 editor path.
3. Verify MetaRoad Free is actually enabled/available in the local project or engine plugin inventory.
4. Record MetaRoad plugin/version/path and whether it is project-installed or engine-installed.
5. Open the accepted/current PINK-CAB map read-only and verify the Tatra baseline still launches before road work.
6. If the workspace is dirty, do not normalize or overwrite it; isolate via worktree or runner checkout.
7. Record the preflight evidence in CD-869.

**Expected:** clean isolated implementation workspace, exact branch/HEAD known, MetaRoad Free discoverable. If MetaRoad is not accessible, stop with a real dependency blocker before writing MetaRoad-dependent content.

**Commit:** none.

---

## Task 1 — pure chunk math and direction model (TDD)

**Files:**
- create `PinkCabL1EndlessRoadModel.h/.cpp`;
- create `PinkCabL1EndlessRoadModelTests.cpp`.

### RED

Write tests first:

1. `0 cm → chunk 0`;
2. `99999.9 cm → chunk 0`;
3. `100000 cm → chunk 1`;
4. `-0.1 cm → chunk -1`;
5. `-100000 cm → chunk -1`;
6. `-100000.1 cm → chunk -2`;
7. positive travel window = `N-2..N+4`;
8. negative travel window = `N+2..N-4`;
9. all seven coords unique;
10. velocity above +threshold selects Positive;
11. velocity below -threshold selects Negative;
12. near-zero velocity retains the last stable direction.

Run focused:
`Automation RunTests PinkCab.World.L1EndlessRoad.Model`

**Expected RED:** tests fail because model does not exist/behavior is unimplemented.

### GREEN

Implement the smallest pure model satisfying the cases.

Important: use mathematical floor for negative coordinates; do not rely on truncation toward zero.

Run focused suite again.

**Expected GREEN:** all model tests pass.

### Commit

`feat(CD-869): add deterministic L1 endless-road chunk model`

**Produces:** signed chunk index + travel direction + seven-coordinate window.  
**Consumed by:** Tasks 2, 4 and 6.

---

## Task 2 — 14-lane straight RoadGraph topology (TDD)

**Files:**
- extend `PinkCabL1EndlessRoadModel.h/.cpp`;
- extend `PinkCabL1EndlessRoadModelTests.cpp`.

### RED

Add tests proving:

1. one logical chunk contributes exactly 14 ground lanes;
2. lane IDs are stable for same City + same chunk;
3. different chunk index produces different stable lane IDs;
4. five express A + five express B + two local A + two local B are present;
5. direction-A lanes connect from chunk N toward N+1;
6. direction-B lanes connect from chunk N toward N-1;
7. adding adjacent chunks yields valid continuity at the seam;
8. repeated reconstruction produces the same topology signature.

**Expected RED:** topology builder absent.

### GREEN

Implement `AppendStraightChunkLanes` using existing:
- `FPinkCabRoadGraph::MakeNodeId`;
- `FPinkCabRoadGraph::MakeLaneId`;
- `FPinkCabLogicalLane`.

Use stable road/lane index ranges documented in code comments so future variants do not renumber existing v1 lane IDs.

Run:
- focused L1 endless-road model tests;
- existing `PinkCab.World.CityRuntime`;
- existing route/world graph focused tests.

### Commit

`feat(CD-869): add deterministic L1 straight road topology`

**Produces:** stable 14-lane logical chunk graph.  
**Consumed by:** Tasks 4, 6 and future traffic/routing.

---

## Task 3 — author and admit the MetaRoad 1000 m visual module

**Files/content:**
- MetaRoad editor network/source content under a clearly marked authoring folder if its license/package supports committing source;
- baked road output under `/Game/World/L1/Road/`;
- no gameplay C++ changes in this task.

### Authoring contract

Build exactly one straight 1000 m module matching the approved spec:

- express width 360 cm × 10;
- local width 320 cm × 4;
- center green median 800 cm;
- two green/service separators 400 cm each;
- 100 cm outer safety shoulders;
- flat Z;
- start/end cross-section identical;
- no buildings/signs/trash/traffic/parking/wallride.

### Materials

Create/assign the smallest practical family:

- dark worn asphalt;
- dirty readable white markings;
- green separators;
- subtle wet response;
- macro breakup;
- no mirror-road;
- no unique 1000 m texture.

Prefer reusable texture density and master/instance parameters over unique kilometer textures.

### Bake/admission verification

Add/prepare editor evidence that the baked module:

1. loads;
2. has approximately 100000 cm longitudinal bounds;
3. has collision;
4. has no unintended MetaRoad runtime ownership requirement;
5. starts/ends at seam-compatible transforms;
6. is reusable as the same physical representation for different logical ChunkIds.

If MetaRoad outputs multiple meshes, wrap them as one chunk representation without changing the logical contract.

### Commit

`feat(CD-869): admit MetaRoad L1 straight 1000m module`

**Produces:** canonical baked visual/collision asset(s).  
**Consumed by:** Tasks 4–7.

---

## Task 4 — reusable physical road-chunk actor (TDD)

**Files:**
- create `PinkCabL1RoadChunkActor.h/.cpp`;
- create initial `PinkCabL1EndlessRoadRuntimeTests.cpp`.

### RED

Write runtime/editor tests proving:

1. unbound actor reports no logical chunk;
2. valid bind stores exact stable ChunkId/index;
3. chunk 0 places at X=0;
4. chunk +7 places at X=700000 cm;
5. chunk -3 places at X=-300000 cm;
6. rebind changes logical identity without spawning a second actor;
7. clear binding leaves actor reusable;
8. missing baked asset reports invalid visual readiness and does not substitute greybox geometry.

**Expected RED:** actor does not exist.

### GREEN

Implement the actor with one clear responsibility: reusable physical representation only.

Use baked MetaRoad mesh/prefab references. Do not put CityCode generation, RoadGraph construction, vehicle lookup or streaming policy in this actor.

Run focused runtime tests.

### Commit

`feat(CD-869): add reusable L1 road chunk representation`

**Produces:** rebindable 1000 m physical chunk.  
**Consumed by:** Task 5.

---

## Task 5 — seven-slot endless-road streamer (TDD)

**Files:**
- create `PinkCabL1EndlessRoadStreamer.h/.cpp`;
- extend `PinkCabL1EndlessRoadRuntimeTests.cpp`.

### RED

Tests first:

1. initial refresh creates/activates exactly seven pool slots;
2. active logical IDs are unique;
3. active count never exceeds seven;
4. positive travel at chunk N binds N-2..N+4;
5. negative travel binds N+2..N-4;
6. entering N+1 reuses a leaving slot rather than increasing pool size;
7. after 100 sequential chunks pool size remains seven;
8. after forward travel and return, original logical ChunkIds are reconstructed;
9. direction changes only after hysteresis threshold and does not flap at standstill;
10. teleport from chunk 0 to chunk 1000 directly reconstructs one bounded window;
11. missing visual readiness does not allocate beyond seven;
12. streamer uses `FPinkCabWorldMaterializationPolicy` rather than duplicating selection/dematerialization logic.

### GREEN

Implement:

- fixed pool creation once;
- tracked Actor position/velocity observation;
- cheap Tick allowed only for detecting relevant state change;
- no materialization work when chunk + stable direction are unchanged;
- candidate logical chunks sent through existing materialization policy;
- deterministic mapping from requested logical IDs to free/reusable pool slots;
- diagnostics.

Do not modify vehicle physics/input.

Run:
- `PinkCab.World.L1EndlessRoad`;
- `PinkCab.World.Materialization`.

### Commit

`feat(CD-869): stream bounded L1 road chunks around the Tatra`

**Produces:** runtime endless materialization.  
**Consumed by:** Tasks 6–8.

---

## Task 6 — integration map and current Tatra wiring (TDD/editor generation)

**Files/content:**
- create `PinkCabL1EndlessRoadMapGenerator.cpp`;
- create/update `/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight`;
- create `PinkCabL1EndlessRoadMapTests.cpp`;
- do **not** switch `GameDefaultMap` yet.

### RED

Map tests require:

1. map exists/loads;
2. exactly one current `APinkCabChaosTatraPawn` exists;
3. exactly one endless-road streamer exists;
4. streamer tracks the Tatra;
5. exactly seven chunks are active after initialization;
6. baked road asset(s) are bound;
7. map contains no legacy finite L1 greybox corridor as the drive surface;
8. MapCheck passes.

### GREEN

Generate a dedicated candidate map using the existing editor-test generator pattern.

Keep current accepted default map untouched until human acceptance.

Spawn the Tatra at a safe position on the correct-direction express carriageway, not on a seam.

### Commit

`feat(CD-869): add L1 endless straight road candidate map`

**Produces:** drivable candidate map.  
**Consumed by:** Tasks 7–9.

---

## Task 7 — runtime/physics seam verification

**Files:**
- extend `PinkCabL1EndlessRoadRuntimeTests.cpp` and/or map tests only as needed.

### RED

Add integration checks that reproduce the user-visible failure modes:

1. Tatra can cross at least one 1000 m seam without falling/launching;
2. at least two Chaos wheels remain in road contact after crossing;
3. road collision exists on both sides of the seam;
4. active pool count stays seven before/after crossing;
5. current chunk advances exactly once;
6. no duplicate logical IDs appear during rollover.

If a test cannot practically drive a full kilometre in automation, move the pawn/fixture to a controlled pre-seam starting position and cross only the seam. Do not weaken the seam assertion.

### GREEN

Fix only world/road representation defects found by the test. Do not compensate by changing vehicle suspension/Chaos settings unless an independent vehicle regression proves the baseline itself is wrong.

### Commit

`test(CD-869): prove L1 road seam and Chaos contact continuity`

---

## Task 8 — long-run deterministic soak

**Files:**
- extend pure/runtime tests; no content redesign.

### RED/GREEN proof

Automated long-run must cover:

- at least 100 sequential chunks / 100 km logical travel;
- maximum active physical road chunks = 7;
- no duplicate active IDs;
- no monotonic growth of active actor count;
- return to origin reconstructs origin logical window;
- existing 2048-window `FPinkCabWorldMaterializationLongRunBoundedTest` remains green.

Where practical, also capture memory/actor-count diagnostics at start/middle/end. Do not invent a memory-pass threshold without a reproducible baseline; the hard acceptance in this task is bounded representation count and no progressive Actor growth.

### Commit

`test(CD-869): lock 100km bounded endless-road soak`

---

## Task 9 — focused verification + full regression + code health

On exact candidate HEAD:

1. build `PinkCabEditor`;
2. run focused:
   - `PinkCab.World.L1EndlessRoad`;
   - `PinkCab.World.Materialization`;
   - relevant CityRuntime/RoadGraph/Route tests;
   - candidate map tests;
3. run full `Automation RunTests PinkCab`;
4. run strict code-health `-RequireZeroDebt`;
5. scan UE logs for crash/fatal/assert/ensure/error signals and classify any expected automation text correctly;
6. verify git diff contains no unrelated cockpit/input/vehicle behavior edits.

Because this feature adds/changes `Content/`, the control-plane recook guard must route final delivery through **`release_gate`**, not the code-only `human_gate`.

No completion claim is allowed from compile-only evidence.

### Commit

Only if verification requires evidence/docs update:

`docs(CD-869): record endless-road technical evidence`

---

## Task 10 — independent code review

Review the full range:

`ed969edac845248464f8442a78899d26e6dfecde..candidate HEAD`

Review focus:

- negative-coordinate floor correctness;
- direction hysteresis and reverse behavior;
- pool reuse without logical-ID aliasing;
- stale binding after slot reuse;
- use of existing materialization policy vs duplicated streamer policy;
- RoadGraph direction and seam continuity;
- missing-asset fail-closed behavior;
- UObject/Actor lifetime and pointer validity;
- Tick cost and accidental per-frame materialization;
- MetaRoad runtime coupling;
- collision seam risk;
- world-coordinate precision at the 100 km proof boundary;
- no vehicle-control regression.

Critical/Important findings must be fixed with RED→GREEN regression tests before proceeding. Minor findings are recorded separately.

---

## Task 11 — release-gate package + human road gate

Because baked road content changed, use the full content-aware/release delivery lane.

Human candidate must launch the dedicated endless-road map/build with exact SHA evidence.

Human check:

1. road looks like a real dark/worn PINK-CAB road rather than cubes;
2. road is visible far enough ahead that construction is never seen;
3. drive forward across repeated 1000 m seams;
4. no visible gaps;
5. no collision kick/pop at seams;
6. no recurring obvious hitch at rollover;
7. turn around and drive backwards through prior chunks;
8. road materializes correctly in the new travel direction;
9. no obvious progressive degradation during the test.

The planned 100 km soak can be automation-assisted; the user does not need to manually drive 100 km unless useful for feel. Human acceptance focuses on visible/physical quality and seamlessness.

If accepted, only then consider switching the canonical default/dev map from the existing ChaosWeave baseline to the new Level 1 road candidate.

---

## Task 12 — Jira/Confluence/Git closeout

After human acceptance:

1. record exact SHA, Actions run, asset paths, chunk/window settings and human result in CD-869;
2. update durable Level 1 implementation status in Confluence only where the runtime evidence changes stable project truth;
3. do not mark the full L1→L2→L1 parent result complete merely because the straight endless-road foundation passes;
4. merge the feature branch through reviewed PR;
5. keep CD-869 open or split/continue its remaining L1→L2 route work according to the evidence; do not falsely close the broader route objective.

## Review focus / known risk ledger

The implementation must deliberately check these risks:

- **R1:** negative world coordinates truncate incorrectly instead of floor;
- **R2:** reverse travel keeps the positive-X ahead window;
- **R3:** velocity sign chatters near zero and continually rebinds chunks;
- **R4:** pool slots retain old ChunkId/material state after reuse;
- **R5:** a missing MetaRoad asset silently falls back to greybox and masks packaging errors;
- **R6:** lane topology for reverse carriageway points the wrong way;
- **R7:** 1000 m mesh bounds or collision do not meet exactly at the seam;
- **R8:** runtime accidentally depends on MetaRoad editor classes/plugin;
- **R9:** the streamer duplicates `FPinkCabWorldMaterializationPolicy`;
- **R10:** content changes are delivered via code-only human gate and therefore test stale cooked content;
- **R11:** long-run actor/pool count grows with distance;
- **R12:** world-coordinate precision becomes visible during the 100 km proof.

## Definition of technical completion

Technical completion requires fresh evidence on exact candidate HEAD for:

- MetaRoad baked road admitted and loadable;
- focused endless-road suite PASS;
- existing materialization long-run PASS;
- full PINK-CAB automation PASS;
- build PASS;
- zero-debt PASS;
- release/full-cook candidate PASS;
- independent review with no unresolved Critical/Important findings.

Human visual/physical acceptance remains a separate final gate.
