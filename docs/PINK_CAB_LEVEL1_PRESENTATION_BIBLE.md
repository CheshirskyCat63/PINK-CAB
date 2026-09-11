# PINK CAB · Level 1 Straight Corridor Presentation Bible

**Status:** CURRENT IMPLEMENTATION-FACING PRESENTATION AUTHORITY
**Confluence:** page `8323081`
**World/gameplay authority:** `docs/PINK_CAB_LEVEL1_WORLD_BIBLE.md` / Confluence `8388630`
**Jira:** asset `CD-714`, material `CD-715`, lighting `CD-716`, audio `CD-717`, VFX/density `CD-718`, proof `CD-719`

This file owns how the direct Level 1 corridor is **built, rendered, lit, sounded and profiled**. It does not own world-geometry or gameplay numerics.

## 1. North star

PINK CAB Level 1 should read like an unusually art-directed 2003–2004 game with modern production discipline:

- low-poly reusable geometry;
- very dense visual information per square meter;
- cold/permanent-night city base with strong saturated practical lights;
- deliberately simple materials;
- PS2-style hard light transitions and coarse moving shadows;
- strong silhouette/readability from the moving cockpit;
- aggressively bounded runtime systems.

The player sees a huge city machine. Runtime should see a compact set of reusable systems.

## 2. Density doctrine

Target:

`HIGH PERCEIVED OBJECTS/m² / LOW ACTIVE ENTITY COUNT`

Representation ladder:

### Near

Actual low-poly mesh/actor only when collision, interaction, individual motion or close light/shadow response matters.

### Mid

Prefer ISM/HISM, precombined clusters, simplified meshes/rigs, grouped audio/light emitters and low-frequency animation.

### Far

Prefer HLOD/merged proxy, impostor, billboard/card and aggregate traffic/audio/light representation.

Ordinary trash, grass, dandelions, frontage clutter, balcony clutter and decorative props get **no per-object Tick**.

## 3. Reusable geometry families

### Road / rule geometry

- five-lane express straight;
- gentle left/right curves;
- gradual rise/descent;
- right-side deceleration lane and exit throat;
- right-side acceleration/merge lane;
- two-lane local/access road;
- legal parking edge;
- narrow underground parking ingress;
- curb/gutter/drain/seams;
- sign/camera sockets and exclusion zones.

Exact meter values remain `CD-589` authority.

### Road-surface visual kit

- worn asphalt;
- rut strips;
- patch families;
- crack families;
- standing water/puddle variants where used;
- dirty edges / rough parking-ramp variants;
- clean/readable lane markings over degraded surface.

Gameplay force data may use cheap proxy volumes independent of visual decals/meshes.

### Median / giant-poplar kit

- tightly controlled giant leafless poplar silhouette;
- very small variant count preserving near-identical canonical height/read;
- rotation/vertex/material variation before silhouette randomization;
- clipped hedge;
- bright artificial grass cards/clusters;
- dandelion clusters;
- dirt/maintenance patches;
- deterministic jump-window modules;
- P-6V-like barrier family.

### Chain-link wallride kit

- repeating visual mesh panels;
- support ribs/arches;
- lower/upper connectors;
- patch/repair variants;
- contact/spark/deformation-fake sockets;
- simple gameplay collision/traversal proxy separate from decorative wire.

No cloth/network simulation is required.

### Freight ceiling kit

- five aligned ceiling lane shells;
- magnetic receiving strips;
- seams/joints/supports;
- non-player dock apertures;
- freight/container sockets;
- roof/convoy-ad sockets;
- wallride transition geometry.

Upper freight direction remains always opposite lower Level 1 road flow.

### Frontage / city-life strip

- entrance/door/canopy pieces;
- benches and entrance courts;
- playground/civic/school/kindergarten slices;
- small fences/service doors/utility cabinets;
- micromobility/courier strip pieces;
- parking-ramp retaining walls;
- optional low concrete pedestrian bridge;
- panel-megablock facade variants;
- crowd/vignette support sockets.

The frontage is a theatrical slice, not a walkable district.

## 4. Prop families

### Road furniture

Signs, cameras/masts, P-6V connectors, reflectors, bollards, drains, cabinets, supports only where needed.

### Trash/debris

Reusable low-poly/clustered bags, bottles, cans, cartons, cups, paper/cardboard, small boxes, household clusters, bins and larger parking clusters.

Most visible trash is instanced/precombined. Gameplay transition/drag logic lives in cheap volumes/state.

### Advertising devices

Prism/projector emitters, mounts/brackets, utility boxes, crossed-card/card-stack/billboard hologram geometry, phone/device ad family, sequential-ad sockets and freight-convoy ad sockets.

### Micromobility/courier

Small scooter/e-bike silhouette families, courier bag/box variants, parked clusters and simple near/mid/far rider representation.

## 5. Crowd strategy

Near characters: low-poly skeletal actors only where individually readable/useful.
Mid: simplified rigs/meshes/shared loops.
Far: cards/impostors/clusters.

Useful vignettes include courier stops, entrance conversations, benches, playground, pedestrian bridge crowd, maintenance/service and school/civic frontage fragments.

No deep persistent life schedules are required.

## 6. Texture / atlas doctrine

Prefer a compact reusable family:

- road/asphalt/rut/patch/crack/wet edge;
- markings/arrows/rule graphics;
- black/orange/white signs;
- concrete/P-6V/grime/runoff;
- panel/frontage/civic facade;
- metal/infrastructure/chain-link mask;
- bark/hedge/grass/dandelion;
- trash/debris;
- advertising/hologram frames;
- micromobility/shared props;
- people/clothing/crowd cards.

Prefer:

`mesh family × rotation × vertex tint × grime/wetness params × decal/ad frame × deterministic cluster composition`

before creating a new unique material package.

Provisional authoring bands remain `EXPERIMENTAL / NON-AUTHORITY` until profiling:

- tiny prop regions roughly 64–256 px;
- ordinary module regions roughly 256–512 px;
- larger repeating regions roughly 512–1024 px;
- multi-asset atlases typically 1K–2K.

Ordinary drive-by props/buildings do not get unique 4K packages by default.

## 7. Material doctrine

Keep master-material count small. Core families:

- road;
- opaque architecture/concrete;
- metal/infrastructure;
- vegetation/cards;
- vehicle;
- signage/emissive/hologram;
- people/props;
- glass only where needed;
- VFX/decal.

Simple roughness/specular is preferred. Normal maps are optional and must earn their cost at driving distance/speed.

PS2-style tools may include strong vertex color, crude highlight bands, hard material transitions, animated emissive sheets and low-frequency grime/wetness masks.

## 8. Sequential advertising system

Adjacent roadside emitters may form a frame sequence:

`AD[0] = n`
`AD[1] = n+1`
`AD[2] = n+2`

At speed the ad appears to animate along the road.

Data should expose `AdSequenceId`, frame/index, offset rule, material/emissive profile, light-emitter profile, audio profile and distance fallback.

Freight convoys may share one long holographic composition across multiple 3–5-unit containers instead of spawning unrelated effects per carriage.

## 9. PS2 lighting doctrine

### North star

Lighting is a moving graphic/compositional system, not a photometric simulation.

Valid scene rhythm:

`DEEP DARK → SATURATED AD WASH → BRAKE-LIGHT RED → ONCOMING WHITE → GIANT MOVING SHADOW → DARK`

### Primary active source families

- Tatra two outer primary headlights;
- Tatra center steering-linked directional lamp;
- nearby traffic headlights;
- tail/brake lights;
- indicators;
- roadside ad/hologram emitters;
- freight-convoy ad emitters;
- occasional entrance/parking/ServiceNode practical lights.

The normal Level 1 express road does not require a dense modern streetlamp grid.

### Dynamic behavior

Lights move/change with their source. Brake/indicator states pulse visibly; ad color changes may recolor local composition; center lamp follows steering.

### Shadow rule

Important nearby lights/occluders should create readable moving shadow response where useful. Shadows may be:

- low-resolution;
- hard-edged;
- coarse;
- projected;
- blob-like;
- distance-simplified;
- reduced-update-rate.

The target is readable occlusion, not physically correct penumbra.

### Emissive proxy rule

A visually huge hologram may use only a few cheap real dynamic emitters.

`BRIGHT PIXEL != REAL LIGHT`

### Explicit non-requirements

Level 1 does **not** require:

- ray tracing;
- path tracing;
- expensive realtime GI;
- premium-only area/volumetric-light stacks;
- cinema-grade photometry;
- 4090-class hardware assumptions.

Use the cheapest real dynamic-light/shadow method that produces the intended image.

## 10. Tatra center lamp presentation

Locked identity:

- physically centered on the front/longitudinal axis;
- rotates with steering/wheel direction;
- reads somewhat farther than outer pair;
- remains a visual/vehicle mechanic, not auto-navigation.

Exact yaw/transfer/cone/range remain `CD-561` locks.

## 11. Audio kit

### Player Tatra

Engine RPM/load, intake/exhaust, gear/clutch/driveline, suspension/body rattle, tyre roll/scrub, rut/patch/rough-road response, debris impact, wallride contact/slide, magnetic attach/stable/residual/detach, freight near-pass, indicator/cockpit switches and optional center-lamp servo cue.

### Ground traffic

Passenger-car size families, motorcycle/sidecar families, acceleration/deceleration, pass-by/Doppler, braking/horns/incidents.

Level 1 lower baseline excludes ordinary bus/heavy-freight audio actors.

### Road/freight/frontage

Asphalt/rut/patch/debris/chain-link; freight propulsion/magnetic hum/container resonance/coupling/lane servo/dock/convoy/ad hum; wind around poplars/megablock, trash flutter, residential/civic beds, entrance/service sounds, bridge crowd, micromobility whine, parking-ramp concrete/drainage/ventilation.

### Incident theatre

Tire blowout, brake/horn cluster, metal/plastic scrape/impact, debris burst and settling/hazard detail where audible.

## 12. Audio optimization

Canonical hierarchy:

`near important actor → individual emitter`
`mid actors → grouped/priority emitters`
`far traffic → lane/traffic bed`

Use pooled emitters, concurrency limits, nearest-N prioritization, grouped crowds/ads and distance fallback.

## 13. Required VFX

- tyre smoke/dust / rough-road micro response;
- trash/debris burst and cheap paper/bag/cardboard flight;
- windshield liquid/trash occlusion;
- hologram flicker / sequence transition;
- magnetic attach/stable/detach cues;
- chain-link contact/deformation fake/sparks;
- freight magnetic/contact cues;
- collision sparks / incident smoke/debris;
- bounded parking/utility effects.

All transient effects have explicit lifetime/pooling/cleanup.

## 14. Runtime metrics

A representative capture records:

### Geometry / entity density

- active Actors;
- active Components;
- ISM/HISM instances;
- HLOD/proxy state;
- visible traffic and near/mid/far population representation.

### Rendering

- draw/submission metrics;
- GPU and CPU/game-thread frame time;
- FPS avg/min/1% low;
- transparent overdraw;
- material instance count;
- texture/material memory;
- active real lights;
- shadow-casting lights and method/resolution class.

### VFX/audio

- active particle/system instances;
- active audio emitters;
- voices/concurrency;
- audio memory;
- pool counts/recycle counts.

### Streaming

- loaded/visible chunks;
- HLOD transitions;
- warm/peak memory;
- actor/emitter/particle/light/chunk count history over 30-minute deterministic driving.

## 15. No-monotonic-growth rule

Extended driving must not show unbounded growth of:

- trash actors;
- particles;
- emitters;
- ads;
- crowds;
- transient lights;
- incident props;
- streamed chunks.

An infinite city implemented as an infinite leak fails acceptance.

## 16. Presentation Zero

Proof mirror: `docs/qa/PINK_CAB_LEVEL1_PRESENTATION_ZERO.md`.

`CD-719` must prove:

1. Level 1 road/right-entry grammar reads at speed;
2. giant-poplar/hedge/P-6V identity is immediate;
3. dense frontage/micromobility/trash/ads do not reveal one-Actor-per-object construction;
4. PS2 low-poly/atlas style survives LOD/HLOD transitions;
5. Tatra outer + center directional lamp behavior reads;
6. traffic headlights/tail/brake/indicators dynamically shape the image;
7. ads create strong scene color with bounded real emitters;
8. moving cheap shadows remain readable;
9. audio hierarchy works without emitter explosion;
10. debris/hologram/magnetic/wallride/incident VFX clean up;
11. no premium-only RT/GI path is required;
12. measured frame/memory/entity/emitter counts stay bounded;
13. 30-minute drive shows no monotonic presentation leak.

## 17. OPEN production numerics

Do not guess hard limits for triangles/LODs, atlas pages, draw calls/material instances, HLOD distances/counts, real lights/shadows, ad concurrency, audio voices, particles, crowd counts, transparent overdraw, memory or reference hardware.

These become production authority only after representative `CD-719` evidence.
