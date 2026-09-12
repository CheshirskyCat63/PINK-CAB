# PINK CAB Fare / Passenger / Economy Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete the primitive-only taxi transaction from pickup decision through boarding, distance+time fare, settlement/evasion, passenger exit, and next fare.

**Architecture:** Keep existing `FPinkCabFareSession`, `FPinkCabOrder`, seat assignment and `FPinkCabEconomyLedger`. Add small pure-C++ owners for taximeter, passenger manifest/load, exactly-once fare settlement, and a coordinator that composes them without presentation ownership.

**Tech Stack:** Unreal Engine 5.8.2 C++, Automation Tests, native containers, existing Core StableId/exactly-once contracts.

**Spec:** `docs/superpowers/specs/2026-09-12-pink-cab-core-gameplay-code-complete-design.md`

## Global Constraints

- Primitive-only gameplay; no model, animation, VFX, audio or final UI dependency.
- Native Chaos remains vehicle dynamics owner.
- Passenger group size is 1–5 and seat order is Rear1, Rear2, Rear3, Front1, Front2.
- Boarding and destination completion require `FPinkCabFullStopGate`.
- Metered fare is distance + elapsed fare time; hard pause excludes elapsed fare time.
- Off-meter uses the same loop with an explicit agreed amount, never hidden tariff math.
- Every economy mutation uses a stable transaction ID and is exactly once.

---
### Task 1: Taximeter and pricing terms

**Files:**
- Create: `Source/PinkCab/Public/Taxi/PinkCabTaximeter.h`
- Test: `Source/PinkCabTests/Private/Taxi/PinkCabFareLoopTests.cpp`

**Interfaces:**
- `FPinkCabFarePricingTerms::Metered(BaseMinor, PerKmMinor, PerMinuteMinor)`
- `FPinkCabFarePricingTerms::OffMeter(AgreedMinor)`
- `FPinkCabTaximeter::Start`, `Tick(DistanceDeltaKm, DeltaSeconds, bHardPaused)`, `Stop`, `GetFareMinor`.

- [ ] Write RED tests proving distance+time accrual, hard-pause exclusion, stop freeze, and explicit off-meter amount.
- [ ] Run `PinkCab.Taxi.FareLoop.Taximeter` and confirm RED on missing `PinkCabTaximeter.h`.
- [ ] Implement integer-minor deterministic accumulator with fractional carry; reject negative deltas.
- [ ] Run the same filter and require all tests PASS.
- [ ] Commit `feat(CD-796): add deterministic taximeter`.

### Task 2: Passenger manifest, admission and exactly-once vehicle load

**Files:**
- Create: `Source/PinkCab/Public/Taxi/PinkCabFarePassengerManifest.h`
- Modify: `Source/PinkCab/Public/Vehicle/PinkCabVehicleLoadState.h`
- Test: `Source/PinkCabTests/Private/Taxi/PinkCabFareLoopTests.cpp`

**Interfaces:**
- Manifest stores stable passenger IDs, assigned seats and deterministic masses once.
- `TryBoard(fullStop, doorOpen, LoadState)` mutates passenger load once only.
- `TryExit(fullStop, doorOpen, LoadState)` removes that fare group once only.

- [ ] Write RED tests for 1–5 seats, stop/door gates, deterministic mass, duplicate board rejection and clean exit.
- [ ] Run `PinkCab.Taxi.FareLoop.Passengers` and confirm RED.
- [ ] Add passenger group load replacement/removal without touching crew/fuel ownership.
- [ ] Implement manifest and require PASS.
- [ ] Commit `feat(CD-796): add fare passenger manifest`.
### Task 3: Fare settlement service

**Files:**
- Create: `Source/PinkCab/Public/Economy/PinkCabFareSettlementService.h`
- Test: `Source/PinkCabTests/Private/Taxi/PinkCabFareLoopTests.cpp`

**Interfaces:**
- `CommitFare(FareId, FareMinor, TipMinor, Ledger)` emits stable fare and optional tip transaction IDs.
- Replaying the same fare returns duplicate results without double credit.
- Evasion produces no fare credit and leaves a stable consequence record for the coordinator.

- [ ] Write RED tests for payment, separate tip, replay idempotency, zero-tip path and evasion no-credit path.
- [ ] Run `PinkCab.Taxi.FareLoop.Settlement` and confirm RED.
- [ ] Implement settlement using `FPinkCabTransactionId`/`FPinkCabEconomyLedger`; do not add a second ledger.
- [ ] Require settlement tests and existing `PinkCab.Economy` regression PASS.
- [ ] Commit `feat(CD-796): add exactly-once fare settlement`.

### Task 4: Complete fare loop coordinator and primitive integration gate

**Files:**
- Create: `Source/PinkCab/Public/Taxi/PinkCabFareLoopCoordinator.h`
- Test: `Source/PinkCabTests/Private/Taxi/PinkCabFareLoopTests.cpp`

**Interfaces:**
- Coordinator consumes valid `FPinkCabOrder`, pricing terms, manifest and ledger.
- Explicit transitions: Offer -> Declined or Pickup -> Boarded -> Active -> AwaitingPayment -> Paid/Evaded -> Exited -> Complete.
- `CanAcceptNextFare()` is true only after decline cleanup or completed passenger exit.

- [ ] Write RED integration tests for deliberate full-stop admit/decline, door-gated boarding, metered and off-meter paths, payment-before-exit, eligible evasion, and next-fare activation.
- [ ] Implement minimal coordinator by delegating state to existing `FPinkCabFareSession`, taximeter, manifest and settlement service.
- [ ] Run `PinkCab.Taxi.FareLoop`, `PinkCab.Taxi.Contracts`, `PinkCab.Economy` and fresh build.
- [ ] Run `git diff --check`, commit exact-head evidence, update Jira `CD-796`, and transition DONE only if all gates pass.
