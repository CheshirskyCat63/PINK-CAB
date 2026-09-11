# PINK CAB · ServiceNode Acceptance QA

**Status:** CURRENT FIRST-EURO QA MIRROR / NOT RUNTIME VERIFIED
**Jira:** `CD-713`
**BASE-100:** `CD-746/CD-752/CD-753`
**Architecture:** `CD-576`, schema/persistence `CD-711`, future common-lobby `CD-712`
**Confluence:** page `9076737`

## FIRST EURO scope

Year-one acceptance covers automotive ServiceNodes only:

`Parking / Practice Hangar where required / Parts / Garage-Tuning / Repair-Service`.

Social/lifestyle/common-lobby tests are POST-FIRST-EURO and cannot block the 12-month single-player product.

Moving refueling is FIRST EURO but remains a separate live-road service QA family rather than a ServiceNode transition case.

## Fixture identity

Every run records:

- exact commit/build;
- CityCode + generator/content version;
- `ServiceNodeId`, type and placement ID;
- `PersistenceVersion`;
- entrance/exit module IDs;
- owned Tatra identity;
- `VehicleBuild` version/identity;
- Vehicle Health state where repair is tested;
- passenger/fare state where relevant;
- transaction IDs where relevant.

Visual theme is not required as a BASE-100 code fixture unless it changes runtime configuration.

## FIRST EURO core tests

| ID | Node / area | Required proof |
|---|---|---|
| `SN-PARK-001` | Parking Hub | park/secure/enter/return reconstructs same taxi/world |
| `SN-PARK-002` | Parking persistence | save/checkpoint/day-end behavior matches the final `O03` contract without duplicate state |
| `SN-HANG-001` | Practice Hangar | road→hangar→floor/wall/ceiling/wall/floor→road round trip uses same vehicle/traversal stack |
| `SN-HANG-002` | Practice reset | failed attachment/rollover resets cleanly without duplication/loss |
| `SN-PART-001` | Parts Shop | purchase charges/awards exactly once |
| `SN-PART-002` | Parts retry | timeout/retry/reload cannot duplicate money/items |
| `SN-PART-003` | Compatibility | incompatible part is rejected deterministically |
| `SN-GAR-001` | Garage | install/remove/replace modifies versioned VehicleBuild exactly once |
| `SN-GAR-002` | Build reload | exit/save/reload reconstructs exact build |
| `SN-GAR-003` | Migration | old/new/unknown VehicleBuild schema migrates or fails explicitly |
| `SN-REP-001` | Repair | repair/maintenance transaction is idempotent and persistent |
| `SN-REP-002` | Vehicle Health | repaired components match authoritative Vehicle Health state and approved vehicle adapter effects |
| `SN-SCHEMA-001` | Schema | unknown/newer ServiceNode version does not silently load incompatible state |
| `SN-ID-001` | CityCode identity | same CityCode/content version reconstructs same known node entrance/type |
| `SN-OWN-001` | Vehicle ownership | no enter/exit/retry path spawns a second owned Tatra |
| `SN-OWN-002` | State ownership | no passenger/inventory/money/VehicleBuild/VehicleHealth duplication or loss |
| `SN-INT-001` | Interrupted transition | save/restart/crash/interruption resolves to one deterministic node/world state |

## POST-FIRST-EURO tests

Only when future scope is promoted:

- lifestyle/social ServiceNode behavior;
- lobby instance identity/capacity;
- party/invite permissions;
- host CityCode authority;
- each player’s vehicle ownership;
- long-distance interest management;
- reconnect/host-leave/failure cleanup;
- moderation/privacy/safety;
- no duplicated account/vehicle/economy state through network retry.

These tests do not participate in FIRST EURO BASE-100 or year-one release acceptance.

## Boundary tests

ServiceNode QA must reject implementations that:

- create destination-specific save/economy/vehicle ownership frameworks;
- replace moving refueling with indoor `REPAIR_SERVICE`;
- treat Level2 station transit state as ServiceNode-owned;
- fork persistence by visual theme;
- write vendor vehicle plugin state directly from Garage/Parts code outside the approved adapter;
- use destination-specific monetary mutation without shared exactly-once transaction semantics;
- require social/network runtime for a FIRST EURO automotive node.

## Verification rule

Documentation establishes SPECIFIED state only. VERIFIED requires exact executable evidence with expected/observed results and artifact paths.
