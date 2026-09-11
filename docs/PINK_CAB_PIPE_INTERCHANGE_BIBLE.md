# PINK CAB · Right-Entry Pipe Interchange World Bible

Implementation-facing mirror of Confluence page `8421388` — **34 · PINK CAB · Right-Entry Pipe Interchange World Bible**.

Jira delivery/proof: `CD-680` / `CD-681`. Detailed owners: `CD-564`, `CD-568`, `CD-589`, `CD-696`, `CD-697`, `CD-698`, `CD-699`; QA `CD-704`.

## Authority rule

This file mirrors detailed current direction. Geometry/traffic/clearance numerics that remain OPEN must not be silently chosen as production values.

Temporary placeholders must be labeled `EXPERIMENTAL / NON-AUTHORITY` and reference `OD-INTERCHANGE-PIPE-001` or the relevant Jira owner.

## Core design

Outside view:

`BUTTERFLY / SPIDER / CLOVER / LOOP KNOT`

Driver view:

`RIGHT ENTRY → KNOWN BRANCH → TWO-LANE ONE-WAY PIPE → KNOWN LOOP/EXIT`

The node must look complex before it becomes learned route knowledge.

## Absolute entry rule

Every ordinary interchange entrance begins from the **right side** of the current carriageway.

Requirements:

- right-side deceleration/entry lane before turn-in;
- right-side acceleration/merge behavior on return;
- mirrored right-entry legality for opposite longitudinal travel;
- no arbitrary high-speed left cut across the carriageway;
- no parking/service/vignette collision inside mandatory approach/merge envelopes.

Detailed owner: `CD-696`.

## Approach traffic

Approach remains live traffic. Valid behaviors include:

- early braking;
- right-lane queues;
- late cut-ins;
- imperfect gap choice;
- through traffic continuing past the node.

Traffic friction is intentional, deadlock is not.

## Route graph requirements

A selected interchange family may expose:

- forward continuation;
- complete longitudinal reversal;
- loop/circle;
- sustained oval/circuit;
- figure-eight variant;
- tier-transfer branch;
- closed circuit supporting indefinite circulation;
- long-oval/service annex connection where selected.

Graph must work in both longitudinal directions and contain no accidental dead ends.

## Playable core

The drift-driving interior resolves into a **two-lane, one-direction pipe/tube family**.

Supported geometry can include:

- continuous banking;
- circular/oval curvature;
- spirals;
- climb/descent;
- branch transitions;
- panel-megablock penetration.

Exact tube radius/diameter, banking, grade, spiral pitch, lane widths and barriers: `OD-INTERCHANGE-PIPE-001` / `CD-589`.

Detailed geometry owner: `CD-697`.

## Handling identity

The pipe uses the ordinary Tatra arcade-sim continuum. There is no separate drift mode.

A valid route supports:

- clean grip pass;
- normal drift entry;
- sustained drift;
- curve/angle transitions;
- live-traffic overtake while sliding;
- recovery after over-rotation/contact without magical snap-to-lane.

If the pipe needs a dedicated surface/grip profile, that profile must be explicitly versioned rather than hidden in level geometry.

## Live traffic inside the pipe

The core remains a road, not an empty stunt arena.

Required experience:

`DRIFT LINE + MOVING GAP + OVERTAKE + EXIT DECISION`

Rules:

- sparse but non-zero traffic;
- two one-way lanes remain populated;
- speed/headway profile leaves readable gaps;
- AI follows actual tube graph/banking;
- no lane-cut teleporting;
- incidents are allowed only when a bypass/recovery path remains;
- collision feeds normal damage/property/economy systems;
- restart restores player/AI to valid route ownership without overlap.

Owner: `CD-699`.

## Enforcement boundary

Current canon:

- ordinary approach enforcement remains valid;
- **fixed enforcement cameras are absent from the core drift tube**;
- damage/property consequences remain active;
- no consequence-free stunt-score arena is implied.

## Building penetration

The tube may run through the volume of the panel megastructure.

Near-miss scenery can include:

- fire stairs;
- balcony frames;
- service galleries;
- structural supports;
- utility spaces;
- residential edges/windows.

Visual proximity is intentional. The legal driving envelope remains physically authoritative.

Decorative geometry cannot intrude into legal clearance unless an explicit hazard variant owns that collision.

## Balcony/fire-escape vignette kit

Initial cheap deterministic vignette families:

- smoking;
- cooking;
- arguing;
- laundry;
- TV glow;
- household repair;
- toys/children silhouettes;
- maintenance/service/utility activity.

Rules:

- no deep NPC schedule requirement;
- deterministic slot ids;
- bounded actor concurrency;
- variation masks/repetition control;
- no major pop-in directly in front of the car;
- no decorative collision intrusion into legal road envelope.

Owner: `CD-698`; budget lock: `OD-INTERCHANGE-VIGNETTE-001` / `CD-590`.

## Near-miss clearance contract

Production numeric contract must define minimum clearances for:

- balconies;
- fire stairs;
- supports/columns;
- service bridges;
- utility galleries;
- signage;
- vignette props/actors;
- recovery/restart volumes.

No team may invent these ad hoc.

## Signs and learnability

Requirements:

- right-entry warning early enough for Level 1 approach speed;
- lane/branch ownership is machine-readable;
- ordinary readable road-information grammar;
- mirrored legality in opposite direction;
- deterministic route identity by CityCode.

## Collision / recovery

The pipe remains normal road geometry:

- traffic/support collisions possible;
- collision consequences hand off to normal systems;
- no respawn into overlapping traffic;
- restart must restore valid lane/link ownership;
- no collision seams at building-penetration streaming boundaries;
- no impossible floating/attached state after abort/restart.

## CityCode / generator ids

Persist/reconstruct at minimum:

- approach module id;
- decel/merge zone ids;
- route graph template/version;
- node/link ids;
- pipe segment ids;
- branch/exit ids;
- building-penetration module ids;
- vignette slot ids;
- traffic profile id;
- recovery volume ids;
- tier-transfer link ids where present.

Reverse longitudinal travel using the same CityCode/content version must reconstruct the same graph family with mirrored legality.

## Streaming / occlusion

High-speed building penetration must budget simultaneous visibility for:

- tube road shell;
- traffic;
- facade/interior fragments;
- balcony/fire-escape props;
- vignette actors;
- signs/lights;
- neighboring structural geometry.

Aggressive LOD/occlusion is allowed, but major route/near-miss geometry cannot pop after it enters the driving decision envelope.

Owners: `CD-589`, `CD-590`.

## OPEN locks

`OD-INTERCHANGE-PIPE-001`:

- approach/deceleration lengths;
- merge throat widths;
- graph templates/node/link limits;
- tube radius/diameter;
- banking/grade/spiral pitch;
- two lane widths;
- barrier/road clearances;
- building/near-miss envelope;
- traffic density/speed/headway;
- drift-safe recovery volumes;
- streaming/occlusion distances.

`OD-INTERCHANGE-VIGNETTE-001`:

- final vignette pool;
- actor concurrency;
- repetition policy;
- hazard-vs-decoration boundary;
- high-speed LOD/pop-in thresholds.

## Interchange Zero gate

QA mirror: `docs/qa/PINK_CAB_PIPE_INTERCHANGE_ZERO.md` / Jira `CD-704`.

Proof must include:

1. right entry in both longitudinal directions;
2. valid decel/merge and signs;
3. forward route;
4. reversal;
5. loop;
6. sustained oval/circuit;
7. figure-eight where selected;
8. tier transfer where selected;
9. two-lane one-way pipe continuity;
10. building penetration + clearance report;
11. clean grip pass;
12. sustained drift pass;
13. live traffic overtake/gap case;
14. collision/recovery/restart;
15. reverse CityCode graph identity/checksum.
