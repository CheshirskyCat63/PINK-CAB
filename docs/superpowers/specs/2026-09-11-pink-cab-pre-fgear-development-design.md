# PINK CAB Pre-FGear Development Design

**Status:** APPROVED DEVELOPMENT DESIGN
**Date:** 2026-09-11
**Authority base:** `a24d697a494aa81b0639a9f3149dfc5a2f1c7308`
**Development branch:** `dev/pinkcab-pre-fgear-20260911`
**Product root:** `CD-519`
**BASE-100 program:** `CD-746`
**Scope:** FIRST EURO = PC / single-player / full L1+L2

## 1. Goal

Start real PINK CAB development with zero paid assets and progress until a production-quality hero-vehicle road-dynamics solver becomes the first objectively required external dependency. The project must reach that purchase gate without temporary replacement vehicle physics, temporary damage-deformation middleware, speculative Marketplace/Fab purchases, or programmer-invented answers to OPEN owner decisions.

The first paid dependency gate is FGear Vehicle Physics because current authority already assigns FGear sole ownership of hero-Tatra road dynamics.

## 2. Non-negotiable development rule

Before the FGear gate, implement only systems whose production architecture does not depend on a concrete vendor runtime. Use built-in Unreal primitives, debug presentation and synthetic test doubles only to verify PINK CAB-owned interfaces and logic.

Forbidden before the gate:

- no temporary Chaos Vehicle implementation for the hero Tatra;
- no custom temporary tyre/drivetrain/vehicle solver;
- no temporary deformation system intended to stand in for VDS;
- no environment, city, vehicle, audio or convenience asset purchases “for later”;
- no OPEN BASE-100 row silently promoted to implementation truth;
- no vendor API types exposed through PINK CAB gameplay contracts;
- no broad content production that depends on unresolved K11-K13 geometry numerics.

Allowed before the gate:

- Unreal built-in primitives, splines, debug shapes and developer-only widgets;
- synthetic telemetry/contact/collision events;
- deterministic mock/test providers that never ship as production solvers;
- C++ interfaces, state machines, schemas, transaction logic, deterministic generation logic and tests;
- greybox geometry explicitly marked non-authoritative where dimensions remain OPEN.

## 3. Completion condition

The pre-FGear corridor is complete when a fresh checkout can build and run the canonical development map, all vendor-independent contract/logic tests pass, and the next acceptance test requires actual authoritative hero-Tatra road dynamics through `IPinkCabVehicleDynamicsProvider`.

At that point the expected runtime state is deliberately:

`No authoritative vehicle dynamics provider installed.`

That state is not a defect. It is the purchase gate.

## 4. Architectural boundaries

### Core

PINK CAB owns stable IDs, versioned configuration, schema identity, feature configuration, build identity, logging and shared event contracts. Engine/vendor details stay outside domain contracts.

### Vehicle

Create PINK CAB-owned control, telemetry, load, fuel, health and dynamics-provider contracts. The dynamics provider has no production implementation before FGear. Tatra data may define authoritative mass/load and intended drivetrain profile, but no code may approximate FGear road motion.

### Interaction

Enhanced Input is converted into semantic commands before domain systems consume it. Current locked grammar remains `1–4 START -> LMB ATTENTION -> RMB GO`, Space gaze, mouse steering. Physical cabin controls operate through reusable interaction contracts.

### Taxi and passengers

Passenger templates, persistent identities, order state and FareSession state exist independently from presentation Actors. Implementation stops at OPEN owner-dependent transitions rather than inventing meter/receipt/abort behavior.

### Economy and persistence

Economy uses typed, stable-ID transactions and exactly-once semantics. Persistence stores versioned logical state. Save-slot, quit-anywhere, FareSession reload and terminal-recovery OPEN decisions remain boundaries, not assumptions.

### City / traffic

CityCode, deterministic chunk/road/lane identity and logical traffic exist before physical traffic. No Actor-per-lane or hero-physics-per-traffic-car architecture is allowed. Greybox L1 validates topology and interfaces only where geometry numerics remain OPEN.

### ServiceNodes and moving refueling

ServiceNodes share transition/economy/vehicle interfaces. Moving refueling owns a session state machine but does not duplicate FuelTank, Economy, VehicleTelemetry or RoadGraph. OPEN queue/gap/settlement/insufficient-funds semantics remain explicit gates.

### Testing

Every pre-FGear production-facing unit must have deterministic tests where practical. Build success is not gameplay verification. The corridor ends with a machine-readable smoke path plus contract tests and an explicit failing/blocked FGear acceptance test.

## 5. Work packages

### PF-00 · Development Bootstrap

Create the real Unreal C++ project, canonical source/config/test layout, build identity, project logging and a zero-marketplace canonical development map. This is the first package that requires access to the authorized local development machine because Unreal project generation, installed Engine detection and compilation must be verified there.

### PF-01 · Core Contracts

Stable IDs, schema/config/build identity, common results/errors/events and vendor-independent interfaces.

### PF-02 · Input + Cabin Interaction

Enhanced Input semantic routing, gaze/attention/go interaction, bounded targeting and test cabin controls.

### PF-03 · Tatra Logical Vehicle

Vehicle identity, control commands, fuel/load/passenger mass, CG/load model, profile data and telemetry schema without authoritative motion.

### PF-04 · Vehicle Health

Component health, hit zones, severity mapping, capability-based terminal classification and persistence-ready state. No deformation middleware is implemented here.

### PF-05 · Taxi Domain

Order/FareSession/PassengerTemplate/PassengerIdentity foundations up to owner-dependent edge transitions.

### PF-06 · Economy + Transactions

Typed transactions, stable TransactionId, exactly-once settlement/journal semantics and deterministic retry behavior.

### PF-07 · Persistence Foundation

Versioned save header, logical-state serialization boundaries, migration/failure APIs and deterministic incompatible-schema handling.

### PF-08 · CityCode + Road Graph

City identity, generator/content versions, deterministic chunk IDs and road/lane graph.

### PF-09 · Logical Traffic

Bounded logical/kinematic traffic records, density/headway interfaces, playable-gap invariants and materialization boundary.

### PF-10 · L1 Greybox Zero

Built-in-primitives L1 corridor proving topology, graph integration, contact surfaces and debug route. OPEN geometry values remain visibly provisional.

### PF-11 · Wallride State Logic

Detached / Contact / Residual / Expired state logic, mass-dependent residual calculation and synthetic contact tests. No custom vehicle-force solver.

### PF-12 · Automotive ServiceNodes

Common lifecycle plus Parking/Garage/Parts/Repair contracts tied to Economy, VehicleBuild and VehicleHealth.

### PF-13 · Moving Fuel Logic

Session state, connection eligibility interface, fuel transfer ledger boundaries and duplication-safe logic without choosing OPEN P03/P05/P09/P11 behavior.

### PF-14 · Enforcement + Neural Foundation

Stable EnforcementEvent plus persistent repeat-client/contact/history data foundations without broad social simulation.

### PF-15 · Pre-FGear QA / Evidence Gate

Fresh-checkout build, deterministic fixtures, smoke runner, schema/config/seed/build evidence and complete vendor-independent contract suite.

### PF-16 · FGear Purchase Gate

Add one acceptance test/specification requiring a production `IPinkCabVehicleDynamicsProvider` implementation that moves the hero Tatra and supplies normalized authoritative telemetry. No substitute implementation is permitted. This package ends with the explicit procurement decision: purchase/install FGear before implementation continues.

## 6. Dependency order

The critical path is:

`PF-00 -> PF-01 -> PF-02/PF-03 -> PF-04/PF-05/PF-06 -> PF-07 -> PF-08 -> PF-09/PF-10 -> PF-11/PF-12/PF-13/PF-14 -> PF-15 -> PF-16`.

Packages may overlap only after their consumed contracts are committed and tested. No package may consume a later package's concrete implementation to bypass an interface.

## 7. Acceptance policy

Each package closes only with:

1. exact commit SHA;
2. documented files/interfaces produced;
3. deterministic test or explicit runtime acceptance evidence;
4. no new hidden owner decision;
5. no paid dependency introduced;
6. authority mirror updated when implementation creates a new executable fact.

PF-16 is different: it is accepted when every prerequisite is green and the only remaining blocker is the missing approved production vehicle-dynamics provider.

## 8. Owner-decision containment

Current OPEN and PROPOSED DEFAULT BASE-100 rows remain unchanged by this corridor. A work package may create an interface around an OPEN question, but cannot choose the answer unless the owner explicitly locks it.

Examples:

- fare START/STOP ordering may expose state/event boundaries without choosing F05/F06;
- save persistence may serialize a FareSession without choosing I04 resume/cancel;
- L1 greybox may prove topology without canonizing K11-K13 dimensions;
- moving fuel may implement transaction primitives without locking P03/P05/P09/P11;
- vehicle health may classify terminal state without choosing I06 recovery.

## 9. Purchase policy

A paid asset/plugin is eligible for purchase only when all of the following are true:

1. an approved authority assigns it or its capability a production responsibility;
2. the PINK CAB adapter/interface boundary already exists;
3. a concrete acceptance test is blocked without that dependency;
4. no approved built-in production solution already satisfies the same responsibility;
5. the purchase is for the current execution package, not speculative inventory.

Under current authority, the first dependency satisfying this rule is FGear at PF-16.

## 10. Administrative handoff to local execution

Before PF-00 begins, remote administration must provide:

- this approved design;
- a detailed implementation plan;
- Jira program + PF work items + dependency graph;
- Confluence execution page;
- Git development branch from the final ADMIN CLEAN authority HEAD;
- explicit local-machine handoff checklist.

The first required user/local-machine interaction is then limited to confirming/using the installed Unreal development environment and allowing project generation/build verification. No asset purchase is required at that handoff.
