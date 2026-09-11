# PINK CAB · Art, LOD, Lighting & Runtime Budget Framework

**Status:** CURRENT PINK CAB BUDGET FRAMEWORK
**Jira:** `CD-590`, Level 1 presentation `CD-714..719`
**Confluence:** page `6717461` Content Budgets; page `8323081` Level 1 Presentation Bible
**Detailed implementation mirror:** `docs/PINK_CAB_LEVEL1_PRESENTATION_BIBLE.md`

> Historical DEADRACE pursuit/police/combat art assumptions are **LEGACY / SUPERSEDED** for PINK CAB. This file is no longer a pursuit-game budget.

## Core rule

Asset quality is judged from the moving first-person Tatra cockpit and from gameplay readability at target speed.

PINK CAB must feel visually dense while runtime remains deliberately cheap:

`HIGH PERCEIVED DENSITY / LOW ACTIVE ENTITY COUNT`

Spend geometry, materials, animation, lighting and audio only where the player can perceive or use them. Everything else should collapse through reuse, instancing, clusters, HLOD/impostors, pooled emitters and theatrical fakery.

## Presentation north star

The Level 1 image is intentionally PS2-style / 2003–2004-era economical rather than physically realistic.

Required tendencies:

- low-poly strong silhouettes;
- compact shared atlases;
- small master-material family;
- bold saturated practical light signals;
- deep dark areas;
- coarse/hard/low-resolution moving shadows where useful;
- visible but controlled repetition;
- dense composition from few asset families;
- no premium-GPU-first dependency.

## Explicit non-requirements

PINK CAB Level 1 does **not** require:

- ray tracing;
- path tracing;
- expensive realtime global illumination;
- premium-only volumetric/area-light stacks;
- physically correct photometry everywhere;
- 4090-class minimum assumptions;
- unique 4K materials for ordinary drive-by props;
- one active Actor/light/emitter per visible object.

## Level 1 production owners

| Area | Jira owner |
|---|---|
| Reusable low-poly mesh/prop kit | `CD-714` |
| PS2 textures/materials/atlases | `CD-715` |
| Dynamic lighting / cheap moving shadows | `CD-716` |
| Audio emitter/ambience kit | `CD-717` |
| VFX / density / pooling / ISM-HISM-HLOD | `CD-718` |
| Integrated Presentation Zero | `CD-719` |

## Geometry budget policy

Exact triangle/LOD ceilings remain OPEN until representative profiling. Temporary authoring values are `EXPERIMENTAL / NON-AUTHORITY`.

Asset decisions must record:

- role and gameplay relevance;
- near/mid/far representation;
- collision proxy ownership;
- LOD/HLOD/impostor strategy;
- ISM/HISM/precombined-cluster eligibility;
- material/atlas family;
- estimated active Actor/Component cost;
- measured draw/memory/frame cost once implemented.

Do not use maximum triangle budgets as targets.

## Required Level 1 kit families

- five-lane express / right decel-accel / local road / legal parking modules;
- curb/gutter/drain/sign/camera support;
- worn asphalt/rut/patch/crack/water visual families;
- giant leafless poplar / clipped hedge / bright grass / dandelion / P-6V-like edge;
- chain-link wallride visual shell with simple collision proxy;
- five-lane opposite-flow freight ceiling and container/convoy family;
- thin frontage / entrances / civic slices / parking ingress / optional bridge;
- trash/debris clusters;
- advertising/projector/hologram devices;
- scooters/e-bikes/courier props;
- near/mid/far crowd representation.

## Texture / material policy

Prefer shared atlas families:

- road/asphalt/rut/patch/crack/wet edge;
- markings/signage;
- concrete/P-6V/grime;
- panel/frontage/civic facade;
- metal/infrastructure/chain-link;
- bark/hedge/grass/dandelion;
- trash/debris;
- advertising/hologram frames;
- micromobility/shared props;
- people/crowd cards.

Variation priority:

`mesh family × rotation × vertex tint × grime/wetness params × decals/ad frames × deterministic cluster composition`

before adding unique textures.

Provisional atlas-region sizes remain experimental until profiling; ordinary drive-by props/buildings do not receive unique 4K packages by default.

## Crowd / human variation

A visually crowded corridor does not imply a full skeletal population.

- near: low-poly skeletal characters where individually readable;
- mid: simplified meshes/rigs and shared loops;
- far: cards/impostors/clusters.

Use bounded vignettes and repetition cooldowns instead of deep persistent schedules.

## Vehicle roster boundary

Level 1 lower-road baseline:

- passenger cars;
- motorcycles;
- sidecar/Ural-like motorcycles;
- the player Tatra as the ordinary taxi identity.

Ordinary heavy freight belongs to the five-lane Level 1 ceiling. Ordinary buses belong to Level 2 transit. Historical police pursuit shells are not Level 1 baseline art-budget requirements.

## PS2 lighting budget policy

Primary Level 1 active source families:

- player outer headlights;
- center steering-linked Tatra headlamp;
- nearby traffic headlights;
- tail/brake lamps;
- indicators;
- roadside/freight advertising emitters;
- rare parking/ServiceNode practical lights where physically present.

The ordinary express road does not require a dense streetlamp grid.

Important nearby sources/occluders should produce readable moving shadow response, but shadows may be low-resolution, projected, blob-like, hard-edged, reduced-frequency or distance-simplified.

A large emissive hologram may use only a small number of real dynamic emitters. Emissive complexity and real-light count are deliberately decoupled.

Production ceilings to profile under `CD-716/CD-719`:

- active real dynamic lights;
- shadow-casting lights;
- shadow method/resolution/distance classes;
- ad-emitter concurrency;
- transparent overdraw;
- GPU frame cost.

## Audio/runtime density policy

Audio follows:

`near individual → mid grouped/priority → far lane/traffic bed`

Do not attach full always-active emitters to every visible vehicle, NPC, ad or prop.

VFX/audio/lights/incidents/trash must use pooling, concurrency limits, distance fallback and cleanup.

## Presentation Zero budget gate

Before mass Level 1 content multiplication, `CD-719` must prove a representative corridor with exact build/CityCode/traffic seed and record:

- active Actors/Components;
- ISM/HISM instances and HLOD state;
- draw/submission metrics;
- texture/material memory;
- transparent overdraw;
- active real lights and shadow casters/method classes;
- active audio emitters/voices/concurrency;
- active VFX/particles;
- GPU/CPU frame time;
- FPS avg/min/1% low;
- warm/peak memory;
- 30-minute count history.

Fail if decorative systems show monotonic unowned growth after warm-up.

## Truth rule

This budget framework is CANON/SPECIFIED documentation. Numeric production ceilings become authority only after measured representative evidence. Screenshots alone do not close `CD-590` or `CD-719`.
