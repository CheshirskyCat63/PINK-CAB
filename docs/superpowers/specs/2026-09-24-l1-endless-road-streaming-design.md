# PINK-CAB CD-869 · Level 1 Endless Straight Road Streaming Design

**Status:** FOUNDATION IMPLEMENTED · STREAMING AUTHORITY RETAINED · R3 ROAD GEOMETRY SUPERSEDES ORIGINAL CALIBRATION  
**Date:** 2026-09-24  
**Jira:** CD-869  
**Historical execution base:** `main@ed969edac845248464f8442a78899d26e6dfecde`  
**Current road integration:** PR #22 squash `55e9fd8481a114265fd43f1c6433d86df53ba991`  
**Implementation branch:** `feat/CD-869-l1-endless-road`  
**Road gate state (2026-09-25):** R1/R2/R3 HUMAN ACCEPTED / INTEGRATED / FROZEN; R4 is NEXT / NOT STARTED; R5–R6 remain frozen.

## 1. Player result

The player can enter the current PINK-CAB Tatra and drive indefinitely along the first playable Level 1 city road while the road is materialized ahead of the vehicle and retired behind it.

The first implementation intentionally excludes houses, trash, traffic signs, parked cars, pedestrians, wallride, freight ceiling, services and route variants. It delivers the stable endless-road foundation first.

The road must already look like a believable road rather than greybox cubes: dark worn asphalt, readable lane markings, green separation strips and clean repeatable geometry.

## 2. Canonical Level 1 cross-section · R3 accepted supersession

The source world canon remains Confluence pages 8388630, 6455318 and 8323081.

For the first straight module, the materialized geometry is:

```
outer edge
  two-lane local/access road
  green/service separation strip
  five-lane express carriageway
  central green median
  five-lane opposite express carriageway
  green/service separation strip
  two-lane opposite local/access road
outer edge
```

This produces 14 logical ground lanes in the v1 straight chunk:

- 5 express lanes direction A;
- 5 express lanes direction B;
- 2 local lanes direction A;
- 2 local lanes direction B.

The streaming architecture remains authoritative. The transverse road calibration below is the owner-accepted R3 geometry:

- express lane width: `450 cm` (owner-accepted R3, supersedes original 360 cm calibration);
- local lane width: `400 cm` (owner-accepted R3, supersedes original 320 cm calibration);
- central green median: `800 cm`;
- each green/service separation strip: `400 cm`;
- outer safety shoulder beyond each local road: `100 cm`.

The current accepted authored road design envelope is `7900 cm`; physical runtime assembly is `7915 cm` including native outer-curb profile protrusion. The original `6680 cm` value is historical pre-R3 calibration only.

Parking bays, building sidewalks and service entrances are omitted from this first visual module. Their future addition must not alter the chunk identity or streaming architecture.

## 3. Spatial contract

One logical road chunk is exactly:

- `1000 m` longitudinal length;
- `100000 cm` in Unreal units;
- straight;
- flat for v1;
- tangent `+X` at both ends;
- exact seam-compatible start/end cross-section.

The chunk starts at local `X=0` and ends at local `X=100000`.

Every road edge, lane center, separator edge, marking path and collision boundary that crosses the seam must have identical lateral coordinates at `X=0` and `X=100000`.

A valid chunk must therefore satisfy:

`Chunk[N].End == Chunk[N+1].Start`

within the authored tolerance used by the bake.

The existing one-kilometre project convention in `APinkCabL1GreyboxCorridor` is retained.

## 4. MetaRoad role

MetaRoad Free is an **editor authoring/bake tool only** for this feature.

It owns no PINK-CAB runtime state.

Authoring flow:

```
MetaRoad Free
  → author one 1000 m straight road
  → assign road/marking/median material families
  → bake geometry
  → assemble one reusable PINK-CAB road-chunk prefab/actor
```

The runtime must not require MetaRoad graph ownership, MetaRoad traffic ownership, CityCode ownership, persistence ownership or per-frame procedural MetaRoad generation.

The baked chunk remains usable by PINK-CAB even if the MetaRoad authoring plugin is later removed or upgraded.

## 5. Existing PINK-CAB systems reused

No competing world system may be introduced.

The implementation reuses:

- `FPinkCabCityIdentity` for stable city identity;
- `FPinkCabChunkId` for logical chunk identity;
- `FPinkCabRoadGraph` for PINK-CAB lane identity/topology;
- `FPinkCabWorldMaterializationPolicy` for bounded materialize/dematerialize decisions;
- existing world tests for deterministic forward/reverse reconstruction;
- current Native Chaos vehicle runtime unchanged.

The existing `PinkCabWorld` module remains the owner of world identity and materialization policy.

## 6. Runtime architecture

### 6.1 Logical chunk index

The player's longitudinal position maps to a signed logical chunk coordinate.

For a 1000 m chunk:

`ChunkIndex = floor(LongitudinalCm / 100000.0)`

Negative indexes are valid so reverse travel is symmetric.

The stable PINK-CAB chunk ID is derived from:

`CityIdentity + {Longitudinal=ChunkIndex, Lateral=0, Layer=0}`

The physical road geometry being reused does not change that logical identity.

### 6.2 Materialization window

The initial v1 window is fixed at seven physical chunks:

- 2 chunks behind relative to current travel direction;
- current chunk;
- 4 chunks ahead relative to current travel direction.

For current chunk `N` while travelling toward positive X, the desired set is:

`[N-2, N-1, N, N+1, N+2, N+3, N+4]`

For current chunk `N` while travelling toward negative X, the desired set is:

`[N+2, N+1, N, N-1, N-2, N-3, N-4]`

Travel direction is derived from authoritative vehicle longitudinal velocity with hysteresis. Near standstill, the last non-zero travel direction is retained so the materialization window does not flap while parking or reversing the steering wheel.

The window is recomputed when the player changes logical chunk or when stable travel direction changes. It is not rebuilt every frame.

Forward and reverse driving share one streamer and one signed-coordinate model. No separate reverse streamer exists.

### 6.3 Pooling

The system creates at most seven physical road-chunk actors/components for this pass.

Leaving chunks are not repeatedly destroyed and recreated during ordinary traversal. Their physical representation is returned to the fixed pool, moved to the next required kilometre transform and rebound to the new logical `ChunkId`.

The bounded active representation count therefore remains constant over long travel.

### 6.4 Physical placement

Chunk `N` is placed at:

`WorldX = N * 100000 cm`

for the v1 proof.

Logical identity is kept separate from physical representation so future origin-rebasing or local-coordinate materialization can be introduced without changing saves, CityCode or chunk IDs.

The v1 acceptance run is 100 km, which is sufficient to validate the streaming/pooling behavior before adding any origin-rebasing complexity.

## 7. Runtime components

### `APinkCabL1EndlessRoadStreamer`

Single orchestration actor for this feature.

Responsibilities:

- observe the authoritative Tatra/player longitudinal position;
- resolve the current logical chunk coordinate;
- construct candidate chunks for the required local window;
- call `FPinkCabWorldMaterializationPolicy`;
- acquire/rebind/reposition pooled visual chunk actors;
- release chunks that leave the active set;
- expose bounded diagnostic counters for tests/logging;
- never own taxi, traffic, persistence, vehicle physics or MetaRoad authoring behavior.

Tick is permitted only as a cheap position observation path. Expensive materialization work occurs only when the chunk index changes.

### `APinkCabL1RoadChunkActor`

Reusable physical representation of one 1000 m road module.

Responsibilities:

- hold the baked chunk geometry/prefab reference;
- own collision for the visible road representation;
- bind one logical `FPinkCabChunkId`;
- apply deterministic per-chunk material parameters when supported;
- expose seam/identity diagnostics.

It contains no generator, save, traffic or route logic.

### PINK-CAB RoadGraph binding

For every logical chunk, the runtime creates or resolves stable lane IDs for the 14 lanes using existing `FPinkCabRoadGraph::MakeLaneId`.

The straight-v1 graph connects each compatible lane in chunk `N` to the same lane index in chunk `N+1`.

The game does not consume MetaRoad's runtime graph as authority.

## 8. Material and presentation contract

The first road pass uses a deliberately small material family.

Required:

- dark graphite worn asphalt;
- macro colour/roughness breakup so kilometre repetition does not read as a tiled checkerboard;
- subtle wetness suitable for the PINK-CAB night baseline;
- readable but slightly worn white lane markings;
- green median/separation surfaces;
- no mirror-like road surface;
- no unique 1000 m texture sheet;
- real-world-density UV/repeatable texture use.

Per-chunk visual variation, if implemented in this pass, must be deterministic from chunk identity and limited to presentation parameters such as:

- macro grime offset;
- asphalt tint;
- wetness amount;
- marking wear.

These parameters cannot alter collision, lane topology or RoadGraph identity.

## 9. Excluded from this pass

Explicitly excluded:

- houses / panel blocks;
- trash/debris dressing;
- traffic signs and enforcement cameras;
- parked vehicles;
- traffic simulation;
- pedestrians;
- parking bays;
- service entrances;
- right exits/merges;
- gentle curves;
- elevation changes;
- wallride;
- freight ceiling;
- poplar jump windows;
- Level 2;
- route objectives.

Those become later chunk variants or CD-869/CD-870 follow-on work after the endless straight foundation passes.

## 10. Failure behavior

The streamer is fail-closed.

If the road chunk prefab/asset is unavailable:

- do not silently replace it with unrelated city geometry;
- emit a clear PINK-CAB diagnostic;
- retain the last valid active set where possible.

If a requested logical chunk cannot be materialized:

- do not grow the pool;
- do not duplicate a logical `ChunkId`;
- report the missing binding through diagnostics/evidence.

If the player teleports across several chunks, the next materialization update reconstructs the bounded seven-chunk window around the resolved current index rather than iterating through every skipped kilometre.

## 11. Tests

TDD coverage must include:

1. signed longitudinal position → correct chunk index;
2. exact boundary behavior at 0 / ±100000 cm;
3. desired seven-chunk window for positive-X travel;
4. direction-flipped seven-chunk window for negative-X travel;
5. standstill hysteresis retains the last stable travel direction;
6. stable `ChunkId` for the same city/chunk after leaving and returning;
7. no duplicate active logical IDs;
8. active physical representation count never exceeds seven;
9. long-run traversal of at least 100 chunks without pool growth;
10. existing 2048-window materialization tests remain green;
11. lane IDs remain deterministic across adjacent straight chunks;
12. teleport/large-index jump rebuilds one bounded window;
13. missing visual asset fails visibly and does not create unbounded fallback actors.

## 12. Runtime acceptance

Automated acceptance:

- focused `PinkCab.World` endless-road tests pass;
- existing world/materialization regression remains green;
- build succeeds on exact candidate;
- code-health remains zero-debt.

Human acceptance candidate:

- launch the current Tatra on the new Level 1 straight road;
- drive forward continuously;
- road is already present several kilometres ahead;
- no visible void appears at a 1000 m boundary;
- no obvious collision seam kicks the vehicle;
- no recurring visible hitch at kilometre rollover;
- travel at least 100 km in the test/soak path without active-road count growth or progressive memory growth;
- turn around and drive back;
- road materializes behind the original travel direction using the same logical chunk identities;
- visual road quality is acceptable as an initial PINK-CAB road foundation.

## 13. Evidence

Record:

- exact implementation SHA;
- MetaRoad/baked road asset path(s);
- chunk length and active-window settings;
- focused automation result;
- long-run bounded-count evidence;
- runtime log with current chunk / materialized count;
- human forward/reverse result.

Technical PASS does not imply owner visual acceptance.

## 14. Rejected approaches

### Pre-place many kilometres in the map

Rejected because it is not endless streaming, scales content size with distance and does not exercise the existing materialization policy.

### Generate MetaRoad geometry continuously at runtime

Rejected because it couples world runtime to an authoring plugin, increases hitch risk and duplicates responsibilities already owned by PINK-CAB.

### Create a second generic streaming framework

Rejected because `FPinkCabWorldMaterializationPolicy`, CityCode and stable chunk identity already exist and are tested.

## 15. Extension path after this gate

After straight-v1 is accepted, the same logical chunk contract can admit additional 1000 m variants without rewriting the streamer:

- `Straight_B`;
- gentle left;
- gentle right;
- rise;
- descent;
- right exit;
- right merge;
- parking/service variant.

CityCode will eventually select variants deterministically. The streaming and identity model remains unchanged.
