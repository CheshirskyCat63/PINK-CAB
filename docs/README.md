# PINK CAB В· Documentation Index

**Status:** CURRENT REPOSITORY ENTRYPOINT
**Active product:** PINK CAB `CD-519`
**BASE-100 program:** `CD-746`; scope `CD-753`; readiness `CD-660/CD-661`
**Confluence authority index:** `6586369`
**Repository:** `CheshirskyCat63/PINK-CAB` is the sole active PINK CAB technical truth.

## Start here

1. [`CONTROL_PLANE.md`](CONTROL_PLANE.md) — single active Git/Jira/Confluence execution path, resource policy, and resume gate.
2. [`AUTHORITY.yaml`](AUTHORITY.yaml) вЂ” machine-readable current authority/scope graph.
3. [`PINK_CAB_ACTIVE_BASELINE.md`](PINK_CAB_ACTIVE_BASELINE.md) вЂ” concise active product baseline.
4. [`FIRST_EURO_SCOPE.md`](FIRST_EURO_SCOPE.md) вЂ” exact first-12-month single-player scope.
5. [`PINK_CAB_BASE100_CODE_ARCHITECTURE.md`](PINK_CAB_BASE100_CODE_ARCHITECTURE.md) вЂ” modular implementation doctrine.
6. [`PINK_CAB_BASE100_TECH_OWNER_PACK_01.md`](PINK_CAB_BASE100_TECH_OWNER_PACK_01.md) вЂ” current code-only owner-question intake.
7. [`PINK_CAB_START90_READINESS.md`](PINK_CAB_START90_READINESS.md) вЂ” current reproducible START-90 score and gap.
8. [`OPEN_DECISIONS.md`](OPEN_DECISIONS.md) вЂ” current implementation-facing unresolved locks.
9. [`VERIFICATION_MATRIX.md`](VERIFICATION_MATRIX.md) вЂ” FIRST EURO executable proof index.
10. [`PROGRAM_ROADMAP.md`](PROGRAM_ROADMAP.md) вЂ” current dependency/delivery roadmap.
11. [`PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`](PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md) — current owner control/vehicle-mechanics release authority mirror (`CD-848`, Confluence `16744449`).

## Current execution freeze — 2026-09-22

Gameplay changes are paused while Jira `CD-868` closes control-plane drift. Jira `CD-848` remains the current mechanics/recovery owner. Git PR #7 is the sole active implementation PR and targets `main`. Heavy UE verification is manual-only; use the default `fast` lane for normal iteration and `human_gate` only for an owner-test candidate.

Last runtime-affecting candidate before admin-only cleanup: `8bed51e5c47e037eb45a670f9d6c1c2cdd28b712`; run `35746083559` failed packaged Windows OS-input throttle dosing after build/tests/package/smoke passed. There is no current HUMAN_ACCEPTED vehicle build.

## BASE-100 rule

BASE-100 measures implementation-facing **code logic, state ownership, numerics, data, persistence, dependency direction, failure behavior, performance bounds, engine/plugin choices and test contracts** for the complete FIRST EURO.

Visual/art/material/lighting/animation/narrative completeness does not enter the readiness denominator unless it creates a runtime/API/performance requirement.

`CANON в†’ SPECIFIED в†’ IMPLEMENTED в†’ VERIFIED`

No prose, ticket status or committed config proves runtime behavior by itself.

## Current START-90 snapshot вЂ” 2026-09-16

Technical Owner Pack 01: **196 rows = 105 LOCKED + 3 CALIBRATION + 69 PROPOSED DEFAULT + 19 OPEN**.

Scoring is reproducible: LOCKED/CALIBRATION = 1.0; PROPOSED DEFAULT = 0.5; OPEN = 0.0.

**Current code-only specification readiness = 72.7%. START-90 FAIL. Gap = 17.3 percentage points.**

Domain snapshot: CORE 69.1%; VEHICLE 71.4%; TAXI 85.0%; STATE 92.9%; WORLD 67.2%; SERVICE 56.5%. SCOPE `CD-753` is separately 100% locked.

If all 69 proposed defaults are owner-accepted, score becomes 177/196 = 90.31%; no remaining OPEN row is required to cross START-90. Historical в‰€59%, 53.0% and 38.2% scores are retired.

## FIRST EURO вЂ” locked delivery scope

FIRST EURO = **first 12 months / PC / single-player / full L1 + L2**.

Included: native Chaos hero Tatra + bounded authored/Chaos damage/destruction, taxi/fare/passenger loop, repeat clients + basic Neural, deterministic CityCode/world/road/traffic/rules, automotive ServiceNodes `Parking/Garage-Tuning/Parts/Repair-Service` plus Practice Hangar where Level1 training/acceptance requires it, moving refueling, economy/session/save/config/build/QA foundations.

VEHICLE DEPENDENCY POLICY: production vehicle physics/destruction spend target is EUR 0. FGear/VDS evaluation remains archived research only; no vehicle-plugin purchase is on the production critical path.

POST-FIRST-EURO: multiplayer/coop/common rooms, Level3 gameplay, lifestyle ServiceNodes, full Taxi Regulator and daily insurance. Only minimal extension boundaries are preserved in year one.

## Current implementation-facing system mirrors

- control/mechanics release contract: [`PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`](PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md);
- vehicle stack: [`PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md`](PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md);
- hero-vehicle pre-model freeze: [`PINK_CAB_PRE_MODEL_VEHICLE_FREEZE.md`](PINK_CAB_PRE_MODEL_VEHICLE_FREEZE.md);
- Tatra asset import contract: [`PINK_CAB_TATRA_ASSET_IMPORT_CONTRACT.md`](PINK_CAB_TATRA_ASSET_IMPORT_CONTRACT.md);
- Tatra handling: [`PINK_CAB_TATRA_HANDLING_E34_REFERENCE.md`](PINK_CAB_TATRA_HANDLING_E34_REFERENCE.md);
- PRE-MODEL Tatra donor/import asset contract: [`PINK_CAB_TATRA613_PREMODEL_ASSET_CONTRACT.md`](PINK_CAB_TATRA613_PREMODEL_ASSET_CONTRACT.md);
- mass/load: [`PINK_CAB_VEHICLE_MASS_LOAD_DYNAMICS.md`](PINK_CAB_VEHICLE_MASS_LOAD_DYNAMICS.md);
- taxi physicality: [`PINK_CAB_TAXI_WORK_PHYSICALITY.md`](PINK_CAB_TAXI_WORK_PHYSICALITY.md);
- passenger/Neural: [`PASSENGER_NEURAL_SYSTEM.md`](PASSENGER_NEURAL_SYSTEM.md);
- world streaming: [`WORLD_STREAMING.md`](WORLD_STREAMING.md);
- Level1: [`PINK_CAB_LEVEL1_WORLD_BIBLE.md`](PINK_CAB_LEVEL1_WORLD_BIBLE.md);
- vertical transit: [`PINK_CAB_VERTICAL_TRANSIT_PARKOUR.md`](PINK_CAB_VERTICAL_TRANSIT_PARKOUR.md);
- interchange: [`PINK_CAB_PIPE_INTERCHANGE_BIBLE.md`](PINK_CAB_PIPE_INTERCHANGE_BIBLE.md);
- ServiceNode: [`PINK_CAB_SERVICENODE_TAXONOMY.md`](PINK_CAB_SERVICENODE_TAXONOMY.md);
- moving refuel: [`MOVING_REFUELING_SERVICE.md`](MOVING_REFUELING_SERVICE.md);
- L1в†’L2 delivery: [`PINK_CAB_L1_TO_L2_ROADMAP.md`](PINK_CAB_L1_TO_L2_ROADMAP.md).

## Current QA mirrors

- vehicle stack: `qa/PINK_CAB_VEHICLE_STACK_ACCEPTANCE.md`;
- Tatra handling: `qa/PINK_CAB_TATRA_HANDLING_ACCEPTANCE.md`;
- mass/load: `qa/PINK_CAB_VEHICLE_MASS_LOAD_ACCEPTANCE.md`;
- Fare Zero: `qa/PINK_CAB_FARE_ZERO.md`;
- Level1 Corridor: `qa/PINK_CAB_LEVEL1_CORRIDOR_ZERO.md`;
- vertical: `qa/VERTICAL_TRANSIT_TESTS.md`;
- ServiceNode: `qa/PINK_CAB_SERVICENODE_ACCEPTANCE.md`;
- interchange: `qa/PINK_CAB_PIPE_INTERCHANGE_ZERO.md`;
- Level2: `qa/PINK_CAB_LEVEL2_TRANSIT_ZERO.md`.

`qa/PINK_CAB_INSURANCE_ACCEPTANCE.md` is **POST-FIRST-EURO future QA**, not a first-year gate.

## Confluence durable authorities

Primary current pages include:

- `6586369` Authority Index;
- `16744449` Control & Vehicle Mechanics Release Contract;
- `5537802` Active Product Baseline;
- `6553617` START-90 / Specification Coverage Ledger;
- `11239425` BASE-100 Code Architecture & FIRST EURO 12M Contract;
- `5832744` Open Decisions;
- `6619137` Tatra & Cockpit;
- `13303842` Native Unreal Chaos production authority (`10977288` is archived historical FGear/VDS material);
- `6225936` Tatra Handling;
- `5832724` Taxi Work/Fare;
- `6258704` Three-Tier World;
- `8388609` Vertical Transit;
- `8388630` Level1 World;
- `8421388` Pipe Interchange;
- `5931030` Level2 Transit;
- `9076737` ServiceNode;
- `6062115` Moving Refuel;
- `6651945` FIRST EURO 12M Roadmap;
- `8716289` L1в†’L2 Delivery Roadmap;
- `6750209` QA/Evidence.

Post-year pages remain valid future design authorities only where explicitly marked.

## Jira BASE-100 ownership

- `CD-746` master epic;
- `CD-747` CORE;
- `CD-748` VEHICLE;
- `CD-749` TAXI;
- `CD-750` STATE;
- `CD-751` WORLD;
- `CD-752` SERVICE;
- `CD-753` SCOPE;
- `CD-588` Open Decisions;
- `CD-673` code-only owner-question register;
- `CD-660/CD-661` readiness/governance.

Detailed existing subsystem tasks remain referenced from `AUTHORITY.yaml`.

## Legacy / salvage rule

Files still named after DEADRACE, old 8-week programs, pursuit/combat/launcher/drone-news systems, old online-within-12-month plans, superseded input mappings and old visual-scope readiness are **not PINK CAB authority** unless a current PINK CAB document explicitly re-adopts them.

Legacy files may be retained for engineering/art/reference salvage, but they must be marked historical/superseded and may not expand FIRST EURO or BASE-100.

## Source-of-truth rule

- Jira: live priority, owner decisions, dependencies, status, evidence pointers.
- Confluence: durable product/system contracts.
- Git: implementation-facing mirrors, schemas, tests, build history.
- Executable evidence: final authority for whether something is IMPLEMENTED/VERIFIED.
