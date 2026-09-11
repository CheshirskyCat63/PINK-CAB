# PINK CAB · Level 1 Presentation Zero QA

**Status:** CURRENT QA MIRROR / NOT RUNTIME VERIFIED
**Jira gate:** `CD-719`
**Confluence QA contract:** page `6750209`
**Presentation Bible:** Confluence `8323081`, Git `docs/PINK_CAB_LEVEL1_PRESENTATION_BIBLE.md`

## Purpose

Prove one representative direct Level 1 corridor is recognizably PINK CAB at driving speed while keeping presentation systems bounded and independent of premium-only rendering features.

## Fixture identity

Every run records:

- exact commit/build;
- engine/platform/config;
- `CityCode`;
- traffic seed;
- corridor/chunk IDs;
- generator/content/schema versions;
- current Tatra center-lamp numeric profile and whether it is LOCKED or `EXPERIMENTAL / NON-AUTHORITY`.

## Required visual tests

| ID | Test | Pass condition |
|---|---|---|
| `L1-PRES-001` | Road read | five-lane express/right-side exit/merge/local/frontage grammar reads at speed |
| `L1-PRES-002` | Identity kit | giant poplars, clipped hedge, bright grass/dandelions and P-6V-like edge read immediately |
| `L1-PRES-003` | Density | frontage/micromobility/trash/ads feel dense without one active Actor per visible object |
| `L1-PRES-004` | LOD/HLOD | near→mid→far transitions preserve PS2 silhouette/composition without unacceptable pop or gameplay ambiguity |
| `L1-PRES-005` | Center lamp | center lamp is physically centered, steering-linked and visibly tracks steering |
| `L1-PRES-006` | Beam hierarchy | center beam reads somewhat farther than outer pair using the current approved numeric profile |
| `L1-PRES-007` | Traffic lights | nearby head/tail/brake/indicator states dynamically alter the moving image |
| `L1-PRES-008` | Ads | hologram/ad visual complexity uses bounded real dynamic emitters and remains readable |
| `L1-PRES-009` | Shadows | major nearby moving light/occluder pairs produce readable cheap moving shadows |
| `L1-PRES-010` | Non-premium path | test scene does not require ray tracing, path tracing or expensive realtime GI to meet visual acceptance |

## Required audio tests

| ID | Test | Pass condition |
|---|---|---|
| `L1-AUD-001` | Tatra hierarchy | engine/driveline/road/contact cues remain readable in traffic |
| `L1-AUD-002` | Traffic aggregation | near individual → mid grouped → far lane-bed transition is stable and not obviously discontinuous |
| `L1-AUD-003` | Freight | ceiling propulsion/container/coupling/pass-by layers remain readable without per-container emitter explosion |
| `L1-AUD-004` | Frontage | crowd/micromobility/entrance/projector details sell life without masking driving-critical cues |
| `L1-AUD-005` | Cleanup | removed/streamed-out actors release transient audio ownership and pool entries |

## Required VFX tests

| ID | Test | Pass condition |
|---|---|---|
| `L1-VFX-001` | Debris | burst/trash/windshield effects terminate/recycle correctly |
| `L1-VFX-002` | Hologram | ad flicker/sequence effects stay within concurrency and overdraw budget |
| `L1-VFX-003` | Magnetic | attach/stable/residual/detach effects reflect gameplay state and clean up |
| `L1-VFX-004` | Wallride | contact/deformation fake/spark effects do not leave persistent systems |
| `L1-VFX-005` | Incident | smoke/debris/sparks clean up after incident lifecycle |

## Runtime evidence fields

Record at fixed capture markers and continuously where possible:

- active Actor count;
- active Component count;
- ISM/HISM instance counts;
- HLOD/proxy state;
- draw/submission metrics;
- GPU frame time;
- CPU/game-thread frame time;
- FPS avg/min/1% low;
- transparent overdraw;
- material instance count;
- texture/material memory;
- active real dynamic lights;
- active shadow casters;
- shadow method/resolution/distance class;
- active audio emitters;
- active voices/concurrency;
- audio memory;
- active particle/VFX systems;
- pooled transient-object counts;
- loaded/visible chunks;
- warm/peak process memory.

## 30-minute deterministic soak

Run the same CityCode/traffic fixture for at least 30 minutes including normal road, advertisement clusters, traffic incidents, debris/wallride transition, ceiling/freight exposure and parking-ingress vicinity.

Fail if, after warm-up, unowned counts trend monotonically upward without a legitimate bounded reason:

- chunks;
- Actors/Components;
- debris;
- transient lights;
- audio emitters;
- VFX/particles;
- ad systems;
- crowd actors;
- incident props.

## Evidence artifact package

A result package contains:

- `build.txt` / exact commit ID;
- fixture manifest with CityCode/seed/chunk IDs;
- performance capture;
- light/shadow counter capture;
- audio counter capture;
- VFX/pool counter capture;
- screenshots/video from cockpit;
- errors/warnings log;
- expected-vs-observed table;
- final PASS/FAIL with failing test IDs.

## Verification rule

This document and `CD-719` being well-specified do not mark Presentation Zero VERIFIED. Only a fresh exact-build runtime run with the evidence above can do so.
