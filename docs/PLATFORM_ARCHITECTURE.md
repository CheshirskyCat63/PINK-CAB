# PINK CAB · Platform Architecture & Ownership Boundaries

**Status:** CURRENT FIRST-EURO IMPLEMENTATION-FACING ARCHITECTURE
**BASE-100:** `CD-746/CD-747`; Confluence `11239425`
**Scope:** `CD-753`
**Vehicle stack:** `CD-730`

## Principle

Keep authoritative state owners small, versioned and testable. Do not build a monolithic `GameManager`, and do not scatter third-party plugin types across unrelated gameplay code.

Preferred dependency form:

`ENGINE / PURCHASED PLUGIN → PINK CAB ADAPTER → DOMAIN STATE/DATA → ORCHESTRATION → PRESENTATION`

One production owner per responsibility. Runtime Actors represent materialized world state; they are not automatically persistent truth.

## FIRST EURO domain owners

### Core/runtime

Owns project startup, feature flags, configuration registry, plugin/version manifest, logging/build identity and shared event/interface conventions. Prefer Unreal Subsystems over ad-hoc global singleton managers.

### VehicleRuntime

FGear is the sole physical road-dynamics solver for the hero Tatra. PINK CAB exposes normalized `VehicleTelemetry` and command adapters; unrelated systems do not depend directly on vendor classes.

### VehicleBuild

Persistent installed parts/configuration/tuning schema. Garage changes this through versioned transactions/adapters; no arbitrary parallel stat system.

### VehicleHealth

Functional damage/wear/thermal state. VDS owns permanent body deformation. Authored hit-zone mappings may change Vehicle Health and approved FGear parameters; cosmetic dents need not affect function.

### Interaction/Input

Enhanced Input feeds one PINK CAB input/interaction layer. Current grammar is `1–4 START → LMB ATTENTION → RMB GO`, Space gaze, mouse steering. Semantic commands/events are emitted to target systems; physical control meshes do not own unrelated state.

### FareSession

Owns one fare lifecycle: order/pickup/boarding/meter/trip/drop-off/payment/failure. Exact one-active-fare rule remains owner-pack confirmation where marked. Meter/payment events are duplicate-sensitive transactions.

### PassengerIdentity

Persistent passenger identity/history/preferences/mass/contact state independent of whether a passenger Actor is currently materialized.

### PassengerPresentation

Temporary physical body/voice/cosmetic representation derived from identity/template/context. It may be pooled/recycled without deleting persistent identity.

### Neural

FIRST EURO owns a bounded repeat-client/contact/history/message layer. It consumes fare outcome events and may create repeat orders through the normal order/fare pipeline. It does not own FareSession.

### EconomyService

Sole monetary balance/settlement owner. Fares, tips, fines, fuel, parking, parts and repair use typed stable transaction IDs and exactly-once semantics.

### Workday/Session

Owns current shift/day lifecycle, pause/sleep/quit/recovery transitions and safe orchestration boundaries. It does not own vehicle physics or monetary data directly.

### Persistence

Owns save schema/version/migration/checkpoint rules. It serializes logical domain state/deltas; it does not dump every transient Actor.

### CityDefinition

Owns `CityCode + GeneratorVersion + ContentSetVersion` and persistent world-delta identity.

### WorldChunk/Recipe

Owns one reconstructable local world recipe/coordinate and legal adjacency constraints. Prefer UE World Partition/PCG/engine systems with thin PINK CAB deterministic recipe logic rather than a second world engine.

### RoadGraph

One machine-readable road/lane graph is the intended common source for generation/routing/traffic/rule lookup. Final owner acceptance is tracked in Technical Owner Pack K01.

### TrafficRuntime

Owns bounded logical traffic population/state. Expensive collision/Actor representation is materialized only where needed. Distant/ordinary traffic does not require FGear per vehicle.

### RoadRule / EnforcementEvent

RoadRule defines machine-readable legality. `EnforcementEvent` records an authoritative observation/fact consumed by first-year fines/reputation and later by the POST-FIRST-EURO Taxi Regulator.

### ServiceNodeRuntime

One shared bounded transition system for FIRST EURO Parking, Garage/Tuning, Parts and Repair/Service plus Practice Hangar where L1 requires it. It preserves exactly one Tatra/CityCode/passenger/fare/economy/inventory/VehicleBuild/VehicleHealth identity.

### MovingRefuelSession

Separate live-road FIRST EURO service. Owns only refuel-session state; reuses FuelTank, EconomyService, normalized VehicleTelemetry, RoadGraph and common persistence/evidence.

### VerticalContact

Bounded Level1/Level2 traversal/contact state layered over FGear. It does not become a replacement vehicle solver. Level1 residual timeout uses the locked linear load rule.

### OperatorTask

Daughter/operator task information only. It never owns steering/braking/throttle/gap/route authority.

### Inventory / PartDefinition

Inventory owns item instances/quantities/version. PartDefinition owns compatibility and permitted VehicleBuild/vehicle-profile modifications. Economy remains separate.

## POST-FIRST-EURO extension owners

The following are future architecture, **not FIRST EURO runtime dependencies**:

- account-backed multiplayer identity/presence;
- SocialRoom/CommonLobby;
- network VehicleSession/prediction/replication;
- RaceDefinition/RaceSession;
- Level3 gameplay owner;
- Taxi Regulator proper;
- daily insurance policy/premium/claim state.

FIRST EURO preserves only generic extension contracts where needed: stable IDs/versioning, `EnforcementEvent`, `RecoveryPolicy/RecoveryHook`, extensible ServiceNode/tier schema and clean adapter boundaries. Do not implement speculative multiplayer code merely for future-proofing.

## Repository target layout

Exact Unreal module names remain Technical Owner Pack A03, but the intended separation is approximately:

```text
PINK-CAB/
  Config/
  Content/
    Game/
      Vehicle/
      Cockpit/
      City/
      Traffic/
      Passenger/
      Neural/
      Services/
      UI/
      Audio/
    ThirdParty/
  Source/
    PinkCabCore/
    PinkCabVehicle/
    PinkCabTaxi/
    PinkCabWorld/
    PinkCabTraffic/
    PinkCabEconomy/
    PinkCabPersistence/
    PinkCabInteraction/
  Data/
    TestSeeds/
    Schemas/
    ContentVersions/
  docs/
  scripts/
  .github/
```

Do not treat this naming sketch as locked module count until A03 is answered.

## Dependency rules

- domain systems expose public interfaces/events, not private state reach-through;
- presentation observes state and sends approved semantic commands; it never becomes authoritative truth;
- immutable configuration is separate from mutable runtime state;
- cross-domain money changes route through EconomyService;
- save/load routes through explicit domain serialization/version contracts;
- vendor APIs are isolated behind adapters;
- no cyclic ownership between Fare, Passenger, Neural, Economy, World or Vehicle systems.

## Performance rules

No unbounded Tick, `GetAllActors*`-style global scans in normal hot paths, synchronous asset loads or normal-frame spawn/allocation storms. Repeated populations use bounded pools/caches/queues where appropriate. Logical entities may exist without Actors.

## Testing rule

Each owner must be testable in isolation where practical; integration tests then prove legal combinations. Build success does not equal gameplay verification. Every persistent/money-changing boundary has save/retry/duplication tests.

Runtime status remains **NOT IMPLEMENTED / NOT VERIFIED**.
