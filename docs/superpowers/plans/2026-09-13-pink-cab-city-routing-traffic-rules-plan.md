# PINK CAB City / Routing / Traffic / Rules Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete the deterministic FIRST-EURO L1+L2 city, routing, continuously-moving traffic and rules/enforcement code without final art or Actor-per-car simulation.

**Architecture:** `FPinkCabRoadGraph` remains the single topology owner. Pure C++ state/services own city deltas, anchors, routes, far traffic, incidents and enforcement; UE presentation receives bounded materialization/proxy requests and never owns gameplay truth.

**Tech Stack:** UE 5.8.2 C++, native containers, Automation tests, existing EconomyLedger/StableId/CityIdentity/RoadGraph.

**Spec:** `docs/superpowers/specs/2026-09-12-pink-cab-core-gameplay-code-complete-design.md` section 4.

## Global Constraints

- City identity is `CityCode + GeneratorVersion + ContentSetVersion`; supported versions reconstruct deterministically.
- One road/lane graph feeds generation, routing, traffic, rules, service anchors and reverse reconstruction.
- No traffic lights and no systemic standing-jam state; ordinary traffic speed must remain positive.
- Far traffic is pure data; only a bounded interaction bubble may request expensive proxies.
- Final models/materials/lighting are excluded; primitive presentation must not own logical state.
- Fines settle only through `FPinkCabEconomyLedger`; city reputation stays separate from PassengerIdentity.
- Native Chaos vehicle ownership remains unchanged.

---
## File map

- Modify `Source/PinkCab/Public/World/PinkCabRoadGraph.h`: lane metadata/query APIs needed by all later services.
- Create `Source/PinkCab/Public/World/PinkCabCityDeltaState.h`: persistent logical closures/incidents/module deltas keyed by stable IDs.
- Create `Source/PinkCab/Public/World/PinkCabCityLocationRegistry.h`: stable pickup/destination/service/rule anchors on graph lanes.
- Create `Source/PinkCab/Public/World/PinkCabRouteService.h`: deterministic bounded route search with excluded lanes and reverse reconstruction.
- Create `Source/PinkCab/Public/World/PinkCabWorldMaterializationPolicy.h`: bounded chunk/anchor materialization window.
- Modify `Source/PinkCab/Public/Traffic/PinkCabTrafficEntity.h` and `PinkCabTrafficFlow.h`: route cursor and lane-transition-safe moving simulation.
- Create `Source/PinkCab/Public/Traffic/PinkCabTrafficIncident.h`: blocked-lane incidents and deterministic bypass requests.
- Create `Source/PinkCab/Public/Traffic/PinkCabTrafficInteractionBubble.h`: bounded promotion/demotion proxy assignments.
- Create `Source/PinkCab/Public/Enforcement/PinkCabRoadRuleProfile.h`: lane/location legality lookup.
- Create `Source/PinkCab/Public/Enforcement/PinkCabEnforcementService.h`: stable event evaluation, exactly-once fine settlement and city reputation.
- Create focused tests under `Source/PinkCabTests/Private/World`, `Traffic`, and `Enforcement`.

### Task 1: Persistent city deltas and graph anchors

**Interfaces:** Produces `FPinkCabCityDeltaState`, `FPinkCabCityLocationRegistry`; both consume stable graph IDs only.

- [ ] Write RED tests proving deterministic delta IDs, duplicate rejection, lane closure lookup, anchor uniqueness and reconstruction signature.
- [ ] Build and confirm failure only on missing new headers/types.
- [ ] Implement bounded pure-data delta/anchor stores; no Actor/UObject references.
- [ ] Run `PinkCab.World.CityRuntime.Identity` and existing `PinkCab.World.CityCode` GREEN.
- [ ] Commit `feat(CD-797): add persistent city delta anchors`.
### Task 2: Shared graph metadata and deterministic routing

**Interfaces:** `FPinkCabRouteService::FindRoute(Graph, Request, DeltaState, OutRoute)` returns ordered lane IDs and a stable route signature. `FPinkCabRouteRequest` names start/goal lanes and a max-visited-node budget.

- [ ] RED: shortest deterministic route, stable tie-break by serialized lane ID, closed-lane exclusion, invalid/unreachable request and max-search budget.
- [ ] Extend `FPinkCabLogicalLane` with positive length/layer metadata while preserving current aggregate callers.
- [ ] Add read-only graph lookup/enumeration helpers; do not expose mutable containers.
- [ ] Implement deterministic Dijkstra/A* style search with bounded visited count and no Actor queries.
- [ ] RED/GREEN reverse reconstruction: reconstructing the same supported city/deltas gives the same lane sequence/signature; reverse request resolves the graph-valid reverse path when present.
- [ ] Run `PinkCab.World.Routing` + `PinkCab.World.CityCode`; commit `feat(CD-797): add deterministic road routing`.

### Task 3: Bounded world materialization

**Interfaces:** `FPinkCabWorldMaterializationPolicy` consumes player chunk/layer + candidate logical chunk IDs and emits a bounded stable request set; no spawned Actor is authoritative.

- [ ] RED: near chunks selected deterministically, L1/L2 layer filter obeyed, hard max request count, stable dematerialization set after movement.
- [ ] Implement squared-distance/order-key selection with pre-sized arrays and deterministic stable-ID tie-breaks.
- [ ] Add anchor/service-location requests through `FPinkCabCityLocationRegistry` so later ServiceNode placement reuses graph IDs.
- [ ] Test repeated forward/reverse movement produces the same logical module identities and no request-set growth.
- [ ] Run `PinkCab.World.Materialization`; commit `feat(CD-797): add bounded city materialization window`.
### Task 4: Continuous traffic, incidents and interaction bubble

**Interfaces:** logical traffic follows lane routes; `FPinkCabTrafficIncidentRegistry` exposes blocked lanes; `FPinkCabTrafficInteractionBubble` emits bounded proxy assignments by stable TrafficId.

- [ ] RED: traffic crosses lane boundaries while speed stays positive, population/gaps remain bounded and duplicate TrafficIds remain rejected.
- [ ] Add per-entity route cursor/remaining-lane distance without Actor state; clamp invalid deltas and reject zero/reverse ordinary speed.
- [ ] RED: an incident closes one lane and route service supplies a moving bypass; no ordinary entity enters a designed standing-jam state.
- [ ] Implement bypass reassignment only at deterministic lane boundaries; preserve TrafficId through reroute.
- [ ] RED: interaction bubble promotes nearest candidates up to capacity, keeps stable slots where possible, demotes leaving IDs and never allocates unbounded proxy ownership.
- [ ] Implement pool-assignment data only; actual proxy backend remains replaceable.
- [ ] Run `PinkCab.Traffic`; commit `feat(CD-797): add continuous traffic incident bypass`.

### Task 5: Road rules, enforcement, fines and city reputation

**Interfaces:** `FPinkCabRoadRuleProfile` resolves legality by graph anchor/lane. `FPinkCabEnforcementService` emits stable `FPinkCabEnforcementEvent` and settles fines into existing EconomyLedger.

- [ ] RED: speed/lane/turn/parking/transit observations resolve only from supplied rule data and stable graph IDs.
- [ ] Implement immutable observation/result structs and data-driven lookup; no UI strings or PassengerIdentity mutation.
- [ ] RED: identical event operation replays exactly once; fine transaction uses `EPinkCabTransactionType::Fine`; city reputation applies once.
- [ ] Add duplicate EventId protection to enforcement ledger/service and stable fine transaction IDs derived from EventId.
- [ ] RED calm/reckless comparison with identical route: calm emits zero/less severity, reckless emits repeatable events/fines/reputation deltas.
- [ ] Run `PinkCab.Enforcement` + `PinkCab.Economy`; commit `feat(CD-797): integrate road enforcement economy`.
### Task 6: Integrated primitive-only city acceptance gate

**Interfaces:** one acceptance fixture composes CityIdentity + deltas + anchors + RoadGraph + RouteService + MaterializationPolicy + Traffic + Incidents + Enforcement, while each subsystem remains independently replaceable.

- [ ] Build a deterministic L1+L2 primitive fixture with alternate lane bypass and graph-anchored pickup/destination/service/rule points.
- [ ] RED/GREEN: same CityCode/version/deltas reproduces topology, route, anchor and materialization signatures after forward travel, turn-around and reconstruction.
- [ ] RED/GREEN: acceptance window keeps logical traffic and promoted proxy assignments under explicit ceilings while all ordinary traffic remains moving.
- [ ] RED/GREEN: incident causes deterministic moving bypass rather than standing jam.
- [ ] RED/GREEN: calm vs reckless traversal produces repeatable enforcement difference and exactly-once fine settlement.
- [ ] Fresh build; run `PinkCab.World`, `PinkCab.Traffic`, `PinkCab.Enforcement`, `PinkCab.Economy` and affected Taxi/Vehicle regressions.
- [ ] `git diff --check`, commit final integration, attach evidence to CD-797 and transition DONE only after all fresh gates are green.

## Self-review

- Spec coverage: city/version identity, persistent deltas, shared graph, route finding, bounded materialization, reverse reconstruction, moving traffic/gaps, incidents/bypass, interaction bubble, rules, fines and city reputation each map to a task.
- Optimization: no per-frame UObject allocation; far traffic is pure data; promotion is capped; route search has a hard visited budget; presentation/proxy backends consume requests only.
- Ownership: RoadGraph owns topology, EconomyLedger owns money, Enforcement owns city reputation, PassengerIdentity remains untouched, Chaos vehicle ownership unchanged.
- Placeholder scan: no TBD/TODO steps; all acceptance behavior is explicitly assigned.
- Scope boundary: actual L1 wallride/freight/magnet and L2 bus/metro traversal mechanics remain CD-798; this task provides their graph/layer/streaming IDs only.
