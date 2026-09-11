# PINK CAB · ServiceNode City Destination Taxonomy & Contract

**Status:** CURRENT IMPLEMENTATION-FACING MIRROR
**Confluence:** page `9076737`
**BASE-100:** `CD-746/CD-752/CD-753`
**Jira:** architecture `CD-576`, schema/persistence `CD-711`, FIRST EURO QA `CD-713`, future common-lobby `CD-712`

## 1. Definition

`ServiceNode` is the common architecture for bounded purposeful destinations attached to the persistent PINK CAB road world.

Canonical code transition:

`ROAD WORLD -> ELIGIBLE ENTRANCE -> ENTER -> NODE ACTIVE -> COMMIT/TRANSACTION -> EXIT -> SAME OWNED TATRA / SAME CITYCODE`

A ServiceNode is not a pedestrian open world and is not permission for each destination to invent separate save, vehicle, economy or networking ownership.

## 2. FIRST EURO scope

The first 12-month single-player product implements only automotive node types:

- `PARKING_HUB`;
- `GARAGE_TUNING`;
- `PARTS_MARKET` / `PARTS_SHOP`;
- `REPAIR_SERVICE`;
- `PRACTICE_HANGAR` where required as the Level1 training/parking-derived subtype.

Moving refueling is also FIRST EURO but is a separate live-road mechanic, not a ServiceNode subtype.

## 3. POST-FIRST-EURO extensions

Preserved but not year-one implementation:

- `MALL_RETAIL`;
- `DINER_CANTEEN`;
- `CLUB_BAR_SOCIAL`;
- `EVENT_CLASS`;
- social/casino-theme non-real-money activities;
- common-lobby/network/social runtime.

Optional schema extension fields may be reserved, but FIRST EURO code may not depend on network/social systems.

## 4. Shared versioned identity

A FIRST EURO node uses one versioned schema containing where applicable:

- `ServiceNodeId`;
- `ServiceNodeType`;
- `Tier`;
- `CityCodePlacementId`;
- `EntranceModuleId` / `ExitModuleId`;
- `ParkingContract`;
- activity/interior endpoint IDs needed by runtime;
- `EconomyEndpointSet`;
- `InventoryEndpointSet`;
- `VehicleBuildEndpointSet`;
- `SaveReturnPolicy`;
- `CapacityProfile` where technically relevant;
- `PersistenceVersion`.

Future-only `SocialMode`, `LobbyTemplateId`, moderation/network fields remain optional extension data.

Unknown/newer/incompatible versions migrate or fail explicitly. Silent reshuffle/duplication is invalid.

## 5. Ownership invariants

Across every FIRST EURO subtype:

- one owned Tatra remains one owned Tatra;
- node entry/exit cannot duplicate or lose taxi, passenger, money, inventory, `VehicleBuild` or Vehicle Health state;
- EconomyService is sole monetary settlement owner;
- deterministic entrance identity derives from `CityCode + generator/content version`;
- retry/save/load paths are idempotent where transactions are involved;
- representation/interior map is not persistent truth;
- Garage changes vehicle only through versioned VehicleBuild/profile adapters;
- Repair consumes the same Vehicle Health component state used by damage;
- outside road simulation may be abstracted/streamed while player is inside according to global world/session rules.

## 6. `PARKING_HUB`

Code purpose: legal/secure parking state, entry/exit transition and exact taxi/world return. Whether Parking also owns explicit save/checkpoint/day-end actions remains owner-open in Technical Owner Pack `O03`.

## 7. `PRACTICE_HANGAR`

Level1 training subtype using the same ServiceNode transition/persistence contract.

`LIVE ROAD -> NODE ENTRY -> HANGAR ACTIVE -> FLOOR/WALL/CEILING/WALL/FLOOR -> RESET/EXIT -> SAME TATRA/CITYCODE`

FGear remains vehicle-physics authority; the Hangar does not create another traversal solver.

## 8. `PARTS_MARKET` / `PARTS_SHOP`

Versioned item/catalog IDs, compatibility checks, inventory persistence and exactly-once purchase/refund/retry through EconomyService. Exact stack/slot/inventory behavior remains code-facing owner work where it changes gameplay.

## 9. `GARAGE_TUNING`

Install/remove/replace parts and modify versioned `VehicleBuild`. Physical/mechanical changes reach FGear only through the approved PINK CAB vehicle profile/adapter boundary. VehicleBuild save/migration is explicit.

## 10. `REPAIR_SERVICE`

Reads component Vehicle Health, applies authored repair/service choices, settles cost via EconomyService and persists restored component state. It does not replace moving refueling.

## 11. Level1 / Level2 placement

Every ServiceNode entrance is a deterministic road-world module with Tier/road eligibility, entrance/exit IDs, conflict exclusions, transition trigger, safe return socket/policy and content/persistence version.

L1 may host year-one automotive nodes. L2 station/bus/metro remains transit/world infrastructure; an attached automotive node can consume ServiceNode, but transit timing/geometry is not owned by the node.

## 12. Moving refueling boundary — FIRST EURO

Moving refueling lives under `CD-593/CD-752` and Confluence `6062115`.

It reuses `FuelTank + EconomyService + normalized VehicleTelemetry + RoadGraph + global persistence/evidence` and owns only its bounded refuel session. It does not fork fuel, money, vehicle physics or save truth.

## 13. Future common-lobby contract — POST-FIRST-EURO

Owner `CD-712` preserves future direction:

`PERSONAL CITY -> SERVICENODE -> COMMON LOBBY -> PARTY/INVITE -> HOST CITYCODE -> OWN VEHICLES`

No lobby/session/replication/party/moderation runtime belongs to FIRST EURO. Year-one schema merely avoids blocking future subtype/session extensions.

## 14. Save / transaction rules

Each FIRST EURO subtype explicitly defines save allowance, serialized logical state, resume location, interruption/crash behavior, content-version migration and duplicate-Tatra prevention.

Any money/item/build-changing action uses explicit idempotent/exactly-once transaction semantics where retries can occur.

## 15. FIRST EURO acceptance

QA mirror: `docs/qa/PINK_CAB_SERVICENODE_ACCEPTANCE.md`; owner `CD-713`.

Year-one proof covers Parking, Practice Hangar where applicable, Parts, Garage, Repair, invalid/newer schema, exact taxi/CityCode return and no vehicle/passenger/item/money/build/health duplication or loss.

Social/common-lobby/disconnect/host-leave tests are POST-FIRST-EURO and do not block current acceptance.

## 16. Current code-facing OPEN decisions

- `O02` exact transition implementation form;
- `O03` Parking save/day-end/checkpoint responsibility;
- parts/inventory representation where behavior changes;
- exact safe resume rules where not already decided by global Session/Save authority.

Visual theme/interior dressing is outside BASE-100.

## 17. Maturity

FIRST EURO automotive scope and architecture are SPECIFIED in direction. Runtime nodes remain NOT IMPLEMENTED / NOT VERIFIED until exact-build evidence passes `CD-713`.
