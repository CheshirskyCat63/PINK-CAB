# PINK CAB · BASE-100 Code Architecture

**Status:** CURRENT IMPLEMENTATION-FACING PREPRODUCTION AUTHORITY
**Jira:** `CD-746`; domain packs `CD-747..753`
**Confluence:** `11239425` — `40 · PINK CAB · BASE-100 Code Architecture & FIRST EURO 12M Contract`
**Runtime:** SPECIFIED / NOT IMPLEMENTED / NOT VERIFIED

## 1. BASE-100 definition

BASE-100 measures **code/logic/numeric/technical specification coverage for the complete 12-month FIRST EURO single-player product**.

It does not score visual art, mesh detail, materials, lighting, animation polish, decorative sound or narrative presentation except where those create an implementation-facing state/input/output/performance contract.

A programmer must be able to implement a 100% domain without inventing owner/game-design choices.

Every runtime domain is documented as:

`PURPOSE -> BUY/ENGINE/WRITE -> AUTHORITATIVE OWNER -> STATE MACHINE -> INPUTS -> OUTPUT EVENTS -> DATA MODEL -> NUMERIC CONFIG -> PERSISTENCE -> DEPENDENCIES -> FAILURE MODES -> PERFORMANCE BOUNDS -> TEST FIXTURES -> EXTENSION POINTS`

## 2. FIRST EURO 12-month scope

### Included

- PC single-player only;
- complete Level 1 + Level 2 gameplay/runtime;
- hero Tatra using FGear Vehicle Physics + Debugging Delight Vehicle Damage System;
- taxi order/fare/passenger/payment loop;
- persistent repeat clients + basic Neural history/contact/messages/repeat orders;
- CityCode generation, persistence, streaming, road graph, traffic, incidents, rules/fines;
- stable `EnforcementEvent` interface;
- automotive ServiceNodes: Parking, Garage/Tuning, Parts Shop, Repair/Service, plus **Practice Hangar where Level1 training/acceptance requires it**;
- moving refueling gameplay logic;
- vehicle health/damage/repair/load/fuel/economy;
- build/package/config/save/migration/QA/evidence architecture.

### Post-FIRST-EURO implementation

- multiplayer/coop/shared rooms/online clubs/network replication/matchmaking/server/voice/chat/anti-cheat;
- Level 3 gameplay;
- lifestyle ServiceNodes: mall/food/bar/club/social interiors;
- full Taxi Regulator/inspection gameplay;
- daily insurance purchase/toggle/premium/claim/extra-life economy.

Deferred systems retain only extension points required to avoid structural FIRST EURO rewrites. They do not lower FIRST EURO readiness.

## 3. Buy/engine-before-build doctrine

Before implementing a large subsystem, audit ready Unreal Engine functionality, Fab/Marketplace plugins and proven third-party libraries/assets.

Preferred order:

`ENGINE FEATURE -> PURCHASED/PROVEN PLUGIN -> THIN PINK CAB ADAPTER -> CUSTOM IMPLEMENTATION ONLY FOR PRODUCT-SPECIFIC GAP`

Rules:

- no duplicate competing production owners for one responsibility;
- purchased APIs are isolated behind PINK CAB interfaces/adapters;
- vendor classes must not leak throughout gameplay code;
- a solution covering most requirements plus a thin extension is preferred to a bespoke replacement framework;
- bake-off branches are allowed before technology lock;
- selected plugin/version/license/source belongs in dependency manifest and evidence records.

## 4. Code/data responsibility split

### C++

Owns interfaces, authoritative state, persistence, schema/versioning, invariants, critical state-machine orchestration, vendor adapters and performance-sensitive bounded runtime logic.

### Blueprint

Thin composition/orchestration and content hookup where iteration benefits. Blueprint does not become a hidden second persistent/gameplay owner.

### DataAssets/DataTables/config

Own tunable gameplay profiles: vehicle, surfaces, fare, passenger, traffic, city, ServiceNode, economy, rule/fine and calibration data. Tunable balance must not require recompilation.

### Actors

Actors are materialized runtime representations. Logical/persistent identities may exist without an Actor and survive Actor recycling/destruction.

### Locked runtime/release/input foundation (2026-09-11)

Shipping runtime modules are `Core / Vehicle / Taxi / World / Traffic / Economy / Persistence / Interaction`; optional `Editor / Tests` modules are non-shipping and the project must not overfragment. CI uses GitHub Actions with a self-hosted Windows Unreal runner and the same canonical build/smoke path as local development; production merge fails closed without canonical CI, with explicit local evidence allowed only as bootstrap fallback. FIRST EURO support is Windows 10/11 x64 + DX12 + Steam. Acceptance seeds are 1080p60 Low minimum and 1440p60 High recommended with a 16.67 ms frame target (~14 ms GPU / ~8 ms Game Thread seeds), subject to empirical hardware-equivalence validation. Semantic KBM rebinding ships with conflict handling and Restore Defaults while preserving the canonical physical-control model.

## 5. Cross-system architecture rules

- one authoritative owner per state;
- communicate by public interfaces, semantic commands/events and explicit data contracts;
- feature code cannot mutate another subsystem's private state;
- stable IDs for persistent entities/transactions;
- deterministic IDs for reconstructable generated world objects where appropriate;
- explicit save/config/generator versioning and migrations;
- no silent schema fallback;
- exactly-once semantics for monetary/fare/fuel/service/persistent transitions where duplicate execution changes player state;
- no unbounded Tick, world-wide scans, synchronous loading or object spawning in normal hot paths;
- repeated populations use bounded pools/caches/instancing where appropriate;
- one shared QA/evidence runner family instead of feature-specific runner ecosystems;
- build success is not gameplay verification.

## 6. Domain ownership map

### CORE — `CD-747`

Runtime foundation, module boundaries, config/data architecture, Enhanced Input routing, physical-control interaction abstraction, plugin/dependency policy, common performance/evidence rules.

### VEHICLE — `CD-748`

FGear/VDS/Vehicle Health/mass/CG/surfaces/Tatra Expression/wallride/magnet/Level2 vehicle integration. Existing vehicle authority remains `CD-729/CD-730/CD-731..740` plus vertical owners.

### TAXI — `CD-749`

FareSession, order/pickup/boarding/trip/dropoff/payment, taximeter, passenger logical/materialized identity, repeat clients and basic Neural.

### STATE — `CD-750`

EconomyService, Workday/Session, saves/checkpoints/migrations, transaction ledger and future recovery extension boundary.

### WORLD — `CD-751`

CityCode, generator/chunks, streaming, road/lane graph, traffic, incidents, navigation/rule lookup, fines and `EnforcementEvent`.

### SERVICE — `CD-752`

Shared automotive ServiceNode runtime, conditional Practice Hangar and moving-refuel orchestration over common services.

### SCOPE — `CD-753`

FIRST EURO vs post-year delivery boundary consumed by all readiness calculations.

## 7. Vehicle stack boundary

`PLAYER INPUT -> PINK CAB INPUT ADAPTER -> FGEAR -> AUTHORITATIVE VEHICLE STATE`

`COLLISION -> VDS DEFORMATION + HIT ZONE -> OPTIONAL VEHICLE HEALTH CONSEQUENCE -> FGEAR PARAMETER/STATE ADAPTER`

`AUTHORITATIVE VEHICLE STATE -> TATRA EXPRESSION/PRESENTATION`

FGear is sole hero-Tatra road-dynamics owner. VDS is sole permanent mesh-deformation owner. Expression/presentation cannot write steering, throttle, brake or tire force.

Mass/fuel/passengers affect authoritative mass and CG. Dry/wet/storm are surface/environment profile changes, not duplicate vehicle definitions. Wallride/magnet is a bounded external force/contact/state layer over FGear, never a second vehicle solver.

## 8. Taxi / Passenger / Neural boundary

One active `FareSession` owns hero-taxi fare progression.

Procedural `PassengerTemplate` and persistent `PassengerIdentity` are distinct. Persistent identity may contain stable ID, trip history, compact relationship/preferences, contact permission and repeat-order eligibility.

Only locally needed passengers materialize as Actors. Taxi/Fare publishes semantic outcomes; Neural consumes them. Neural never owns FareSession, vehicle or economy truth. Repeat orders re-enter the normal order/fare pipeline.

## 9. Economy / persistence boundary

One `EconomyService` owns balance and settlement. Fare, tip, fuel, parts, repair, parking and fines are typed transactions with stable IDs where persistence/retry matters.

Daily insurance is post-FIRST-EURO. FIRST EURO retains only a generic `RecoveryPolicy/RecoveryHook` interface so a future policy system can react to terminal vehicle/workday outcomes without rewriting Economy, Workday or Vehicle Health. **Exact FIRST EURO terminal-crash recovery remains owner question `I06` until locked.**

## 10. World / traffic boundary

World identity is `CityCode + GeneratorVersion + ContentSetVersion + persistent deltas`.

L1 and L2 share one chunk/persistence/road-graph architecture. L3 gameplay is post-year and year-one schema retains only a compatible future-tier extension boundary.

Prefer UE World Partition/PCG/engine systems plus a thin deterministic PINK CAB recipe layer; do not author a separate world-streaming engine without proven necessity.

One machine-readable road/lane graph is consumed by generation, routing, traffic and rule lookup.

Ordinary traffic is bounded logical/kinematic simulation with physical materialization only where gameplay/collision relevance requires it. Do not run hero-level FGear simulation for all distant traffic vehicles by default.

## 11. Enforcement boundary

FIRST EURO contains a stable `EnforcementEvent` producer/consumer contract and ordinary rules/fines. Full Taxi Regulator/inspection gameplay is post-FIRST-EURO and later subscribes to the same events without requiring producer rewrites.

## 12. ServiceNode boundary

FIRST EURO standard automotive ServiceNode types:

`Parking / Garage-Tuning / Parts / Repair-Service`

`Practice Hangar` is an additional **conditional FIRST EURO ServiceNode** only where Level1 training/acceptance requires it. It reuses the same transition/return/persistence framework.

Moving refueling is FIRST EURO. It owns only its refuel-session state machine and orchestrates `FuelTank`, `EconomyService`, `VehicleTelemetry` and `RoadGraph`; it does not create parallel fuel, money or vehicle truth. Fuel credit and financial settlement must be retry/save/crash safe and exactly-once.

## 13. DEADCORN engineering lessons adopted

Adopt the proven engineering discipline, not DEADCORN gameplay architecture:

- bounded state/event lifetimes;
- logical identity separate from materialized representation;
- instancing/pooling instead of Actor-per-object where behavior does not require Actors;
- exact-head/fail-closed evidence;
- one runner stack rather than parallel QA frameworks;
- strict hot-path rules;
- avoid oversized managers/controllers and mixed debug/production responsibility;
- documentation establishes specification, never runtime verification.

## 14. Readiness semantics

`START-90` means the first-year implementation can start without known structural rewrites or programmer-invented gameplay decisions. Late calibration values may remain tunable when they already have a versioned data owner, range/seed and acceptance contract.

`BASE-100` means every FIRST EURO code-facing contract is specified. Neither score claims implementation or runtime verification.
