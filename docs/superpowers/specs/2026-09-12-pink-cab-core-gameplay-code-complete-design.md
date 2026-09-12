# PINK CAB Core Gameplay Code Complete Design

**Status:** OWNER APPROVED IN CHAT on 2026-09-12; written spec pending final owner review.

**Goal:** finish the complete FIRST-EURO single-player gameplay runtime in code using placeholder geometry, so remaining production work is content/art/presentation rather than missing game logic.

**Engine/runtime authority:** Unreal Engine 5.8.2. Native Chaos Vehicles is the sole production road-dynamics owner for the hero taxi. No external vehicle or damage plugin is required for this code-complete gate.

**Product boundary:** PC single-player, full Level 1 + Level 2 gameplay. Level 3, multiplayer, online rooms/racing, lifestyle interiors, full Taxi Regulator and daily insurance remain post-FIRST-EURO.

## Definition of 100% code complete

The project reaches 100% when a clean build can run the entire FIRST-EURO gameplay loop on boxes, capsules, debug materials and data fixtures with no gameplay truth owned by a final model, animation, sound, VFX or authored cinematic.

The final integrated scenario must support: boot -> drive -> order -> route -> physical placeholder passenger -> full-stop pickup -> door/admission -> boarding -> metered/off-meter fare -> continuous traffic drive -> L1/L2 traversal -> rule/fine/reputation events -> full-stop drop-off -> payment/evasion -> passenger exit -> repeat-client/Neural path -> automotive service/refuel -> save/load/retry -> next fare.
## Explicit exclusions from the 100% code gate

The following may remain placeholder-only after code complete: final hero/passenger/traffic/city meshes, skeletons, animation/IK sets, final materials/textures, authored VFX polish, final sound/music/VO, final dialogue writing, final UI art, decorative city vignettes and final lighting composition.

Placeholder actors must still expose the same gameplay sockets/events/state that production content will consume. Replacing a cube with a finished asset must not require rewriting economy, passenger, world, vehicle, service or persistence logic.

## Architectural rule

Gameplay truth lives in focused state/services and versioned data. Presentation reads authoritative state and may request legal actions, but animation notifies, mesh visibility, audio completion and transient Actor existence never own money, fare, passenger identity, route identity, vehicle health or save truth.

Duplicate-sensitive actions use stable IDs and exactly-once settlement. Persistent identity is logical/versioned data; streamed Actors are disposable materializations.

All major systems must have pure or world-light automation coverage plus one integrated PIE/runtime proof. No subsystem is considered complete from compilation alone.

## Program weighting

Progress is reported against these fixed weights: Foundation/state 10%; Vehicle/input 15%; Fare/passenger/economy 15%; City/routing/traffic 20%; L1/L2 vertical gameplay 15%; Services/refuel 10%; Passenger identity/Neural 5%; Persistence/recovery/workday 5%; Integrated code-complete verification 5%.
## 1. Foundation/state — 10%

Own build identity, runtime configuration, stable IDs, event/result envelopes, versioned schemas, transaction idempotency helpers, deterministic seeds and subsystem reset boundaries.

Required runtime services include one EconomyService ledger, one authoritative session/workday state, one save coordinator, one deterministic ID/version surface and one shared event/evidence vocabulary.

Subsystems may not invent private money balances, save formats, random identity rules or duplicate retry semantics.

Acceptance: deterministic unit tests for IDs/versioning/transactions/reset; clean UE build; failure/retry cannot double-commit a transaction.

## 2. Vehicle/input/cockpit-state — 15%

Keep the already verified native Chaos provider/Pawn baseline and finish physical-profile calibration, normalized telemetry and production input ownership.

Canonical input grammar for the code-complete program is mouse steering; Space gaze; `1–4 = START`; `LMB = ATTENTION`; `RMB = GO`; Q clutch contract; W brake; E throttle. Quick targets are turn signal, horn, gearbox and handbrake.

Until stock Chaos exposes a verified clutch input, clutch remains an explicit logical/mechanical contract gap and must never be faked as hidden throttle or torque. Gearbox, handbrake, ignition/stall, passenger-door state and meter controls are logical/placeholder-interactable even before final cockpit meshes/animations exist.

Acceptance: focus-loss/pause/recovery clears transient inputs; no stuck mouse/hand/gaze ownership; all arcade-assist forces remain independently observable and OFF in physical baseline tests.
## 3. Fare/passenger/economy — 15%

One FareSession owns the complete work transaction: order -> route -> visible placeholder passenger group -> valid pickup context -> deliberate full stop -> admit/decline -> passenger-door gate -> simultaneous group boarding -> drive -> deliberate full stop destination -> payment/consequence -> door -> exit -> next order.

Group size is 1–5, rear three seats before front two. Each passenger carries one stable deterministic physical mass; boarding/exiting applies mass exactly once.

Taximeter price is distance + elapsed fare time. METERED and OFF_METER are modes of the same session. Payment, tip, fare evasion, receipt, fine, fuel, service and repair settlement flow through EconomyService with stable transaction IDs.

Passenger bodies may be capsules/boxes. Boarding can be a compressed state transition at code-complete, but cannot occur while moving or with the passenger door logically closed.

Acceptance covers paid/unpaid exclusivity, payment-before-door vs door-before-payment, save/retry idempotency, group seating/mass and repeated fares without leaked state.

## 4. City/routing/traffic/rules — 20%

Implement deterministic `CityCode + GeneratorVersion + ContentSetVersion + persistent deltas`, one shared road/lane graph and bounded placeholder world materialization for FIRST-EURO L1+L2.

Generation, route finding, traffic placement, rule lookup, service placement and reverse reconstruction consume the same graph IDs. Turning around or reloading reconstructs the same logical route/module sequence for the supported versions.

Traffic remains continuously moving: no traffic lights and no systemic standing-jam state. Far traffic is cheap lane/spline logic; near interaction traffic upgrades to bounded collision/Chaos proxies; only the interaction bubble carries expensive simulation.

Rules emit stable EnforcementEvents for speed/lane/turn/parking/transit legality. Fines settle through EconomyService; city reputation is separate from an individual passenger relationship.
Acceptance: deterministic route results, bounded traffic populations, playable gaps, moving bypass around incidents, reverse reconstruction, repeatable calm/reckless enforcement comparison and no unbounded Actor growth in the acceptance window.

## 5. Level 1 / Level 2 vertical gameplay — 15%

Level 1 code owns wallride/contact bands, freight-ceiling traversal, receiving strips, residual magnet state, legal detach/reacquire/failure and poplar-gap cross-carriageway route hooks. Native Chaos remains ordinary road dynamics; magnetic/contact gameplay is an explicit separate layer and never secretly becomes ABS/ESP.

Residual-magnet timeout is locked by authoritative vehicle mass: <=1657 kg = 5.0 s; 1657–2107 kg = `5.0 - (mass-1657)/450`; 2107 kg = 4.0 s. Above the declared fixture is rejected until separately authorized.

Level 2 code owns suspended-bus route/obstacle state, metro/station graph/timing state and Tatra transition/contact states where approved. Metro is not a default driveable surface.

All geometry may be primitive splines/boxes at code complete. Gameplay routes, contact envelopes, failure/recovery and streaming IDs must already be production interfaces.

Acceptance: deterministic wallride/freight/magnet cases across load fixtures; L2 bus/metro/station traversal state; no stuck contact after collision/abort/reload.

## 6. Services / parking / moving refuel — 10%

One ServiceNode architecture owns Parking, Garage/Tuning, Parts and Repair/Service, with Practice Hangar only where useful for code acceptance. Nodes preserve the same owned Tatra, CityCode, VehicleBuild, VehicleHealth, inventory and EconomyService identities across enter/exit.
Moving refuel is not a ServiceNode. It is one bounded live-road session reusing FuelTank, EconomyService, VehicleTelemetry and RoadGraph. Placeholder service vehicle/hose actors follow session state; they never own credited fuel or payment truth.

Acceptance: purchase/install/repair/parking/service transactions are exactly-once; invalid part combinations fail deterministically; moving-refuel success/abort/tolerance-loss/collision/save-reload cases converge to one clean settlement.

## 7. Passenger identity / repeat clients / basic Neural — 5%

Separate transient materialized PassengerActor from persistent PassengerIdentity. Templates generate context-valid passengers; identity stores stable seed, mass, archetype/preferences, relationship state, selected ride memories, Neural permission/block state and repeat-order eligibility.

Actor stream-out cannot delete social identity. A selected client can share Neural, disappear physically, message later and create a repeat order through the ordinary Order/FareSession pipeline.

The daughter/conductor is one persistent cabin identity with code-level service/information hooks. At code complete she can emit/resolve bounded assistance tasks, but never owns steering, braking, throttle, live gap selection or silent route commitment.

Acceptance: repeat identity survives save/load and actor destruction; no duplicate repeat fare; reconstructed passenger preserves deterministic mass/context.

## 8. Persistence / recovery / workday — 5%

Save logical/versioned state rather than uncontrolled Actor snapshots. Required schemas cover CityCode/generator/content, FareSession, Economy ledger, PassengerIdentity/Neural, VehicleBuild/VehicleHealth, services/refuel and session/workday state.

Crash/force-quit boundaries may not duplicate money, passenger, fuel, parts, service or fare results. Unknown/newer schema fails explicitly or uses a tested migration.

Terminal vehicle outcome consumes a common RecoveryPolicy boundary. Until owner recovery semantics are finally locked, code must support the policy without baking an irreversible assumption into vehicle physics or economy.

Acceptance: save/load during representative states, retry/new-workday reset, schema-version rejection/migration and transaction replay protection.
## 9. Integrated verification — 5%

Code complete requires a dedicated primitive-only integration map/scenario that exercises every required service without depending on final content.

The integrated gate must prove at least two consecutive fares, one decline, one metered fare, one off-meter fare, one payment-before-door case, one eligible evasion case, one repeat-client callback, one enforcement/fine event, one ServiceNode transaction, one moving-refuel transaction, one L1 vertical traversal, one L2 transit traversal, one save/reload and one terminal/recovery/reset path.

The same candidate must pass clean UE 5.8.2 build, focused subsystem Automation, integration Automation/PIE, deterministic seed replay, runtime population caps and a bounded endurance run. Failures are fixed at their owning subsystem rather than hidden by test exceptions.

## Data flow and ownership

Input produces intent. Vehicle/input state validates ownership and forwards legal vehicle commands. VehicleTelemetry exposes normalized state. RoadGraph/CityCode supply world identity and legal route context. FareSession consumes route/passenger/door/stop facts. EconomyService alone commits money. PassengerIdentity consumes completed ride facts. Enforcement consumes rule facts. Services consume shared vehicle/economy/world state. Persistence serializes logical owners through versioned DTOs.

Cross-system communication uses explicit immutable event/result records or narrow service APIs. No subsystem reaches into another system's private Actor/component graph to mutate authoritative state.

## Error handling

Illegal state transitions fail closed and emit deterministic reason codes. Duplicate transaction IDs return the prior result rather than applying twice. Missing presentation actors do not erase logical state. Invalid CityCode/schema/content versions fail explicitly. Streaming loss reconstructs from logical identity. Focus loss/pause/recovery invalidates transient input ownership without inventing persistent vehicle changes.

## Testing doctrine

TDD is required for every production subsystem. Prefer pure structs/services for state machines and calculations; add UE world tests only for Actor/component integration; add PIE/runtime smoke only where actual physics, possession, collision, streaming or world lifetime matters.
## Current implementation state carried forward

`CD-786` is verified native-Chaos baseline and remains valid evidence: PinkCab provider -> Chaos movement, four-wheel RWD pawn, placeholder UE chassis, mouse steering, playable weave map and 7/7 ChaosBaseline Automation PASS.

`CD-787` physical calibration is no longer the standalone program goal. It becomes the first unfinished slice inside Vehicle/input/cockpit-state.

Existing mass/load, health, wallride and contract helpers are reused where they match this spec. Historical FGear/VDS research remains audit material only and cannot override the native Unreal production architecture approved after that research.

## Execution order

1. Foundation/state contracts and authority cleanup.
2. Complete native Chaos vehicle/input/cockpit-state and normalized telemetry.
3. Fare/passenger/economy vertical loop on primitives.
4. Deterministic CityCode/road graph/routing/traffic/rules.
5. Level 1/Level 2 vertical/contact/transit gameplay.
6. ServiceNodes and moving refueling.
7. Persistent PassengerIdentity/repeat clients/basic Neural/daughter service hooks.
8. Save/load/recovery/workday integration.
9. Primitive-only complete-game scenario, endurance and code-complete gate.

Each numbered block receives its own implementation plan and commits. A block may expose extension interfaces for later work but may not pull post-FIRST-EURO runtime into the code-complete gate.

## Completion rule

No Jira status, document or percentage can declare 100% by itself. The only 100% condition is fresh exact-head evidence that the primitive-only integrated FIRST-EURO loop works end to end and every mandatory subsystem test is green.
