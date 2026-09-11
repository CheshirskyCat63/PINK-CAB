# PINK CAB · FIRST EURO Scope

**Status:** CURRENT IMPLEMENTATION-FACING SCOPE MIRROR
**Jira:** `CD-746`, scope owner `CD-753`, readiness `CD-660/CD-661`
**Confluence:** page `11239425`, roadmap page `6651945`
**Repository:** `CheshirskyCat63/PINK-CAB` is the sole active PINK CAB technical truth; `DEADRACE` is legacy/migration-source only.

## Definition

FIRST EURO = **the first 12 months of development** and ends as a complete **PC single-player PINK CAB product**. The previous DEADRACE pursuit/combat FIRST EURO and the later eight-week management target are SUPERSEDED.

BASE-100 scores implementation-facing code/logic/numeric/ownership/data/persistence/performance/test completeness for this scope. Visual/art/presentation completeness is outside the denominator unless it creates a runtime/API/performance contract.

## FIRST EURO implementation — IN

- full playable Level 1 + Level 2;
- hero bespoke early/Gen-1 Tatra 603-family through FGear Vehicle Physics + Debugging Delight Vehicle Damage System;
- PINK CAB input/profile/Vehicle Health/Expression/persistence/telemetry adapters;
- complete taxi order → pickup → boarding → fare → payment/evasion → drop-off loop;
- procedural passengers plus persistent repeat clients + basic Neural history/contact/messages;
- one personal deterministic CityCode city, versioned generation/streaming/persistence;
- common road/lane graph, navigation, traffic, incidents, rules/fines and `EnforcementEvent`;
- automotive ServiceNodes: Parking, Garage/Tuning, Parts Shop, Repair/Service; Practice Hangar where L1 training/acceptance requires it;
- moving refueling gameplay;
- economy/workday/session/save/migration/config/build/package/QA/evidence architecture;
- first-person driving/work, keyboard+mouse baseline.

Workday duration is LOCKED by `I08`: 12 in-game hours = 120 real minutes (x6), with eligible early sleep/end-day. `I03/I04` prohibit manual exit while a FareSession/passenger is active and require a full stop after fare completion before save+exit. `I06` owns the locked FIRST EURO terminal-crash Repair recovery path.

## POST-FIRST-EURO implementation

- multiplayer/coop/common rooms/online clubs and all networking runtime;
- Level 3 gameplay;
- lifestyle ServiceNodes: mall, food, bar, club/social interiors;
- full Taxi Regulator / inspection gameplay — FIRST EURO only preserves stable `EnforcementEvent` producers/consumers;
- daily insurance purchase/toggle/premium/policy/claim/extra-life system — FIRST EURO only preserves generic `RecoveryPolicy/RecoveryHook` extension boundary.

Deferred features must not become speculative year-one code dependencies.

## Architecture rule

For every large system use:

`ENGINE / READY ASSET / PLUGIN → PINK CAB ADAPTER → PINK CAB DATA/STATE → GAMEPLAY ORCHESTRATION`

before considering a custom replacement framework.

One authoritative owner per responsibility; no production duplicate solvers. C++ owns interfaces/state/persistence/invariants/critical orchestration, Blueprint stays thin, DataAssets/DataTables/config own tunables.

## Required technical domain set

- CORE — `CD-747`: runtime/data/input/plugin/performance/evidence;
- VEHICLE — `CD-748`: FGear/VDS/Vehicle Health/wallride/L2 vehicle integration;
- TAXI — `CD-749`: FareSession/passengers/repeat clients/Neural;
- STATE — `CD-750`: EconomyService/workday/save/transactions;
- WORLD — `CD-751`: CityCode/streaming/road graph/traffic/rules/EnforcementEvent;
- SERVICE — `CD-752`: automotive ServiceNodes + moving refuel;
- SCOPE — `CD-753`: first-year/post-year boundary.

Each domain must specify `PURPOSE → BUY/ENGINE/WRITE → OWNER → STATES → INPUTS → EVENTS → DATA → NUMERIC CONFIG → PERSISTENCE → DEPENDENCIES → FAILURES → PERFORMANCE → TESTS → EXTENSIONS`.

## Readiness

The historical six-domain ≈59% figure is retired. Current code-only START-90 readiness is **47.4%** from the normalized 196-row Technical Owner Pack 01. Administrative authority hygiene is separate from unresolved owner decisions.

Documentation establishes CANON/SPECIFIED only. Runtime remains **NOT IMPLEMENTED / NOT VERIFIED** without exact executable evidence.
