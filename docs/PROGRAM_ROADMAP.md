# PINK CAB · FIRST EURO 12-Month Program Roadmap

**Status:** CURRENT DEPENDENCY/DELIVERY MIRROR / NOT A PROMISE OF CALENDAR DATES
**Product:** `CD-519`
**BASE-100:** `CD-746`; scope `CD-753`; readiness `CD-660/CD-661`
**Confluence:** `11239425`, FIRST EURO roadmap `6651945`, L1→L2 roadmap `8716289`

## Roadmap rule

The previous Week-1…Week-8 FIRST EURO and Month-5…12 online sequence are **SUPERSEDED**. FIRST EURO now means the first 12 months ending in a complete **PC single-player** product.

This roadmap gives dependency order. Research/calibration can overlap; acceptance cannot skip required predecessor contracts/evidence.

## Phase 0 · BASE-100 / foundation

Owners `CD-746/CD-747/CD-753`, open decisions `CD-588/CD-673`.

Close code-facing owner decisions and exact bootstrap stack sufficiently for START-90. Lock project/module ownership, engine/plugin/version manifest, config/data conventions, save/schema/migrations, logging/build/package/evidence and hot-path rules.

Prefer engine/ready plugin/asset solutions before custom frameworks. A vendor system is isolated behind a PINK CAB adapter.

## Phase 1 · Hero Tatra foundation

Owners `CD-730..740`, handling `CD-729`, input `CD-520/CD-733`.

Order: pin FGear+VDS → authoritative Tatra profile → normalized input → dry/wet/storm calibration → Tatra Expression → VDS deformation → Vehicle Health bridge → damage/save → integrated vehicle proof `CD-738`.

No parallel hero-Tatra vehicle or deformation solver.

## Phase 2 · Taxi / fare / passenger core

Owners `CD-749`, existing `CD-522/CD-720/CD-721/CD-724`, Neural/passenger `CD-570/CD-571`.

Implement one taxi-work/FareSession pipeline, full-stop pickup/drop-off, physical door contract, taximeter, payment/evasion idempotency, passenger logical/materialized split, persistent PassengerIdentity, repeat clients and basic Neural messages/history/repeat orders.

## Phase 3 · State / economy / workday / persistence

Owners `CD-750`, `CD-560/CD-569/CD-598/CD-601/CD-602`.

One EconomyService/transaction ledger, workday/session state, save/checkpoint/recovery behavior, schema migration and anti-duplication tests. Insurance-specific state is POST-FIRST-EURO; retain only generic RecoveryPolicy/RecoveryHook.

## Phase 4 · CityCode / road graph / streaming

Owners `CD-751`, `CD-565/CD-589`.

Implement reconstructable `CityCode + GeneratorVersion + ContentSetVersion + persistent deltas`, deterministic IDs, one road/lane graph, UE World Partition/PCG where suitable plus thin PINK CAB recipe logic, bounded logical populations/materialization and reverse reconstruction.

## Phase 5 · Traffic / rules / enforcement facts

Owners `CD-567/CD-591/CD-751`.

Deliver moving no-signal traffic, playable-gap constraints, incidents/bypass, routing/rule lookup and stable `EnforcementEvent` facts consumed by first-year fines/reputation. Full Taxi Regulator is POST-FIRST-EURO.

## Phase 6 · Level1 complete

Owners/gates `CD-678/CD-679`, QA `CD-700..703`, vertical `CD-592/CD-701`.

Deliver five-lane Level1, surfaces, moving traffic, seven-band wallride, five-lane opposite freight ceiling, load-sensitive contact, exact linear residual function `5.0 s @1657 kg → 4.0 s @2107 kg`, poplar side-swap, incidents, deterministic reconstruction and Corridor Zero proof.

## Phase 7 · Right-entry interchange

Owners `CD-680/CD-681/CD-704` plus detailed implementation tasks.

Deliver deterministic right-entry pipe/tube families with the same FGear Tatra profile and road graph; no interchange-specific vehicle solver.

## Phase 8 · Automotive ServiceNodes

Owners `CD-576/CD-711/CD-713`, BASE-100 `CD-752`.

Implement one shared ServiceNode transition/persistence architecture for Parking, Garage/Tuning, Parts Shop, Repair/Service and Practice Hangar where Level1 requires it. Preserve one Tatra, CityCode, passenger/fare, money, inventory, VehicleBuild and Vehicle Health identity.

Lifestyle/social/common-lobby node runtime is POST-FIRST-EURO.

## Phase 9 · Moving refueling

Owners `CD-540/CD-593/CD-752`.

Implement bounded MovingRefuelSession over shared FuelTank, EconomyService, VehicleTelemetry and RoadGraph; no parallel fuel/money/input/save ownership. Prove success, abort, tolerance loss, collision, save/restart/chunk interruption and exactly-once settlement.

FIRST EURO services only need L1+L2. L3 service gameplay is post-year.

## Phase 10 · Level2 complete

Owners `CD-682/CD-683/CD-705..710`, vehicle boundary `CD-748`.

Deliver suspended bus/metro/stations, final bus profile, two metro tracks per side, station function/geometry/timing and Tatra traversal using the same vehicle/world/persistence foundations. Prove Transit Zero.

## Phase 11 · Integrated FIRST EURO

Owners `CD-594`, readiness/evidence `CD-660/CD-661`, verification matrix.

Prove a complete workday/session with vehicle, taxi/passenger/Neural, economy/save, L1/L2, traffic/rules, automotive ServiceNodes and moving refuel. Run deterministic restart/reload/failure paths, full two-hour soak under the final shift-timer semantics, packaged PC build and release acceptance.

## Explicit POST-FIRST-EURO program

Not first-year gates:

- multiplayer/coop/common rooms/online vehicle/racing/moderation;
- Level3 gameplay;
- mall/food/bar/club/social ServiceNodes;
- full Taxi Regulator/inspection gameplay;
- daily insurance purchase/premium/policy/claim/extra-life system.

Legacy Jira gates/issues for these may remain as future backlog authorities, but they cannot block FIRST EURO or appear as year-one acceptance requirements.

## Current owner-decision dependency

Technical Owner Pack 01 (`docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md`, `CD-673`) is the active closure queue. Any proposed default remains unowned until accepted. Pure plugin constants become CALIBRATION once observable owner behavior/range/test target is locked.

## Status rule

A phase can be fully SPECIFIED without being IMPLEMENTED. A phase can be IMPLEMENTED without being VERIFIED. Only exact build/runtime evidence closes verification.
