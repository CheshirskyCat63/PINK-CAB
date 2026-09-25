# PINK-CAB Level 1 Road V2 · Human-Gated R1→R6 Contract

**Owner approval:** 2026-09-24  
**Active project:** PINK-CAB  
**Jira:** CD-869  
**Execution rule:** R1 → HUMAN acceptance → R2 → HUMAN acceptance → ... → R6.  
**Canonical road state (2026-09-25):** R1/R2/R3 HUMAN ACCEPTED / INTEGRATED / FROZEN; R4 is NEXT and not started. R3 geometry is 5×4.50 m express + 2×4.00 m local per direction, 79.0 m design / 79.15 m physical runtime assembly.

## Frozen foundation

The existing endless-road foundation is retained:
- logical chunk length: 1000 m;
- seven physical road chunks;
- 2 behind / current / 4 ahead, mirrored by travel direction;
- stable CityCode/ChunkId reconstruction;
- existing PINK-CAB RoadGraph ownership;
- forward/reverse streaming;
- content-aware packaged HUMAN delivery.

No R-stage may rewrite the streaming system unless an isolated regression proves it necessary.

## R1 — 1000 m module topology v2

**Status: HUMAN ACCEPTED · 2026-09-24**

Owner acceptance note: the topology/streaming gate is accepted as-is. The current road still reads as a flat/simple visual representation rather than finished road construction. Raised curbs/separators, physical marking geometry/material treatment and the finished road visual are explicitly deferred to R2/R3/R4 and do not reopen R1.

R1 changes topology only. It does **not** change road textures, road markings, raised curbs/medians, physical road friction, or tire calibration.

The cross-section remains:
- 5 express lanes per direction;
- 2 local/access lanes per direction;
- central separation;
- side service separation between express and local.

Each physical 1000 m module contains two deterministic right-side access windows per side.

Physical X-order:
- Access Window A: 150 m → 450 m; fully open around 275–325 m.
- Access Window B: 550 m → 850 m; fully open around 675–725 m.
- 0–150 m and 850–1000 m remain clean straight seam buffers.

Positive-X travel:
- Window A = express → local exit.
- Window B = local → express merge.

Negative-X travel:
- Window B = express → local exit.
- Window A = local → express merge.

Authoring mechanism:
- preserve total side separation width at 4.0 m;
- add a variable-width paved connector surface inside that 4.0 m band;
- complementary green/service-separator width = 4.0 m − connector width;
- connector width is 0 outside access windows, ramps to 4.0 m, holds briefly, then returns to 0;
- local and express main carriageways keep their canonical lane widths and seam positions.

R1 HUMAN acceptance checks only:
1. the module visibly repeats every 1000 m;
2. both directions have a readable right-side express/local access connection;
3. two access windows are visible per kilometre;
4. the start/end 150 m seam regions remain straight and identical;
5. driving express → local and local → express is physically possible;
6. no collision gap/kick appears at chunk seams;
7. endless streaming remains stable.

## R2 — median/separators

**Status: HUMAN ACCEPTED / INTEGRATED · 2026-09-25.**

Add the actual raised separator/curb construction required for the road to read as built geometry rather than a flat drawing. Preserve accepted R1 topology and streaming unchanged.

R2 implementation contract:
- preserve the then-accepted R2 **66.8 m** outer road envelope and 1000 m seam unchanged; **historical R2 value only — superseded by owner-accepted R3 to 79.0 m design / 79.15 m physical runtime assembly**;
- central median is physically raised across the kilometre;
- service separators between express and local roads are physically raised and open only where the accepted R1 access connector opens;
- curb geometry visibly defines median/separator road edges;
- local-road outside edge receives a continuous curb/road-edge treatment inside the accepted envelope;
- curb/separator collision is present but low enough to remain believable road-edge geometry rather than a wall;
- R1 access windows remain physically traversable;
- no lane-marking pass, asphalt visual rework, road-friction change or tire calibration is permitted in R2.

R2 HUMAN acceptance:
1. road no longer reads as a flat painted sheet;
2. central median has clear height/edge construction;
3. express/local separator has clear raised geometry and visible curbs;
4. R1 exit/merge openings remain unobstructed and drivable;
5. outer local-road edge has a readable curb;
6. chunk seams remain visually/collision clean;
7. accepted R1 streaming and controls remain unchanged.

## R3 — road markings

**Status: HUMAN ACCEPTED / INTEGRATED / FROZEN · 2026-09-25.**

R3 is owner-accepted. Native MetaRoad road markings are visible and use the project-owned `M_PC_L1_Mark` material. R1 streaming/topology, R2 native raised construction/access openings and the accepted vehicle/H-gate behavior remain frozen.

Owner-approved R3 transverse calibration:
- express lane width: **4.50 m** (3.60 m × 1.25);
- local lane width: **4.00 m** (3.20 m × 1.25);
- lane count remains 5 express + 2 local per direction;
- design road envelope is **79.0 m**;
- physical runtime assembly is **79.15 m** including the native MetaRoad outer-curb profile;
- the R2 separator/curb construction system and access-opening semantics are preserved; this calibration supersedes only the earlier R2-width snapshot.

R3 does not own final asphalt visual treatment, road friction or tire calibration.

## R4 — Road Visual v2

**Status: NEXT HUMAN-GATED STAGE.**

R3 is frozen. R4 owns the finished asphalt/road visual treatment after accepted physical separators, markings and transverse calibration.

## R5 — dry-asphalt physical surface

Frozen until R4 owner acceptance.

## R6 — tire calibration

Frozen until R5 owner acceptance.

## R1 acceptance evidence

- owner human acceptance: 2026-09-24;
- accepted runtime package SHA: `ba6c5e3221bb6d87f025e26b4956ae0eab5c24bd`;
- packaged frozen-control recovery run: `35997397505` — PASS;
- control-plane verification HEAD at acceptance: `719e0f0c7df16fd4542791356c60bb7abdf66a26`;
- accepted limitation: no finished raised curbs/separators, road markings or final road visual yet; those remain ordered follow-up gates R2→R4.


## R2 acceptance evidence

- owner HUMAN acceptance: 2026-09-25;
- accepted HUMAN source SHA: `13f145bd0ad0427ec62b80479624219bc9cf4dec`;
- packaged HUMAN run: `36082568631` — SUCCESS;
- accepted construction: native MetaRoad 3.2 curb/sidewalk/profile geometry; primitive `RoadConstruction` overlay absent;
- accepted access behavior: R1 express↔local openings remain physically traversable;
- accepted H-gate calibration: 1/2 extreme-left, 5/R extreme-right, broad 3/4 middle zone, doubled fore/aft travel;
- final focused verification run: `36083861324` — SUCCESS;
- verified SHA: `072400cf3d865e9197af49e27c56b473b0ad0206`;
- PR #17 squash integration SHA: `ba96296bd90bd5975637c13a5192ebe4cc7366fb`;
- R2 is frozen; R3 is the next human-gated stage.


## R3 acceptance evidence

- owner HUMAN acceptance: 2026-09-25;
- corrected visible-mark asset commit: `5ce13018f048674bf333d7e985bab4d5322afca0`;
- widened native MetaRoad asset commit: `af8002a04b32fd4d12446248d15e8a1872f06bd0`;
- exact accepted HUMAN package source: `aa5d7c99c72b805e5eee8c7d45bc6d2390ebb752`;
- lane-width calibration/bake run: `36094789710` — SUCCESS;
- final exact-package delivery/recovery run: `36095423828` — SUCCESS;
- native bake inventory: 1 road surface / 1 sidewalk layer / 32 curbs / 50 marking meshes;
- design bake bounds: 100000 cm × 7900 cm × 16.5 cm;
- physical runtime transverse assembly: 7915 cm including native outer-curb protrusion;
- strict zero-debt: PASS;
- `PinkCab.World.L1Road.R3.NativeMetaRoadMarkings`: PASS;
- `PinkCab.World.L1Road.R2.NativeMetaRoadAsset`: PASS;
- access-opening regression: PASS;
- `PinkCab.World.L1EndlessRoad`: PASS;
- `PinkCab.World.Materialization`: PASS;
- vehicle H-gate regression: PASS;
- fresh signed Win64 package: PASS;
- packaged OS-input: PASS;
- packaged smoke: PASS;
- PR #22 squash integration SHA: `55e9fd8481a114265fd43f1c6433d86df53ba991`;
- R3 is HUMAN ACCEPTED / INTEGRATED / FROZEN; R4 is next and remains unstarted until its own execution begins.
