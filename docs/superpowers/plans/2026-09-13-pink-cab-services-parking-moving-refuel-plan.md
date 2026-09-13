# PINK CAB Services / Parking / Moving Refuel Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete CD-799 as a replay-safe service and moving-refuel runtime that reuses existing economy, vehicle, city and fuel owners.

**Architecture:** Service nodes coordinate lifecycle only; state mutation remains in existing authoritative owners. New bounded catalog/inventory/orchestration records provide deterministic compatibility and replay protection, while moving refuel remains a separate live-road state machine.

**Tech Stack:** Unreal Engine 5.8.2, C++20, UE Automation Tests, native Chaos contracts, existing PinkCab Economy/Vehicle/World runtime.

**Spec:** `docs/superpowers/specs/2026-09-13-pink-cab-services-parking-moving-refuel-design.md`

## Global Constraints

- Native Chaos remains ordinary vehicle authority.
- `FPinkCabEconomyLedger` alone owns balance mutation.
- `FPinkCabFuelTank` alone owns credited fuel truth.
- `FPinkCabVehicleBuild` and `FPinkCabVehicleHealthState` remain authoritative vehicle state.
- Moving refuel is not a ServiceNode.
- All collections are bounded and all replay-sensitive operations use stable ids.
- Final presentation assets are excluded.

---### Task 1: Stable service identity and owner continuity

**Files:**
- Create: `Source/PinkCab/Public/Service/PinkCabServiceContext.h`
- Modify: `Source/PinkCab/Public/Service/PinkCabServiceNode.h`
- Test: `Source/PinkCabTests/Private/Service/PinkCabServiceContextTests.cpp`

**Interfaces:**
- Consumes: `FPinkCabCityIdentity`, stable vehicle id plus addresses/identity tokens for build, health, inventory and economy owners.
- Produces: `FPinkCabServiceContext`, stable `ServiceNodeId`, `BindContext`, `HasSameOwners`, explicit abort/reset lifecycle.

- [ ] Write tests `PinkCab.Service.Context.Identity`, `.OwnerContinuity`, `.LifecycleReset` that require deterministic node ids and reject mismatched owner context.
- [ ] Run `scripts/build.ps1`; expected RED is missing `Service/PinkCabServiceContext.h` or missing new APIs.
- [ ] Implement the minimal context record and extend `FPinkCabServiceNode` without moving any authoritative state into it.
- [ ] Run `PinkCab.Service.Context` and existing `PinkCab.Service.Nodes`; require all discovered tests Success and automation exit 0.
- [ ] Run `git diff --check`; commit `feat(CD-799): add stable service context`.

---### Task 2: Bounded parts catalog and inventory

**Files:**
- Create: `Source/PinkCab/Public/Service/PinkCabPartCatalog.h`
- Create: `Source/PinkCab/Public/Service/PinkCabServiceInventory.h`
- Modify: `Source/PinkCab/Public/Service/PinkCabVehicleBuild.h`
- Test: `Source/PinkCabTests/Private/Service/PinkCabPartsInventoryTests.cpp`

**Interfaces:**
- Produces stable part definitions `{PartId, SlotId, PriceMinor, CompatibilityTag}` and bounded owned inventory with operation-id replay guards.
- `FPinkCabVehicleBuild::TryInstallPartOnce(OperationId, SlotId, PartId)` rejects incompatible slot/replay without partial mutation.

- [ ] Write tests for catalog validation, bounded inventory, purchase replay, compatible install and deterministic incompatible install rejection.
- [ ] Run build; expected RED is missing catalog/inventory APIs.
- [ ] Implement minimal bounded catalog/inventory and exactly-once install using stable ids; do not debit money here.
- [ ] Run `PinkCab.Service.Parts` plus `PinkCab.Service.Nodes.VehicleBuild`.
- [ ] Diff-check and commit `feat(CD-799): add bounded service parts inventory`.

---### Task 3: Replay-safe service operations

**Files:**
- Create: `Source/PinkCab/Public/Service/PinkCabServiceOperationRuntime.h`
- Modify: `Source/PinkCab/Public/Service/PinkCabRepairService.h`
- Test: `Source/PinkCabTests/Private/Service/PinkCabServiceOperationTests.cpp`

**Interfaces:**
- Consumes `FPinkCabEconomyLedger`, inventory/build and health owners by reference.
- Produces typed results for ParkingCharge, PartPurchase, PartInstall and Repair using stable operation ids.
- [ ] Write tests for commit-before-owner-mutation, insufficient-funds no-op, duplicate operation no second effect, and minimum-roadworthy repair policy.
- [ ] Run build; expected RED is missing `PinkCabServiceOperationRuntime.h`.
- [ ] Implement thin orchestration that commits a typed economy transaction before any owner mutation.
- [ ] Run `PinkCab.Service.Operations`, `PinkCab.Economy`, and affected vehicle-health tests.
- [ ] Diff-check and commit `feat(CD-799): add replay-safe service operations`.

---

### Task 4: Moving-refuel lifecycle

**Files:**
- Modify: `Source/PinkCab/Public/Service/PinkCabMovingFuelSession.h`
- Test: `Source/PinkCabTests/Private/Service/PinkCabMovingFuelRuntimeTests.cpp`
**Interfaces:**
- Extends the existing state machine with explicit `Complete`, `Abort`, tolerance-loss and collision paths.
- Eligibility must verify valid RoadGraph lane membership in addition to policy/telemetry/gap checks.

- [ ] Write tests for success, tolerance loss, collision, explicit abort, invalid lane, duplicate settlement and no-fuel-on-abort.
- [ ] Run build/test prefix to capture RED on the missing lifecycle/graph APIs.
- [ ] Implement the minimal bounded transitions; keep fuel credit in `FPinkCabFuelTank` and money in Economy.
- [ ] Run `PinkCab.Service.MovingFuel` and `PinkCab.World.Routing` regressions.
- [ ] Diff-check and commit `feat(CD-799): complete moving refuel lifecycle`.

---

### Task 5: Logical snapshots and replay reconstruction

**Files:**
- Create: `Source/PinkCab/Public/Service/PinkCabServiceSnapshot.h`
- Modify: service context/inventory/operation/moving-fuel headers from Tasks 1–4.
- Test: `Source/PinkCabTests/Private/Service/PinkCabServiceSnapshotTests.cpp`
**Interfaces:**
- Produces versioned logical snapshot records only; no disk I/O and no workday policy.
- Snapshot includes stable service/node ids, inventory/build operation journal and moving-refuel logical state required for deterministic reconstruction.

- [ ] Write tests for round-trip reconstruction, invalid schema/state rejection, and replay after restore without duplicate charge/fuel/part/repair effect.
- [ ] Run build; expected RED is missing `PinkCabServiceSnapshot.h`.
- [ ] Implement explicit snapshot export/import with bounded arrays and fail-closed validation.
- [ ] Run `PinkCab.Service.Snapshot`, `PinkCab.Persistence`, and `PinkCab.Economy` regressions.
- [ ] Diff-check and commit `feat(CD-799): add service snapshot boundary`.

---

### Task 6: Integrated service acceptance gate

**Files:**
- Create: `Source/PinkCabTests/Private/Service/PinkCabServiceIntegratedAcceptanceTests.cpp`
- Modify production headers only if the acceptance test exposes a contract gap.

**Interfaces:**
- One deterministic fixture links ServiceNode context, Economy, inventory/build, health, fuel, RoadGraph and telemetry without presentation ownership.
- [ ] Write integrated tests covering parking, purchase/install, repair, moving-refuel success/abort/tolerance-loss/collision and replay after reconstruction.
- [ ] Run acceptance prefix and fix only contract gaps exposed by the test.
- [ ] Run fresh UE build and final regression matrix: `PinkCab.Service`, `PinkCab.Economy`, `PinkCab.Vehicle`, `PinkCab.World`.
- [ ] Run `git diff --check` and `git diff --cached --check`; commit `feat(CD-799): close service runtime acceptance`.
- [ ] Attach exact-head evidence to CD-799 and transition Jira to DONE only if every gate is green.

## Completion accounting

- Task 1 complete: CD-799 1/6 = 16.7%, provisional core 76.7%.
- Task 2 complete: CD-799 2/6 = 33.3%, provisional core 78.3%.
- Task 3 complete: CD-799 3/6 = 50.0%, provisional core 80.0%.
- Task 4 complete: CD-799 4/6 = 66.7%, provisional core 81.7%.
- Task 5 complete: CD-799 5/6 = 83.3%, provisional core 83.3%.
- Task 6 + Jira DONE: CD-799 6/6 = 100%, official core 85%.