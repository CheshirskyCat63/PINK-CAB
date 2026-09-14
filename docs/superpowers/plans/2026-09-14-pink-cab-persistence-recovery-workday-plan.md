# PINK CAB CD-801 Persistence / Recovery / Workday Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build versioned, bounded, atomic aggregate save/recovery for all approved Core gameplay owners without creating a second gameplay authority.

**Architecture:** Each authoritative subsystem exposes a focused snapshot codec. `FPinkCabGameSnapshot` composes those snapshots and `FPinkCabGamePersistenceCoordinator` performs two-phase capture/restore against temporary owner values. `FPinkCabPersistenceService` stays responsible for envelope bytes, compatibility, committed checkpoint history and explicit recovery results.

**Tech Stack:** Unreal Engine 5.8.2 C++20, UE Automation Tests, header-only gameplay contracts, `FMemoryWriter` / `FMemoryReader`, Git worktrees.

**Spec:** `docs/superpowers/specs/2026-09-14-pink-cab-persistence-recovery-workday-design.md`

## Global Constraints

- Core gate only; no post-Core Neural/world/mechanics expansion.
- No production code before a verified failing automation test.
- Restore must validate/reconstruct every owner before mutating live state.
- Replay journals and configured capacities survive save/load and remain bounded/fail-closed.
- Passenger state reuses `FPinkCabPassengerSnapshot`; service/refuel/build reuses `FPinkCabServiceSnapshot`.
- Unknown schema/corrupt payload must fail explicitly; registered schema boundaries return `MigrationRequired`.
- Terminal recovery preserves damaged vehicle state, fails active fare, ends workday and never grants a free reset.
- Workday rollover preserves campaign/profile owners and resets only day/runtime state.

---### Task 1: Economy ledger + fare-settlement replay snapshot

**Files:**
- Create: `Source/PinkCab/Public/Persistence/PinkCabEconomySnapshot.h`
- Modify: `Source/PinkCab/Public/Economy/PinkCabEconomyLedger.h`
- Modify: `Source/PinkCab/Public/Economy/PinkCabFareSettlementService.h`
- Test: `Source/PinkCabTests/Private/Persistence/PinkCabEconomyPersistenceTests.cpp`

**Interfaces:**
- Produces `FPinkCabEconomySnapshot` with balance, debt limit, committed transaction IDs and resolved fare records.
- Produces `FPinkCabEconomySnapshotCodec::Capture(const FPinkCabEconomyLedger&, const FPinkCabFareSettlementService&, FPinkCabEconomySnapshot&)`.
- Produces `FPinkCabEconomySnapshotCodec::Restore(const FPinkCabEconomySnapshot&, FPinkCabEconomyLedger&, FPinkCabFareSettlementService&)` with atomic replacement.

- [ ] **Step 1: Write RED tests** proving paid fare/tip replay survives round-trip, evasion consequence survives, duplicate/invalid IDs reject atomically, and debt-limit/balance survive.
- [ ] **Step 2: Run** `PinkCab.Persistence.EconomySnapshot` and verify compile/test failure is caused only by missing snapshot API.
- [ ] **Step 3: Implement minimal codec**; use friend access rather than public mutation escape hatches. Validate finite/valid IDs, unique transaction IDs, unique fare IDs and known enum values before assigning outputs.
- [ ] **Step 4: Run** `PinkCab.Persistence.EconomySnapshot`, `PinkCab.Taxi.FareSettlement`, and `PinkCab.Persistence` expecting zero failures.
- [ ] **Step 5: Commit** `feat(CD-801): persist economy replay state`.

### Task 2: Active fare runtime snapshot

**Files:**
- Create: `Source/PinkCab/Public/Persistence/PinkCabFareRuntimeSnapshot.h`
- Modify: `Source/PinkCab/Public/Taxi/PinkCabFareLoopCoordinator.h`
- Modify: `Source/PinkCab/Public/Taxi/PinkCabFareSession.h`
- Modify: `Source/PinkCab/Public/Taxi/PinkCabTaximeter.h`
- Modify: `Source/PinkCab/Public/Taxi/PinkCabFarePassengerManifest.h`
- Test: `Source/PinkCabTests/Private/Persistence/PinkCabFareRuntimePersistenceTests.cpp`

**Interfaces:**
- Produces `FPinkCabFareRuntimeSnapshot` containing fare ID, loop/session state, pricing terms, meter counters/running flag, manifest records and boarded/exited state.
- Produces `Capture` / `Restore` codec that validates coherent state combinations before replacing a coordinator.

- [ ] **Step 1: RED tests** for Active and AwaitingPayment round-trip, OffMeter round-trip, boarded manifest identity/mass preservation, and impossible state-combination rejection.
- [ ] **Step 2: Verify RED** with `PinkCab.Persistence.FareRuntimeSnapshot`.
- [ ] **Step 3: GREEN** with friend codec access and one coherence validator; do not add alternate fare runtime paths.
- [ ] **Step 4: Run** new suite plus `PinkCab.Taxi.Contracts`, `PinkCab.Taxi.PassengerAcceptance` and `PinkCab.Persistence`.
- [ ] **Step 5: Commit** `feat(CD-801): persist active fare runtime`.### Task 3: Vehicle health + load snapshot

**Files:**
- Create: `Source/PinkCab/Public/Persistence/PinkCabVehicleSnapshot.h`
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabVehicleHealthState.h`
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabVehicleLoadState.h`
- Test: `Source/PinkCabTests/Private/Persistence/PinkCabVehiclePersistenceTests.cpp`

**Interfaces:**
- Produces `FPinkCabVehicleSnapshot` with all health channels, damage serial, fuel/crew/ordinary loads and active fare-passenger group.
- Produces atomic `FPinkCabVehicleSnapshotCodec::Capture/Restore`.

- [ ] **Step 1: RED tests** for damaged-state round-trip, mass/CG reconstruction equality, active fare group round-trip, invalid NaN/negative mass and invalid fare group rejection.
- [ ] **Step 2: Verify RED** with `PinkCab.Persistence.VehicleSnapshot`.
- [ ] **Step 3: GREEN** by validating all floats and bounds before assignment; preserve damage serial exactly.
- [ ] **Step 4: Run** new suite plus `PinkCab.Vehicle` and `PinkCab.Persistence`.
- [ ] **Step 5: Commit** `feat(CD-801): persist vehicle health and load`.

### Task 4: World, deterministic kernel and workday/session snapshots

**Files:**
- Create: `Source/PinkCab/Public/Persistence/PinkCabWorldSessionSnapshot.h`
- Create: `Source/PinkCab/Public/Persistence/PinkCabWorkdaySessionState.h`
- Modify: `Source/PinkCab/Public/World/PinkCabCityDeltaState.h`
- Modify: `Source/PinkCab/Public/Core/PinkCabStateKernel.h`
- Test: `Source/PinkCabTests/Private/Persistence/PinkCabWorldSessionPersistenceTests.cpp`

**Interfaces:**
- Produces `FPinkCabCityDeltaSnapshot`, `FPinkCabStateKernelSnapshot`, `FPinkCabWorkdaySessionSnapshot` and codecs.
- `FPinkCabWorkdaySessionState` exposes stable workday ID, ordinal, elapsed seconds, summary flag, household transaction count and terminal-recovery marker.
- Produces `CanAdvanceToNextWorkday()` delegating to existing `FPinkCabPersistencePolicy` semantics.

- [ ] **Step 1: RED tests** for city delta/signature round-trip, root-seed/sequence round-trip, invalid duplicate delta rejection, workday gate and workday-only reset behavior.
- [ ] **Step 2: Verify RED** with `PinkCab.Persistence.WorldSessionSnapshot`.
- [ ] **Step 3: GREEN**; reconstruct derived city lookup sets from validated records instead of persisting caches.
- [ ] **Step 4: Run** new suite plus `PinkCab.Core`, world-related automation and `PinkCab.Persistence`.
- [ ] **Step 5: Commit** `feat(CD-801): persist world and workday state`.### Task 5: Aggregate game snapshot + atomic coordinator

**Files:**
- Create: `Source/PinkCab/Public/Persistence/PinkCabGameSnapshot.h`
- Create: `Source/PinkCab/Public/Persistence/PinkCabGamePersistenceCoordinator.h`
- Test: `Source/PinkCabTests/Private/Persistence/PinkCabGameSnapshotTests.cpp`

**Interfaces:**
- `FPinkCabGameSnapshot` contains schema version, explicit `FPinkCabCityIdentity`, Passenger, Economy, FareRuntime, Vehicle, Service, CityDelta, Kernel and Workday snapshots.
- `FPinkCabGamePersistenceCoordinator::Capture(...)` gathers all owners without mutating them.
- `FPinkCabGamePersistenceCoordinator::Restore(...)` reconstructs every owner into temporaries and assigns live state only when all codecs succeed.

- [ ] **Step 1: RED tests** for full aggregate reconstruction-signature equality, explicit city identity, invalid passenger/service/economy child snapshot atomic rejection, and no partial target mutation.
- [ ] **Step 2: Verify RED** with `PinkCab.Persistence.GameSnapshot`.
- [ ] **Step 3: GREEN** aggregate codec/orchestrator; reuse Passenger/Service codecs directly and never duplicate their validation rules.
- [ ] **Step 4: Run** `PinkCab.Persistence.GameSnapshot`, `PinkCab.Taxi.PassengerSnapshot`, service snapshot tests, and `PinkCab.Persistence`.
- [ ] **Step 5: Commit** `feat(CD-801): compose atomic game snapshot`.

### Task 6: Versioned envelope, committed recovery and policy orchestration

**Files:**
- Modify: `Source/PinkCab/Public/Persistence/PinkCabPersistenceService.h`
- Create: `Source/PinkCab/Public/Persistence/PinkCabRecoveryOrchestrator.h`
- Test: `Source/PinkCabTests/Private/Persistence/PinkCabRecoveryTests.cpp`

**Interfaces:**
- Adds aggregate `Serialize/Deserialize` overloads for `FPinkCabGameSnapshot` while preserving legacy logical-save API compatibility.
- Adds selected-checkpoint recovery by bounded ring index without silently skipping corrupt/incompatible checkpoints.
- `FPinkCabRecoveryOrchestrator` applies `FPinkCabTerminalRecoveryPolicy::Canonical()` and workday rollover against reconstructed owners.

- [ ] **Step 1: RED tests** for corrupt/truncated bytes, incompatible product/config/world/schema, registered migration boundary, last-committed recovery, repeated recovery idempotence, checkpoint ring bound, terminal recovery and workday rollover.
- [ ] **Step 2: Verify RED** with `PinkCab.Persistence.Recovery`.
- [ ] **Step 3: GREEN** using immutable committed bytes; deserialize into a temporary `FPinkCabGameSnapshot`, then route through the aggregate coordinator.
- [ ] **Step 4: Run** `PinkCab.Persistence.Recovery`, all `PinkCab.Persistence`, `PinkCab.Taxi`, `PinkCab.Core`, and `PinkCab.Vehicle`.
- [ ] **Step 5: Commit** `feat(CD-801): close atomic recovery and workday`.### Task 7: Integrated CD-801 acceptance and administrative closure

**Files:**
- Create: `Source/PinkCabTests/Private/Persistence/PinkCabPersistenceIntegratedAcceptanceTests.cpp`
- Modify only if required by failing acceptance: existing persistence owner headers above.

**Acceptance flow:**
- Build representative live state: active/awaiting-payment fare, passenger social+Neural, economy credits, vehicle damage/load, service/refuel/build, world delta, deterministic kernel and workday state.
- Capture/commit aggregate snapshot; mutate live owners; recover committed snapshot; verify owner reconstruction signatures/state values.
- Retry the same fare/service/fuel/passenger operations and prove replay/duplicate/conflict rather than duplicate effects.
- Exercise terminal recovery and new-workday rollover boundaries.

- [ ] **Step 1: Write integrated RED tests** under `PinkCab.Persistence.Acceptance` covering the twelve spec scenarios, including repeated recovery and corrupt snapshot atomicity.
- [ ] **Step 2: Verify RED** is caused by missing/incomplete integration behavior only.
- [ ] **Step 3: Apply minimal integration fixes**, then rerun acceptance until green without adding new design scope.
- [ ] **Step 4: Forced clean + fresh build** using UE 5.8.2, then run `PinkCab.Persistence.Acceptance`, full `PinkCab.Persistence`, `PinkCab.Taxi`, `PinkCab.Core`, `PinkCab.Vehicle`, plus affected service/world suites; require zero failures.
- [ ] **Step 5: Run** `git diff --check`, staged diff check, commit `feat(CD-801): close persistence recovery acceptance`.
- [ ] **Step 6: Attach exact build/test/commit evidence to Jira CD-801, mark summary 100%, transition DONE, fast-forward verified branch into local `main`, rerun fresh merged-result verification, then remove only this owned worktree/feature branch.

## Plan self-review

- Spec coverage: every aggregate owner, atomic restore, versioning/migration, replay, terminal recovery, workday rollover and all 12 acceptance scenarios map to Tasks 1–7.
- Placeholder scan: no deferred `TBD`/`TODO` implementation steps; every task names concrete files, interfaces, test suites and commit gates.
- Type consistency: aggregate consumes the exact snapshot codecs produced by Tasks 1–4 plus existing `FPinkCabPassengerSnapshotCodec` and `FPinkCabServiceSnapshotCodec`; recovery consumes the aggregate from Task 5.
- Compatibility: legacy `FPinkCabLogicalSaveState` serialization remains available while CD-801 adds the aggregate path used by new Core acceptance.
