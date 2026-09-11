# PINK CAB · Deterministic World Streaming Contract

**Status:** CURRENT FIRST-EURO IMPLEMENTATION-FACING SPEC / NUMERICS PARTIAL
**BASE-100 WORLD:** `CD-751`
**Persistence:** `CD-560`; geometry/streaming numerics `CD-589`
**Confluence:** CityCode `5931050`, world `6258704`, BASE-100 `11239425`

## Principle

PINK CAB presents an effectively endless longitudinal city while retaining a **persistent personal CityCode identity**. The runtime does not keep the entire city materialized. Static world identity is reconstructed from deterministic data; only a bounded useful envelope is loaded/materialized.

World identity direction:

`CityCode + GeneratorVersion + ContentSetVersion + persistent deltas`

FIRST EURO fully supports Level1 + Level2. Level3 gameplay is POST-FIRST-EURO but the tier/route schema remains extensible.

## Buy/engine-before-build

Prefer Unreal World Partition, PCG and other engine streaming/content systems where they satisfy the requirement. PINK CAB code should provide a thin deterministic recipe/identity/validation layer rather than a second bespoke streaming engine.

Any third-party world tool must sit behind explicit recipe/placement interfaces and may not become the sole opaque persistence authority.

## Logical vs materialized state

A world module/chunk may exist as deterministic logical identity without all Actors loaded. Runtime materialization should contain only near/visible/interaction-relevant content. Distant traffic/passengers/incidents remain logical or absent according to their own bounded contracts.

Persistent save stores seed/version + meaningful deltas; it does not serialize the entire generated world or every transient Actor.

## WorldChunk / recipe contract

Exact chunk classes/dimensions remain owner Pack J / `CD-589`, but each reconstructable unit needs stable implementation-facing identity such as:

- deterministic coordinate / `ChunkId` / `ModuleId`;
- CityCode/generator/content version;
- Level/Tier (`L1`, `L2`, future `L3`, transition);
- entry/exit road/lane graph anchors;
- legal successor/predecessor constraints;
- frontage/ServiceNode placement eligibility;
- surface/rule/traffic profile IDs;
- vertical/interchange hooks where applicable;
- deterministic derived seeds;
- collision/visibility/streaming bounds;
- persistent-delta lookup key;
- cleanup/materialization budget.

Raw asset filenames are not the public gameplay identity.

## Road/lane graph relationship

Current BASE-100 proposed direction is one machine-readable road/lane graph shared by generation, navigation, traffic and rule lookup. Static graph identity must reconstruct for the same CityCode/version and survive forward→reverse traversal.

Unrestricted random adjacency is forbidden. Selection must satisfy geometry, route connectivity, tier/transition legality, visibility/sight-distance, traffic/service exclusions, anti-repetition and performance constraints before seeded variation is applied.

## Persistence / deltas

Persist only state that must outlive rematerialization, such as:

- owned Tatra / VehicleBuild / Vehicle Health;
- active workday/fare/passenger/repeat-client/Neural state;
- economy/inventory transactions;
- ServiceNode state that affects return/ownership;
- significant persistent world deltas/incident state where explicitly adopted;
- version/migration identifiers.

Temporary traffic/presentation debris/ordinary offscreen actors do not become persistent merely because they were once visible.

## Derived seeds

Prefer separate deterministic streams for road topology, static decoration/content selection, traffic, passengers and incidents where doing so prevents harmless content iteration from invalidating core route identity. Exact seed derivation/version policy remains owner Pack J08/J09.

## Streaming lifecycle

Exact implementation may use World Partition cells, PCG-generated content, pooled logical recipes or equivalent engine-backed structures. Conceptually:

`KNOWN LOGICAL IDENTITY → REQUESTED → MATERIALIZED → ACTIVE → RELEASE-ELIGIBLE → UNLOADED/RECYCLED`

Unloading removes expensive runtime representation but does not delete deterministic or explicitly persistent identity.

## Hot-path rules

- no global Actor scans to discover streamed identity;
- no uncontrolled SpawnActor/NewObject storm in normal Tick;
- no synchronous asset loads in normal hot path;
- bounded ahead/behind/materialization windows;
- bounded traffic/passenger/incident pools;
- cleanup cannot orphan collision, contact, fare, moving-refuel or ServiceNode ownership.

## OPEN numeric owners

The following are still BASE-100 code-facing owner/calibration work:

- chunk/recipe granularity;
- lookahead/lookbehind distances/times;
- max loaded cells/recipes and memory budget;
- high-speed sight/generation latency margin;
- preload/async timeout behavior;
- cache/pool sizes;
- safe return/resume spawn rules;
- deterministic migration/reconstruction policy across generator versions;
- hitch/frame-time acceptance threshold.

Old DEADRACE starting values such as `500–1000 m ahead`, `100–200 m behind` and `60–150 m chunks` are **historical experimental references only**, not PINK CAB production locks.

## Debug / evidence

Development evidence should expose current CityCode/generator/content/schema versions; current/adjacent logical module IDs; requested/materialized/released counts; road graph anchors; async generation/load latency; active/pool Actor counts; memory/frame time; traffic/passenger/incident counts; and reconstruction checksum/topology hash where practical.

## Acceptance

At minimum prove:

1. same CityCode/version reconstructs the same static route/module identity;
2. forward then reverse traversal revisits the same known static city;
3. no dead-end/missing road from a valid generated route;
4. materialized world/Actor/memory counts remain bounded during a long run;
5. old cells unload/recycle predictably with no visible/physical seam failure;
6. save/reload preserves persistent deltas and does not duplicate transient actors/transactions;
7. L1↔L2 identity and transition remains deterministic;
8. future L3 fields do not create a first-year L3 gameplay dependency.

Documentation establishes SPECIFIED only. Runtime remains **NOT IMPLEMENTED / NOT VERIFIED**.
