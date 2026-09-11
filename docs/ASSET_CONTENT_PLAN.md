# PINK CAB · Level 1 Asset & Content Production Plan

**Status:** CURRENT IMPLEMENTATION-FACING PLAN
**Confluence:** page `6717461` Content Budgets; page `8323081` Level 1 Presentation Bible
**Jira:** `CD-590`, `CD-595`, `CD-714..719`

> Historical DEADRACE launcher/police/pursuit/chase-stream requirements are **LEGACY / SUPERSEDED**. They are not PINK CAB baseline content requirements.

## Principle

The city must feel larger than the asset folder.

Use **families + deterministic variation**. Prefer silhouette, palette, modular parts, signage, social context, animation role, wetness/grime, emissive state, instancing and HLOD before adding new geometry.

Ask of every asset:

1. Can the player notice/use it from the first-person Tatra cockpit at Level 1 driving speed?
2. Can it be reused many times without exposing the trick?
3. Does it improve route readability, Level 1 identity, tactile vehicle feedback or the feeling of a living city?
4. Can it be represented more cheaply at mid/far distance?

If not, it is not priority Level 1 production work.

## Hero vehicle

One neon-pink early/Gen-1 Tatra 603-family hero vehicle.

Required exterior/cabin identity includes:

- three-lamp front;
- center steering-linked directional lamp with somewhat farther-reading beam;
- intentionally stretched rear/tail region per final owner sheet;
- tactile cockpit controls;
- rear/service packaging for the one 17-year-old daughter → conductor/service → later operator role;
- no third permanent conductor NPC.

Detailed canon: `docs/PINK_CAB_TATRA_DAUGHTER_CONDUCTOR_CANON.md`.

## Level 1 ground traffic

Baseline lower-road families:

- passenger cars across a compact small/medium/large silhouette range;
- motorcycles;
- sidecar/Ural-like motorcycles.

The player Tatra is the ordinary taxi identity in baseline flow.

**Do not** budget ordinary buses/heavy freight as lower Level 1 traffic:

- heavy freight belongs to the five-lane opposite-flow ceiling;
- buses belong to Level 2 transit.

Final family counts/weights remain OPEN under `CD-692/CD-590`.

## Level 1 freight ceiling

Required reusable family:

- single automated freight/container unit;
- linked 3–5-unit convoy variants;
- coupling/priority/lane-change visual states;
- dock aperture interaction visuals;
- roof/convoy hologram sockets;
- near/mid/far representation and instancing strategy.

Direction is always opposite lower Level 1 traffic.

## Environment production kit

### Road / local / parking

- five-lane express straight/curve/rise/descent modules;
- right deceleration/exit and acceleration/merge modules;
- two-lane local/access road;
- legal parking edge;
- narrow underground parking ingress;
- curb/gutter/drain/seams;
- sign/camera sockets and placement exclusions.

### Road surfaces

- baseline worn asphalt;
- rut strips;
- patch families;
- crack families;
- standing-water variants where selected;
- rough parking/local-road treatment;
- clean readable markings.

### Giant-poplar median

- tightly controlled giant leafless poplar silhouette family;
- clipped hedge;
- bright artificial grass;
- dandelion clusters;
- dirt/maintenance patches;
- deterministic jump-window modules;
- P-6V-like barrier/connectors.

### Chain-link wallride

- repeating visual panels;
- support ribs/arches;
- lower/upper connectors;
- patch/repair variants;
- contact/spark/deformation-fake sockets;
- simple gameplay collision shell separate from visual wire.

### Freight ceiling

- five lane shells;
- receiving strips;
- seams/joints/supports;
- dock apertures;
- freight and advertising sockets.

### Frontage / human-life slice

- entrance/door/canopy modules;
- benches and entrance courts;
- playground/civic/school/kindergarten fragments;
- service/utility doors and cabinets;
- micromobility/courier strip;
- parking retaining walls;
- optional low concrete pedestrian bridge;
- panel-megablock facade variants;
- bounded vignette sockets.

Frontage remains thin and theatrical, not a walkable district.

## Trash / debris production

Reusable low-poly/cluster families:

- closed/open bags;
- bottles/cans/cartons/cups;
- paper/cardboard;
- small boxes;
- household refuse clusters;
- small/large bins and parking clusters.

Most visible trash uses ISM/HISM/precombined clusters. Gameplay debris/transition behavior uses cheap volumes/state, not hundreds of rigid bodies.

## Advertising production

Required:

- prism/projector device;
- barrier/wall mounts;
- brackets/supports;
- small utility box;
- cheap crossed-card/card-stack/billboard hologram geometry;
- phone/device ad family;
- sequential-ad sockets;
- freight-convoy composition sockets.

Approved everyday content direction includes loans/credit, pawnshops, discounted expensive devices/phones, food, entertainment and ordinary services.

Sequential chains use adjacent frames/offsets so motion appears to travel along the road at speed.

## Micromobility / courier production

Use a very small reusable family:

- scooter silhouettes;
- e-bike silhouettes;
- courier bags/boxes;
- parked clusters;
- simple stands;
- near rider mesh + simplified mid/far representation.

## Human / crowd strategy

Do not create dozens of unique full-body citizens.

Representation:

- near: low-poly skeletal people only where individually readable;
- mid: simplified shared meshes/loops;
- far: billboards/impostors/cards/clusters.

Useful loops/vignettes:

- idle/walk/talk/sit;
- courier approach/deliver/leave;
- entrance conversation;
- bench scene;
- bridge crowd;
- playground motion;
- maintenance/service work;
- micromobility stop/start.

A corridor may look heavily populated without maintaining deep schedules or one full NPC per body.

## Texture / material plan

Prefer a small reusable master-material family and compact atlases.

Atlas families:

- road/asphalt/rut/patch/crack/wet edge;
- markings/signage;
- concrete/P-6V/grime/runoff;
- panel/frontage/civic facade;
- metal/infrastructure/chain-link;
- bark/hedge/grass/dandelion;
- trash/debris;
- advertising/hologram frames;
- shared props/micromobility;
- people/clothing/crowd cards.

Variation priority:

`mesh × rotation × vertex tint × grime/wetness params × decal/ad frame × deterministic cluster composition`

Provisional authoring bands remain experimental until `CD-719` profiling. Ordinary drive-by assets do not receive unique 4K packages by default.

## PS2 lighting production

Primary active source families:

- Tatra outer headlights;
- Tatra center steering-linked lamp;
- nearby traffic headlights;
- tail/brake lamps;
- indicators;
- roadside/freight advertising emitters;
- rare entrance/parking/ServiceNode practical lights.

Lighting is art-directed and dynamic, not physically realistic.

Allowed shadow strategies include coarse low-resolution dynamic shadows, projected/blob-like shadows, hard-edged shadows, reduced update frequency and distance simplification.

A huge hologram may use only a small number of real lights.

Do not require RT/path tracing/expensive realtime GI/premium-only lighting paths.

## Audio production plan

### Tatra

Engine/load, intake/exhaust, driveline, clutch/gear, suspension/body, tyre roll/scrub, rut/patch/rough-road, debris, wallride, magnetic attach/stable/residual/detach, freight near-pass, indicators/cockpit switches and optional center-lamp servo.

### Ground traffic

Passenger-car families, motorcycle/sidecar, acceleration/deceleration, pass-by, braking/horns and incident reactions.

### Freight / environment

Freight propulsion/hum/container/coupling/lane servo/dock/convoy/ad emitter; wind around poplars/megablock, trash flutter, frontage/crowd clusters, micromobility, entrance/service and parking-ramp concrete/drainage/ventilation.

### Audio optimization

Use:

`near individual → mid grouped/priority → far lane/traffic bed`

plus pooled emitters, concurrency limits, nearest-N priority and distance fallback.

Final source/event/voice/memory counts remain `CD-595/CD-717/CD-719` locks.

## VFX production plan

Reusable pooled families:

- tyre smoke/dust and rough-road micro response;
- debris/trash burst and cheap flying paper/bag/cardboard;
- windshield liquid/trash occlusion;
- hologram flicker/sequence transition;
- magnetic attach/stable/detach;
- chain-link contact/deformation fake/sparks;
- freight magnetic/contact cues;
- collision sparks / incident smoke/debris;
- bounded parking/utility effects.

Every transient effect has explicit lifetime/pool/cleanup ownership.

## ServiceNode production boundary

Bounded city destinations consume `docs/PINK_CAB_SERVICENODE_TAXONOMY.md`.

Do not create bespoke save/vehicle/session architectures for garage, parts shop, mall, diner or club themes.

Practice Hangar is the early Level 1 node and reuses the common persistence/return contract without requiring full retail/social implementation.

## Presentation Zero gate

Before mass Level 1 asset multiplication, `CD-719` must prove one representative corridor from the reusable kit.

Required evidence includes:

- exact build/CityCode/traffic seed;
- active Actors/Components;
- ISM/HISM and HLOD state;
- draw/submission metrics and transparent overdraw;
- texture/material memory;
- active real lights and shadow casters/method classes;
- active audio emitters/voices;
- active VFX/particles;
- GPU/CPU frame time and FPS avg/min/1% low;
- warm/peak memory;
- 30-minute no-monotonic-growth result.

No uncontrolled asset expansion before this proof supplies measured budgets.

## Repetition rules

- do not cluster identical vehicle body/material combinations;
- no adjacent repeat of large vignettes;
- ad/storefront families use repeat cooldowns;
- major modules use longer cooldowns;
- district/frontage material states persist for coherent stretches;
- poplar identity stays intentionally repetitive rather than randomized into a forest.

## Truth rule

This file is SPECIFIED documentation, not runtime proof. Exact production budgets remain OPEN until measured evidence converts them into locked values.
