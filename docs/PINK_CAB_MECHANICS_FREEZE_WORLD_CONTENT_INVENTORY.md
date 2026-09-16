# PINK CAB · Mechanics Freeze World / Content Inventory

Date: 2026-09-16  
Owner task: `CD-845`  
Parent gate: `CD-841`  
State: RECONCILED INVENTORY / NOT MECHANICS FREEZE

## Purpose

This file is the finite unresolved-design inventory for player-facing world/content work after primitive Core Code Complete. It records what is already authoritative, what is FIRST EURO versus post-FIRST-EURO, and which decisions still have to be made before a Mechanics Freeze claim.

Reconciliation does not invent missing mechanics. Primitive/runtime completion is not authored-content completion. Models, meshes and the supplied Tatra placeholder are intentionally untouched by this pass.

## Scope rules

- FIRST EURO remains PC / single-player / full Level 1 + Level 2.
- Level 3 gameplay is post-FIRST-EURO unless a later owner decision changes scope explicitly.
- FIRST EURO automotive ServiceNodes are Parking, Garage-Tuning, Parts and Repair-Service; Practice Hangar exists only where Level 1 training/acceptance requires it.
- Mall/food/bar/club/social lifestyle destinations and common live rooms are post-FIRST-EURO.
- Basic PassengerIdentity/repeat-client Neural substrate exists; authored principal-character and broad social design are not thereby frozen.
- Existing code contracts and verified primitive gates are reused; this inventory owns design gaps, not replacement runtime architecture.

## Decision inventory
| ID | Area | Scope | Current authority / truth | Unresolved decision before freeze | Implementation dependency | Freeze acceptance |
|---|---|---|---|---|---|---|
| `WF-01` | Level 1 authored corridor | FIRST EURO | pages 33/37; `CD-797/798`; L1 runtime primitives verified | final gameplay geometry envelopes, landmark/event placement bands and production density budget | World/Traffic/Vertical + final environment content | every gameplay-bearing L1 geometry class has data bounds and no designer must invent collision/traversal rules while dressing |
| `WF-02` | L1 destinations / service placement | FIRST EURO | page 36 ServiceNode taxonomy | allowed destination classes, spacing/distribution and approach/exit grammar per road context | ServiceNode + RoadGraph + routing | deterministic placement contract exists for every FIRST EURO destination class |
| `WF-03` | Level 2 authored topology | FIRST EURO | pages 09/35; `CD-798`; L2 transit runtime verified | production station/interchange topology, route identity, transfer points and traversal envelope | Vertical + RoadGraph + streaming | L2 can be authored without inventing new traversal/state rules |
| `WF-04` | L2 traffic/transit operating profile | FIRST EURO | pages 09/35; Traffic foundations | authored density bands, bus/metro timing profiles, station approach/clearance and incident constraints | Traffic + Vertical + performance budget | named profiles cover ordinary and stress conditions with objective bounds |
| `WF-05` | Level 3 gameplay | POST-FIRST-EURO | page 06; FIRST EURO scope owner `CD-753` | whether/when detailed L3 mechanics enter a later product gate; no silent year-one implementation | CityCode extension boundary only during FIRST EURO | explicit exclusion remains documented; existing schemas can extend without L1/L2 rewrite |
| `WF-06` | Parking | FIRST EURO | pages 16/36; `CD-799` ServiceNode runtime | authored parking categories, pricing/signage/legality presentation and location mix | ServiceNode + Economy + Enforcement | every parking class has rule, price source, persistence and enforcement consequence |
| `WF-07` | Garage / tuning | FIRST EURO | pages 16/36; VehicleBuild extension | supported tuning categories, install/remove UX, compatibility rules and authored catalog breadth | ServiceNode + Economy + VehicleBuild | finite part-category and compatibility contract exists; visuals remain replaceable assets |
| `WF-08` | Parts shops / inventories | FIRST EURO | pages 16/36 | inventory taxonomy, stock/availability policy, price ownership and persistence | ServiceNode + Economy + VehicleBuild | deterministic inventory/transaction contract exists without shop-specific bespoke code |
| `WF-09` | Repair service | FIRST EURO | page 36; Vehicle Health + Economy contracts | repair menu/categories, partial-vs-full repair policy, pricing table and presentation choreography | ServiceNode + VehicleHealth + Economy | all repairable capability states map to explicit service actions and exactly-once settlement |
| `WF-10` | Mall / food / lifestyle retail | POST-FIRST-EURO | page 16 split authority | future purpose, interaction depth and relationship to Neural/social systems | extension boundary only | explicitly excluded from FIRST EURO; no dependency leaks into current acceptance |
| `WF-11` | Bar / club / live social rooms | POST-FIRST-EURO | page 17 | future room loop, social interaction set, multiplayer/common-room relationship and moderation boundary | future Neural + online stack | explicitly excluded from FIRST EURO and tracked as future design program, not placeholder code |
| `WF-12` | Driver / heroine | FIRST EURO | Active Product Baseline; Tatra/cockpit canon | name/identity presentation, authored voice/personality boundaries, non-mechanical biography and how much appears in UI/Neural | Character content + Neural profile presentation | principal playable-character brief exists and does not alter already-locked input/vehicle authority |
| `WF-13` | Daughter / conductor/operator | FIRST EURO | page 14; daughter canon mirror | exact FIRST EURO task set, dialogue/feedback cadence, state visibility and limits before later operator expansion | Taxi + passenger/service presentation + Neural messaging | every daughter gameplay responsibility is named; forbidden hidden driving/autopilot authority remains explicit |
| `WF-14` | Other principal characters | UNFROZEN / scope decision required | no complete current principal-character roster | roster, role, encounter channel, progression relevance, FIRST EURO vs later classification | PassengerIdentity + Neural + authored content | finite roster exists or each candidate is explicitly excluded from FIRST EURO |
| `WF-15` | Authored repeat clients | FIRST EURO content over implemented substrate | `CD-800`; PassengerIdentity/basic Neural | which authored recurring clients exist, their relationship arcs, order triggers, preferences and message packs | PassengerIdentity + Order/Fare + Neural | finite authored repeat-client set/profile schema and trigger rules exist |
| `WF-16` | Economy / prices | FIRST EURO | EconomyService / exact-once contracts; ServiceNode specs | authored price tables and balancing ownership for parking, parts, tuning, repair and fines | Economy + ServiceNode + Enforcement | one versioned balance source owns prices; no location hardcodes authoritative money values |
| `WF-17` | Enforcement / fines | FIRST EURO | `EnforcementEvent`, traffic/rules core | finite rule/fine catalog, signage discoverability and service/government presentation | Traffic + Economy + Neural notices | every chargeable rule maps event → fine → notice → persistence with explicit amount source |
| `WF-18` | Shared-universe / world canon | RECONCILIATION REQUIRED | page 24 / `CD-538` and numbered world bibles | resolve disputed history/political/world facts only where they affect PINK CAB locations, institutions, signage or characters | narrative/world content | no contradictory current authority remains for player-visible FIRST EURO facts |
| `WF-19` | Weather / night-shift world states | FIRST EURO | Active Product Baseline + world systems | authored weather set, transition frequencies, gameplay visibility and which states affect traction/traffic versus presentation only | World + Vehicle surface profiles + Traffic | each weather state declares gameplay effects, presentation-only effects and deterministic/profile ownership |
| `WF-20` | World events / roadside density | FIRST EURO | bounded Traffic/World architecture | event taxonomy and simultaneous-density budgets for pedestrians, traffic incidents, roadside activity and service encounters | World + Traffic + performance | finite event classes, spawn/expiry rules and hard population budgets exist |

## Already closed anchors — do not reopen in this inventory

- No traffic lights and no systemic standing traffic-jam waiting loop.
- Taxi generally remains in motion except deliberate passenger/service/parking stop interactions.
- Taximeter basis remains distance + elapsed fare time.
- Core L1/L2 traversal/service/persistence contracts already exist and passed primitive acceptance; authored content must plug into them rather than replace them.
- Player input authority is `CD-825`; world/content work cannot add hidden direct-action cockpit shortcuts.
- Native Chaos is the production hero-vehicle dynamics path; world/content work cannot revive FGear/VDS as required dependencies.
- Model/mesh substitution is outside `CD-845`; supplied Tatra placeholder integration happens only after this reconciliation chain is complete.
