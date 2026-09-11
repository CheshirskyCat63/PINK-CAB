# PINK CAB · Production Workflow

**Status:** CURRENT ENGINEERING/ADMIN WORKFLOW
**Product:** `CD-519`
**BASE-100:** `CD-746..753`; Confluence `11239425`
**Repository:** `CheshirskyCat63/PINK-CAB` is the active technical source of truth; DEADRACE pursuit/combat-era workflow is superseded.

## Source of truth

| Information | Authority |
|---|---|
| live priority, dependencies, owner decisions, status, blockers, evidence pointers | Jira |
| durable product/system contracts | Confluence |
| implementation-facing docs, code/config/tests/commits/PR/build history | GitHub |
| actual runtime outcome | exact build/runtime evidence |

`CANON → SPECIFIED → IMPLEMENTED → VERIFIED`.

Never close a runtime behavior as VERIFIED from prose, ticket status or a successful compile alone.

## BASE-100 work intake

Every implementation issue should state:

1. player/runtime outcome;
2. authoritative owner and dependency contract;
3. `BUY / ENGINE / WRITE` choice;
4. state machine/data/numeric config;
5. persistence/transaction behavior;
6. failure/retry/cleanup behavior;
7. performance bound/hot path;
8. acceptance fixture and evidence fields;
9. explicit extension/non-goals.

If an owner decision is still open, do not silently make it in code. Temporary experiments are `EXPERIMENTAL / NON-AUTHORITY`.

## Buy-before-build rule

Before creating a major subsystem:

1. check native Unreal feature suitability;
2. check mature ready-made plugin/asset/library options;
3. choose one authoritative production owner;
4. isolate vendor API behind a PINK CAB adapter/interface;
5. write only the product-specific gap.

Competing duplicate production solvers are forbidden after stack lock. Bake-off prototypes are allowed only before selection.

FGear + Debugging Delight VDS is the current exemplar: FGear owns hero-Tatra road dynamics, VDS owns permanent deformation, PINK CAB owns adapters/Vehicle Health/Expression/persistence/telemetry.

## Development flow

1. Select/activate a Jira issue with a current authority link.
2. Read `AUTHORITY.yaml`, the relevant Confluence page and implementation mirror.
3. Confirm unresolved owner questions are either closed or explicitly parameterized/calibration-only.
4. Work on a short-lived Jira-keyed branch.
5. Implement the smallest coherent domain outcome without bypassing owner boundaries.
6. Run unit/automation/runtime/performance checks appropriate to the change.
7. Open PR with exact test/evidence summary and any authority-impact statement.
8. Merge only after review and required checks.
9. Attach/reference evidence in Jira.
10. Transition the issue only when its stated acceptance is actually met.

## Architecture discipline

- C++: interfaces, authoritative state, persistence, versioning, invariants, critical orchestration and vendor adapters.
- Blueprint: thin composition/orchestration.
- DataAssets/DataTables/config: tunable values and versioned profiles.
- Actors: runtime materialization, not default persistent truth.
- Events/semantic commands: preferred cross-domain communication.
- EconomyService: sole monetary balance/settlement owner.
- Persistence: explicit version/migration, no silent incompatible fallback.
- Duplicate-sensitive fare/fuel/service/economy state: stable IDs + exactly-once behavior.

## Hot-path rules

Reject production designs that require:

- unbounded Tick work;
- global actor scans in normal gameplay hot paths;
- synchronous asset loads in normal hot paths;
- repeated SpawnActor/NewObject storms in normal frames;
- uncontrolled array/queue/cache growth;
- Actor-per-logical-entity where no runtime behavior requires it.

Use bounded logical populations, pools, caches, queues, instancing/HLOD where applicable and explicit lifecycle/cleanup.

## FIRST EURO 12-month delivery order

Current dependency order is summarized in `PROGRAM_ROADMAP.md`:

`BASE-100/foundation → FGear+VDS Tatra → Taxi/Fare/Passenger/Neural → Economy/Save/Workday → CityCode/RoadGraph/Streaming → Traffic/Rules → Level1 → Interchange → Automotive ServiceNodes → Moving Refuel → Level2 → Integrated FIRST EURO`.

Multiplayer, Level3 gameplay, lifestyle/social ServiceNodes, full Taxi Regulator and daily insurance are POST-FIRST-EURO and do not appear in the first-year critical path.

## Evidence families

### Vehicle

Record exact UE/FGear/VDS/profile versions, input, mass/load, speed/yaw/slip/contact, physical vs Expression channels, damage/VehicleHealth result and artifacts. `CD-738` is integrated vehicle proof.

### Taxi / passenger / Neural

Prove full-stop pickup/drop-off, physical door contract, FareSession/meter/payment ordering, mass once-only application, PassengerIdentity persistence, repeat-client/Neural behavior and no duplicate transaction/state after save/retry.

### Economy / session / persistence

Prove exact transaction IDs, before/after state, workday/session IDs, checkpoint/resume behavior, unknown/newer schema handling and anti-duplication through forced restart. Debt/recovery rules follow owner-approved state machines.

### World / traffic / Level1 / Level2

Record CityCode/generator/content versions, deterministic seeds, road/lane graph IDs, chunk lifecycle/counters, traffic materialization, rules/events, L1 wallride/freight/contact, exact load-sensitive residual formula and Level2 bus/metro/station state.

### ServiceNode / moving refuel

ServiceNode proof confirms same Tatra/CityCode/passenger/fare/economy/inventory/VehicleBuild/VehicleHealth identity after node transitions. Moving-refuel proof confirms bounded session, connection/tolerance, fuel mass, EconomyService settlement, interruption/restart cleanup and no duplication.

### Performance

Use fixed deterministic worst-case fixtures. Capture frame time, memory, Actors/Components, materialized logical populations, pool/queue sizes and monotonic-growth checks. Presentation cost can be measured separately from code-readiness scoring.

### Release

Store/build claims must match the exact shipping candidate. Re-check live storefront requirements near release; do not rely on stale platform policy/pricing copied into design docs.

## Design drift

If implementation changes intended behavior:

1. create/resolve owner decision in Jira;
2. update durable Confluence authority;
3. align Git implementation mirrors/tests;
4. only then merge code that relies on the change.

Never keep two contradictory active baselines.

## FIRST EURO scope guard

Do not import legacy DEADRACE launcher/police pursuit/combat/destruction/drone-news systems. Do not implement speculative multiplayer or insurance code in the first-year runtime merely for future-proofing. Preserve clean extension interfaces instead.

## Current readiness

The old ≈59% readiness figure is historical. Current normalized Technical Owner Pack census is 196 rows and current START-90 specification readiness is **46.9%**; broad production remains HOLD.

Runtime remains **NOT IMPLEMENTED / NOT VERIFIED**.
