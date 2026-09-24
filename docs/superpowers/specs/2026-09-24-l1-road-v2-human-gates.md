# PINK-CAB Level 1 Road V2 · Human-Gated R1→R6 Contract

**Owner approval:** 2026-09-24  
**Active project:** PINK-CAB  
**Jira:** CD-869  
**Execution rule:** R1 → HUMAN acceptance → R2 → HUMAN acceptance → ... → R6.

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

**Status: IN PROGRESS · 2026-09-24.**

Add the actual raised separator/curb construction required for the road to read as built geometry rather than a flat drawing. Preserve accepted R1 topology and streaming unchanged.

R2 implementation contract:
- keep the accepted 66.8 m outer road envelope and 1000 m seam unchanged;
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

Frozen until R2 owner acceptance.

## R4 — Road Visual v2

Frozen until R3 owner acceptance. This stage owns the finished asphalt/road visual treatment after physical separators and markings are accepted.

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
