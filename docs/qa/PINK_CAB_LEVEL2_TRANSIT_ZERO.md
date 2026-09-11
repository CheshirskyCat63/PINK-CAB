# PINK CAB QA · Level 2 Transit Zero

Jira proof owner: `CD-710`; gate `CD-683`; delivery `CD-682`.

Detailed owners: `CD-705`, `CD-706`, `CD-707`, `CD-708`, `CD-709`; base bus traversal `CD-572`; geometry `CD-589`; magnetic numerics `CD-592`.

Authority: Confluence pages `5931030`, `8388609`, roadmap `8716289`.

## Evidence header

Every run records:

- exact commit/build id;
- generator/content/schema version;
- CityCode;
- TransitSeed;
- bus-layer profile/version;
- bus lane ids;
- track ids + train ids;
- metro phase/timing data;
- station id/module ids;
- Tatra magnetic state/contact ids;
- player speed/orientation;
- representative Level 1 camera marker ids;
- frame/memory/actor/audio/LOD counters;
- pass/fail + expected/observed;
- video/log/telemetry/profiling artifact path.

No test may silently choose an unresolved bus-lane count, bus geometry, metro timing or station dimension as production authority.

## Acceptance matrix

| ID | Area | Required proof | Authority state |
| --- | --- | --- | --- |
| T2-BUS-001 | Lane authority | runtime/test consumes versioned current bus-lane count | OPEN until `CD-705` lock |
| T2-BUS-002 | Bus cross-section | lane width/elevation/edge/entry geometry matches locked profile | OPEN |
| T2-BUS-003 | Live buses | moving buses populate the traversal layer | LOCKED DIRECTION; density OPEN |
| T2-BUS-004 | Station behavior | bus approach/passage uses explicit station rules | OPEN |
| T2-MAG-001 | Entry | Road→Transition→Partial/FullContact reaches valid bus layer | numerics OPEN `CD-592` |
| T2-MAG-002 | Partial state | bounded knife-edge/partial state works without free-flight ambiguity | PARTIAL |
| T2-MAG-003 | Transfer | valid lane/obstacle transfer uses final bus ids | lane count/geometry OPEN |
| T2-MAG-004 | Rotation | clockwise ascent + clockwise descent = one longitudinal 360° cycle | LOCKED DIRECTION |
| T2-MAG-005 | Descent | controlled descent returns to valid Road state | numerics OPEN |
| T2-MAG-006 | Abort | invalid contact/abort returns to valid road/bus state | LOCKED DIRECTION |
| T2-MAG-007 | Metro exclusion | failed bus attachment cannot transfer into metro-contact state | LOCKED DIRECTION |
| T2-METRO-001 | Track count | exactly two metro tracks/lines per side exist | LOCKED DIRECTION |
| T2-METRO-002 | Track geometry | elevation/spacing/envelope match versioned contract | OPEN `CD-706/CD-589` |
| T2-METRO-003 | Train timing | speed/headway/dwell/direction are reproducible | OPEN |
| T2-METRO-004 | Counter phase | bus/metro opposed/counter-phase rule reproduces exactly | OPEN definition |
| T2-STN-001 | Station module | deterministic repeating station/vokzal module exists | LOCKED DIRECTION; dimensions OPEN |
| T2-STN-002 | Rail continuity | station preserves both tracks per side with no broken route | LOCKED DIRECTION |
| T2-STN-003 | Crowd/service | bounded crowd/public/canteen/service slots fit station budget | budgets OPEN |
| T2-SIGHT-001 | Level 1 trains | moving metro trains are readable from approved Level 1 camera markers | LOCKED DIRECTION; sightline budget OPEN |
| T2-SIGHT-002 | Station readability | station structure is visible without becoming an opaque Level 1 wall | PARTIAL |
| T2-CLR-001 | Bus/metro | bus-layer failure/geometry cannot clip into train envelope | numeric clearance OPEN |
| T2-CLR-002 | Metro/Level 1 | rail/support geometry preserves Level 1 road clearance | numeric clearance OPEN |
| T2-REV-001 | Reverse CityCode | bus/metro/station static identity reconstructs in reverse travel | LOCKED DIRECTION |
| T2-PERF-001 | Heavy scene | ground traffic + buses + trains + station + crowds stay inside explicit budgets | OPEN `CD-709/CD-590` |
| T2-PERF-002 | Recycle/restart | transit actors/audio/contact states clean up without leaks/stuck ownership | LOCKED DIRECTION |

## Bus traversal state procedure

Target state grammar:

`Road → Transition → Partial → FullContact → Transfer/Obstacle → Descent → Road`

Every transition must have:

- valid source state;
- explicit destination state;
- rejection/failure reason;
- cleanup/reset owner;
- telemetry event;
- no double ownership by Road and bus magnetic systems.

The Level 2 envelope is tier-specific. Do not reuse Level 1 freight-ceiling force/speed values without explicit lock.

## Clockwise rotation proof

A complete approved up/down traversal must record body orientation continuously and prove:

1. ascent rotates clockwise around longitudinal travel axis;
2. descent continues clockwise rather than undoing ascent;
3. complete cycle totals one 360° longitudinal rotation within the final tolerance.

Tolerance remains numeric authority under `CD-592` until locked.

## Metro timing proof

For each TransitSeed record:

- track id;
- train id;
- travel direction;
- departure/pass timestamp;
- headway;
- station dwell where applicable;
- bus phase reference;
- derived counter-phase relationship.

The exact expected relationship remains OPEN until `CD-706` locks it.

## Level 1 sightline proof

Define representative Level 1 camera markers before verification.

Each required marker records:

- camera/world transform;
- relevant station/train ids;
- visibility/occlusion state;
- LOD level;
- screenshot/video evidence.

Fail if required trains/station cues disappear because of an optimization that violates the sightline authority.

## Heavy-scene profile

Worst representative scene includes simultaneously:

- Level 1 road traffic;
- player Tatra where applicable;
- suspended buses;
- metro trains;
- station structure;
- station crowd/vignettes;
- transit/road audio emitters.

Numeric pass/fail ceilings are owned by `CD-709`, `CD-590`, `CD-595`.

Capture at minimum:

- frame time / FPS / 1% low;
- peak memory;
- active bus/train/crowd actor counts;
- draw/instance/LOD counts where available;
- audio emitter/stream counts;
- streaming load/unload/hitch events.

## Transit Zero integrated procedure

Minimum deterministic scenario:

1. start on Level 1 with station/metro visible;
2. enter approved Tier 2 bus transition;
3. reach stable/partial/full magnetic states as prescribed;
4. interact with one moving-bus obstacle/transfer;
5. pass one station zone while metro operates below;
6. prove one complete clockwise ascent+descent cycle;
7. execute one controlled abort/failure case without metro penetration;
8. return to valid road state;
9. reverse/revisit same CityCode station corridor;
10. repeat after restart/reload;
11. capture heavy-scene profile.

## Verification rule

`CD-683` cannot be VERIFIED until exact executable evidence contains build, CityCode, TransitSeed, bus lane ids, track/train ids, station id, magnetic state telemetry, Level 1 sightline evidence and profiling artifacts.
