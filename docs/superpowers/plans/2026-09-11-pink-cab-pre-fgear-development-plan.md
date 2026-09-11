# PINK CAB Pre-FGear Development Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the complete vendor-independent PINK CAB production foundation and stop at the first acceptance test that objectively requires purchase/installation of FGear Vehicle Physics.

**Architecture:** PINK CAB owns semantic input, state, data, persistence, transaction, world-identity and vendor-adapter contracts. Before FGear there is intentionally no production hero-vehicle dynamics implementation; synthetic providers exist only in tests. Domain work proceeds behind stable interfaces and stops at unresolved owner decisions instead of silently choosing them.

**Tech Stack:** Unreal Engine 5.8 C++ (`A01 LOCKED`; bootstrap machine currently has 5.8.2), Enhanced Input, Unreal Automation Tests / functional smoke where appropriate, Git, Jira, Confluence. No paid assets/plugins before PF-16.

**Spec:** `docs/superpowers/specs/2026-09-11-pink-cab-pre-fgear-development-design.md`

## Global Constraints

- Authority base is commit `a24d697a494aa81b0639a9f3149dfc5a2f1c7308`; development branch is `dev/pinkcab-pre-fgear-20260911`.
- FIRST EURO remains PC / single-player / full L1+L2; no multiplayer/L3/lifestyle/insurance implementation.
- No temporary Chaos hero vehicle, custom tyre/drivetrain solver or temporary VDS replacement.
- No paid Marketplace/Fab/plugin/content purchase before PF-16.
- OPEN BASE-100 rows remain OPEN unless separately owner-locked.
- Built-in primitives/debug presentation are allowed only as explicit greybox/test surfaces.
- Vendor types may not appear in PINK CAB domain/public contracts.
- Build PASS is not gameplay VERIFIED.
- Every package closes with commit SHA + deterministic test/runtime evidence + authority impact check.

## Planned file structure

PF-00 creates only the minimal Unreal project layout. Later files are introduced by the package that owns them.

```text
PinkCab.uproject
Config/
  DefaultEngine.ini
  DefaultGame.ini
  DefaultInput.ini
Source/
  PinkCab/                         # generated bootstrap runtime module only; A03 still owner-open
    PinkCab.Build.cs
    PinkCab.cpp
    PinkCab.h
    Public/
      Core/
      Vehicle/
      Interaction/
      Taxi/
      Economy/
      Persistence/
      World/
      Traffic/
      Service/
      Enforcement/
    Private/
      Core/
      Vehicle/
      Interaction/
      Taxi/
      Economy/
      Persistence/
      World/
      Traffic/
      Service/
      Enforcement/
  PinkCabTests/
    PinkCabTests.Build.cs
    Private/
Content/
  Dev/
    Maps/L_PinkCab_ContractZero.umap
    Greybox/
Scripts/
  # exact canonical build/package entrypoint is scripts/build.ps1 under A09 LOCKED
```

The generated `PinkCab` module is a bootstrap container, not a decision that A03 is resolved. Domain folders/interfaces are allowed because they do not commit the final UE module split. After A03 is owner-locked, files may be moved into the approved modules with no domain-contract change.

---

### Task PF-00: Development Bootstrap

**Files:**
- Create locally with Unreal project generation: `PinkCab.uproject`
- Create: `Source/PinkCab/PinkCab.Build.cs`
- Create: `Source/PinkCab/PinkCab.h`
- Create: `Source/PinkCab/PinkCab.cpp`
- Create: `Source/PinkCabTests/PinkCabTests.Build.cs`
- Create: `Config/DefaultEngine.ini`
- Create: `Config/DefaultGame.ini`
- Create: `Config/DefaultInput.ini`
- Create in editor: `Content/Dev/Maps/L_PinkCab_ContractZero.umap`
- Modify only if needed for generated-project hygiene: `.gitignore`, `.gitattributes`

**Interfaces:**
- Consumes: final authority docs only.
- Produces: a compiling C++ Unreal project, one empty canonical development map, a runtime module and non-shipping test module.

**Owner gates before finalizing PF-00:**
- `A01`: LOCKED to Unreal Engine 5.8 production line; bootstrap install observed at 5.8.2.
- `A09`: canonical build/package entrypoint is owner-locked to `scripts/build.ps1`; local and CI use the same wrapper.

- [ ] **Step 1: Inventory the authorized local Unreal environment**

Record installed Unreal versions, Visual Studio/MSVC workload state, Windows SDK, Git LFS status and repository checkout path. Do not install paid plugins.

Expected evidence: a text record attached to PF-00 containing exact versions/paths; no credentials/secrets.

- [ ] **Step 2: Bind bootstrap to the owner-locked UE 5.8 production line**

Use the installed UE 5.8 line selected by the owner. Record the exact local build (currently 5.8.2) in bootstrap evidence. Do not change engine line without an explicit migration decision. `A09` is LOCKED to the repository `scripts/build.ps1` wrapper.

- [ ] **Step 3: Generate the minimal C++ project**

Create project name `PinkCab`, no Starter Content and no Marketplace content. The project must contain only the generated bootstrap module plus the test module.

- [ ] **Step 4: Add a failing smoke automation test**

Create a test in `Source/PinkCabTests/Private/PinkCabBootstrapTests.cpp` named `PinkCab.Bootstrap.ProjectIdentity` that initially fails until the runtime exposes product identity `PINK CAB` and build/schema placeholders with explicit non-empty values.

Expected first run: FAIL because project identity provider is absent.

- [ ] **Step 5: Implement the minimal bootstrap identity**

Create `Source/PinkCab/Public/Core/PinkCabBuildIdentity.h` and `Source/PinkCab/Private/Core/PinkCabBuildIdentity.cpp` with a small value type returning product name plus compile/runtime build fields. Use the owner-locked `scripts/build.ps1` boundary; do not bypass it with IDE-only build steps.

- [ ] **Step 6: Run bootstrap automation and editor launch**

Expected: automation PASS; editor opens `L_PinkCab_ContractZero`; project contains no paid plugin/content dependencies.

- [ ] **Step 7: Commit PF-00**

Commit message: `feat: bootstrap PINK CAB Unreal project`.

Acceptance: exact commit recorded in Jira; fresh local rebuild demonstrated; asset ledger still has zero paid dependencies.

---

### Task PF-01: Core Contracts

**Files:**
- Create: `Source/PinkCab/Public/Core/PinkCabStableId.h`
- Create: `Source/PinkCab/Public/Core/PinkCabSchemaVersion.h`
- Create: `Source/PinkCab/Public/Core/PinkCabResult.h`
- Create: `Source/PinkCab/Public/Core/PinkCabFeatureConfig.h`
- Create tests: `Source/PinkCabTests/Private/Core/PinkCabCoreContractTests.cpp`

**Interfaces:**
- Consumes: PF-00 build identity.
- Produces: stable logical identifiers, version values, explicit result/error vocabulary and feature-config read boundary.

**Owner containment:** `A07` feature-flag set remains OPEN; implement the container/API without locking which flags ship.

- [ ] Write failing tests for stable ID equality/serialization round-trip, schema-version comparison and explicit invalid-version rejection.
- [ ] Run only `PinkCab.Core.*`; verify expected FAIL before implementation.
- [ ] Implement minimal value types with deterministic serialization and no UObject/Actor identity dependency.
- [ ] Add a feature-config container that can expose named flags but ships with no owner-unapproved production flags enabled.
- [ ] Run `PinkCab.Core.*`; expected PASS.
- [ ] Commit: `feat: add PINK CAB core contracts`.

Acceptance: public contracts compile without any vendor plugin include.

---

### Task PF-02: Input and Cabin Interaction Contracts

**Files:**
- Create: `Source/PinkCab/Public/Interaction/PinkCabSemanticCommand.h`
- Create: `Source/PinkCab/Public/Interaction/PinkCabInteractionTarget.h`
- Create: `Source/PinkCab/Public/Interaction/PinkCabInteractionRouter.h`
- Create implementation files under `Private/Interaction/`
- Create tests: `Source/PinkCabTests/Private/Interaction/PinkCabInteractionTests.cpp`

**Interfaces:**
- Consumes: Core IDs/results.
- Produces: semantic continuous vehicle controls plus gaze/attention/go interaction commands and bounded target-selection contract.

- [ ] Write tests proving locked grammar maps `1-4`, LMB, RMB and Space into semantic commands without invoking vehicle/vendor code.
- [ ] Write tests proving continuous steering/throttle/brake/clutch/handbrake values remain normalized.
- [ ] Implement router interfaces and bounded current-target selection.
- [ ] Build a test cabin primitive control in `L_PinkCab_ContractZero` that changes developer-visible state through the interaction interface.
- [ ] Run automation + manual contract-zero interaction acceptance.
- [ ] Commit: `feat: add semantic cabin interaction foundation`.

Acceptance: no hidden cabin shortcut behavior is invented; `C08` rebinding remains an explicit owner boundary.

---

### Task PF-03: Tatra Logical Vehicle Contracts

**Files:**
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleControlState.h`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleTelemetry.h`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleDynamicsProvider.h`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabTatraProfile.h`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleLoadState.h`
- Create tests: `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleContractTests.cpp`

**Interfaces:**
- Produces `IPinkCabVehicleDynamicsProvider` as the sole future road-dynamics boundary. Production implementation is intentionally absent until FGear.
- Produces normalized telemetry free of FGear types.

- [ ] Write failing tests for canonical mass cases: base 1450 kg, full-fuel vehicle 1550 kg, heroine 58 kg, daughter 49 kg, reference total 1657 kg and declared maximum fixture 2107 kg.
- [ ] Write tests proving passenger/fuel load modifies authoritative mass and CG input data rather than presentation only.
- [ ] Define normalized control/telemetry structs and provider interface with no vendor headers.
- [ ] Implement logical load calculation and explicit `NoProvider` result when production dynamics provider is absent.
- [ ] Run `PinkCab.Vehicle.Contracts.*`; expected PASS while road-motion acceptance remains intentionally unavailable.
- [ ] Commit: `feat: add vendor-independent Tatra vehicle contracts`.

---

### Task PF-04: Vehicle Health

**Files:**
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleHealthState.h`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleHitEvent.h`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleHealthService.h`
- Create implementation under `Private/Vehicle/`
- Create tests: `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleHealthTests.cpp`

**Interfaces:**
- Consumes authored hit-zone/severity events.
- Produces component health and capability classification; does not own visual deformation.

- [ ] Write failing tests for wheel, alignment, suspension, brake, door, lamp and air-cooled V8 oil/fan/oil-cooler/airflow health updates.
- [ ] Write terminal-classifier tests proving failure is capability-based and not a universal HP scalar.
- [ ] Implement deterministic synthetic hit mapping sufficient for contract tests; authored tuning remains data-driven.
- [ ] Verify cosmetic-only event can produce no handling-health consequence.
- [ ] Commit: `feat: add component vehicle health foundation`.

Acceptance: no VDS substitute/deformation implementation exists.

---

### Task PF-05: Taxi and Passenger Foundations

**Files:**
- Create: `Source/PinkCab/Public/Taxi/PinkCabFareSession.h`
- Create: `Source/PinkCab/Public/Taxi/PinkCabOrder.h`
- Create: `Source/PinkCab/Public/Taxi/PinkCabPassengerTemplate.h`
- Create: `Source/PinkCab/Public/Taxi/PinkCabPassengerIdentity.h`
- Create tests: `Source/PinkCabTests/Private/Taxi/PinkCabTaxiContractTests.cpp`

**Interfaces:**
- Produces logical FareSession/Order/passenger identity state independent of materialized Actors.

- [ ] Test PassengerTemplate and persistent PassengerIdentity separation.
- [ ] Test passenger group size 1-5 and deterministic rear3-then-front2 seat assignment.
- [ ] Test one active FareSession invariant without deciding F12 owner behavior beyond preventing a second authoritative active session.
- [ ] Implement state definitions only through owner-independent transitions.
- [ ] Add explicit blocked transition results for F05/F06/F11-F19 dependent behavior rather than choosing defaults.
- [ ] Commit: `feat: add taxi and passenger logical foundation`.

---

### Task PF-06: Economy and Exactly-Once Transactions

**Files:**
- Create: `Source/PinkCab/Public/Economy/PinkCabTransaction.h`
- Create: `Source/PinkCab/Public/Economy/PinkCabEconomyLedger.h`
- Create implementation under `Private/Economy/`
- Create tests: `Source/PinkCabTests/Private/Economy/PinkCabEconomyTests.cpp`

**Interfaces:**
- Produces typed transactions and stable TransactionId idempotency.

- [ ] Write a duplicate TransactionId test that must settle balance once.
- [ ] Cover FareIncome, Tip, FuelPurchase, PartPurchase, Repair, Parking and Fine transaction types without choosing H04/H05 debt behavior.
- [ ] Implement ledger append/retry semantics and explicit `InsufficientFundsPolicyRequired` boundary where H04/H05 is needed.
- [ ] Test crash/retry simulation by replaying the same transaction journal input.
- [ ] Commit: `feat: add exactly-once economy ledger`.

---

### Task PF-07: Persistence Foundation

**Files:**
- Create: `Source/PinkCab/Public/Persistence/PinkCabSaveHeader.h`
- Create: `Source/PinkCab/Public/Persistence/PinkCabPersistenceService.h`
- Create: `Source/PinkCab/Public/Persistence/PinkCabMigrationRegistry.h`
- Create tests: `Source/PinkCabTests/Private/Persistence/PinkCabPersistenceTests.cpp`

**Interfaces:**
- Consumes logical state snapshots from earlier domains.
- Produces versioned serialization/migration/failure boundary without choosing save-slot/quit policy.

- [ ] Test save header contains product/schema/config/world-version identity.
- [ ] Test compatible synthetic state round-trip for vehicle health, passenger identity and economy ledger.
- [ ] Test incompatible schema fails explicitly instead of silently guessing.
- [ ] Leave I01-I04/I06/I08/I09/I11-I13 as policy inputs/interfaces; do not code outcomes.
- [ ] Commit: `feat: add versioned persistence foundation`.

---

### Task PF-08: CityCode and Road Graph

**Files:**
- Create: `Source/PinkCab/Public/World/PinkCabCityIdentity.h`
- Create: `Source/PinkCab/Public/World/PinkCabChunkId.h`
- Create: `Source/PinkCab/Public/World/PinkCabRoadGraph.h`
- Create tests: `Source/PinkCabTests/Private/World/PinkCabCityCodeTests.cpp`

**Interfaces:**
- Produces deterministic CityCode/GeneratorVersion/ContentSetVersion identity and logical lane/road graph.

- [ ] Write deterministic same-input/same-ID tests across repeated runs.
- [ ] Implement chunk/road/lane stable IDs without Actor identity.
- [ ] Prove unloaded logical graph still resolves route adjacency.
- [ ] Do not choose J05/J07/J09/J11/J12 policy/numeric envelopes.
- [ ] Commit: `feat: add deterministic CityCode road graph foundation`.

---

### Task PF-09: Logical Traffic

**Files:**
- Create: `Source/PinkCab/Public/Traffic/PinkCabTrafficEntity.h`
- Create: `Source/PinkCab/Public/Traffic/PinkCabTrafficFlow.h`
- Create: `Source/PinkCab/Public/Traffic/PinkCabTrafficMaterializationPolicy.h`
- Create tests: `Source/PinkCabTests/Private/Traffic/PinkCabTrafficTests.cpp`

**Interfaces:**
- Consumes RoadGraph.
- Produces bounded logical traffic and a future physical-materialization request boundary.

- [ ] Test no Actor is required for logical lane occupancy progression.
- [ ] Test playable-gap invariant exists as an explicit flow constraint.
- [ ] Test no designed systemic standing-jam state is generated.
- [ ] Leave K04/K06/K08 exact policy owner-open; APIs accept profiles without embedding a chosen default.
- [ ] Commit: `feat: add bounded logical traffic foundation`.

---

### Task PF-10: Level 1 Greybox Zero

**Files:**
- Create in editor: `Content/Dev/Maps/L_PinkCab_L1_GreyboxZero.umap`
- Create developer-only greybox components/assets under `Content/Dev/Greybox/`
- Create: `Source/PinkCab/Public/World/PinkCabGreyboxAuthorityTag.h`
- Create tests: `Source/PinkCabTests/Private/World/PinkCabL1GreyboxTests.cpp`

**Interfaces:**
- Consumes RoadGraph.
- Produces a topology/contact/debug test environment, not canonical K11-K13 geometry.

- [ ] Add explicit `NON_AUTHORITATIVE_GEOMETRY` marker to greybox metadata/debug display.
- [ ] Build a straight corridor with enough primitive lanes/surfaces to validate route/lane/contact integration without canonizing dimensions.
- [ ] Add automation/functional checks that map graph IDs resolve to greybox lane surfaces.
- [ ] Verify no purchased content dependencies.
- [ ] Commit: `feat: add L1 contract greybox zero`.

---

### Task PF-11: Wallride State Logic

**Files:**
- Create: `Source/PinkCab/Public/Vehicle/PinkCabWallrideState.h`
- Create: `Source/PinkCab/Public/Vehicle/PinkCabWallrideController.h`
- Create tests: `Source/PinkCabTests/Private/Vehicle/PinkCabWallrideTests.cpp`

**Interfaces:**
- Consumes synthetic contact + authoritative vehicle mass.
- Produces Detached/Contact/Residual/Expired state and a future external-force request; does not apply custom road dynamics.

- [ ] Test residual maximum 5.0 s at 1657 kg and 4.0 s at 2107 kg with linear interpolation and lighter legal cap at 5.0 s.
- [ ] Test contact loss consumes residual countdown.
- [ ] Expose M05 reacquisition behavior as unresolved strategy input; do not choose reset semantics.
- [ ] Verify no force application occurs without `IPinkCabVehicleDynamicsProvider`.
- [ ] Commit: `feat: add wallride state foundation`.

---

### Task PF-12: Automotive ServiceNodes

**Files:**
- Create: `Source/PinkCab/Public/Service/PinkCabServiceNode.h`
- Create: `Source/PinkCab/Public/Service/PinkCabVehicleBuild.h`
- Create tests: `Source/PinkCabTests/Private/Service/PinkCabServiceNodeTests.cpp`

**Interfaces:**
- Produces common ServiceNode lifecycle and vehicle-build/repair transaction requests.

- [ ] Test Approach -> Eligible -> Enter -> NodeActive -> Commit -> Exit -> ResumeWorld abstract lifecycle.
- [ ] Test Repair consumes VehicleHealth state and requests Economy transaction rather than owning balance.
- [ ] Test installed-part effect path ends at vehicle profile/adapter rather than vendor API.
- [ ] Leave O02/O03 implementation/persistence responsibilities owner-open.
- [ ] Commit: `feat: add automotive ServiceNode contracts`.

---

### Task PF-13: Moving Refueling Logic

**Files:**
- Create: `Source/PinkCab/Public/Service/PinkCabFuelTank.h`
- Create: `Source/PinkCab/Public/Service/PinkCabMovingFuelSession.h`
- Create tests: `Source/PinkCabTests/Private/Service/PinkCabMovingFuelTests.cpp`

**Interfaces:**
- Consumes FuelTank, Economy, normalized VehicleTelemetry and RoadGraph.
- Produces session state and fuel-credit boundary.

- [ ] Test fuel credit occurs only in valid Connected/Fueling state.
- [ ] Test retry with same transaction identity cannot duplicate fuel.
- [ ] Expose queue, gap tolerance, settlement and insufficient-funds strategies as explicit inputs for P03/P05/P09/P11.
- [ ] Do not choose ±25 cm as a production threshold.
- [ ] Commit: `feat: add moving refuel transaction foundation`.

---

### Task PF-14: Enforcement and Basic Neural Foundations

**Files:**
- Create: `Source/PinkCab/Public/Enforcement/PinkCabEnforcementEvent.h`
- Create: `Source/PinkCab/Public/Taxi/PinkCabPassengerHistory.h`
- Create: `Source/PinkCab/Public/Taxi/PinkCabNeuralContactState.h`
- Create tests under matching `PinkCabTests/Private/` folders.

**Interfaces:**
- Produces stable EnforcementEvent and persistent contact/history data; no full Regulator or social simulation.

- [ ] Test EnforcementEvent stable ID + producer-independent serialization.
- [ ] Test persistent PassengerIdentity/history survives absence/destruction of materialized Actor.
- [ ] Test repeat order references the normal Order/FareSession contract.
- [ ] Leave G02/G04 and L05-L07 owner-open.
- [ ] Commit: `feat: add enforcement and repeat-client data foundation`.

---

### Task PF-15: Pre-FGear QA and Evidence Gate

**Files:**
- Create: `docs/qa/PINK_CAB_PRE_FGEAR_ACCEPTANCE.md`
- Create: `Source/PinkCabTests/Private/PinkCabPreFGearSuite.cpp` if a suite aggregator is required by the pinned engine version.
- Modify: authority/evidence mirrors only with executable facts proven by this milestone.

**Interfaces:**
- Consumes every PF-00..PF-14 artifact.
- Produces the release-quality evidence bundle for the purchase gate.

- [ ] Run fresh-checkout project generation/build through the owner-approved `scripts/build.ps1` entrypoint and record exact output/evidence.
- [ ] Run all `PinkCab.*` automation tests and record zero failures.
- [ ] Launch canonical contract map and L1 Greybox Zero; record exact commit, engine version, config/schema versions and test seed(s).
- [ ] Search production source for FGear/VDS/Chaos hero-vehicle includes and verify none are present before purchase.
- [ ] Verify asset license ledger contains no paid dependency introduced by PRE-FGEAR.
- [ ] Commit: `test: certify PINK CAB pre-FGear foundation`.

Acceptance: all vendor-independent systems are green or explicitly blocked only by still-open owner policy rows; no hidden implementation guess is permitted.

---

### Task PF-16: FGear Purchase Gate

**Files:**
- Create: `docs/qa/PINK_CAB_FGEAR_ENTRY_GATE.md`
- Create test specification/source: `Source/PinkCabTests/Private/Vehicle/PinkCabFGearEntryGateTests.cpp`
- Do not create a fake production provider.

**Interfaces:**
- Consumes `IPinkCabVehicleDynamicsProvider`, Tatra profile, normalized controls and telemetry.
- Requires next: an approved production FGear-backed provider.

- [ ] Add an entry-gate test that requires a production dynamics provider to accept normalized steering/throttle/brake/clutch/handbrake commands and return authoritative speed, pose/motion and required normalized telemetry for the hero Tatra.
- [ ] Verify the test is BLOCKED/RED specifically because no production dynamics provider is installed; it must not fail because PF-00..PF-15 infrastructure is broken.
- [ ] Verify current authority still assigns sole hero road-dynamics ownership to FGear.
- [ ] Record procurement request: **FGear Vehicle Physics is now required.**
- [ ] Stop implementation. Do not install a substitute.

**Purchase gate acceptance:** the PINK CAB adapter is ready; all prerequisites are green; the next test cannot become green without installing the approved FGear production dependency.

---

## Administrative tracking rules

- Jira has one PRE-FGEAR program issue plus PF-00..PF-16 work items.
- `Blocks` links encode the critical dependency chain; parallel packages use `Relates` only where useful.
- Every PF issue records authority rows it must not silently resolve.
- Confluence has one durable PRE-FGEAR execution page linking the spec, plan, Jira program and development branch.
- Git is authoritative for implementation-facing plan/spec; Jira is authoritative for execution status/evidence; Confluence is durable narrative/index authority.
- BASE-100 readiness is not increased merely by writing this plan. Only explicit owner locks change specification readiness; implementation evidence changes IMPLEMENTED/VERIFIED state separately.

## Local-machine handoff checkpoint

Remote administrative preparation is complete before PF-00 code starts. The first local interaction must collect only the information needed to begin PF-00 safely:

1. authorized checkout path/device;
2. installed Unreal Engine version(s);
3. Visual Studio/MSVC + Windows SDK availability;
4. Git/Git LFS availability;
5. whether the machine can generate and compile a minimal C++ Unreal project.

No paid asset/plugin is required at this checkpoint.
