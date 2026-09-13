# PINK CAB Passenger Identity / Repeat Clients / Basic Neural Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Consolidate passenger identity, relationship/history, Neural messaging, repeat ordering, and logical persistence into one bounded canonical passenger record.

**Architecture:** `FPinkCabPassengerRegistry` owns bounded `FPinkCabPassengerRecord` values keyed by stable ID. Existing Identity/History/Neural APIs remain compatibility surfaces, while new gameplay uses the registry as the authoritative social-state owner. CD-801 owns full disk recovery; CD-800 provides a validated logical passenger snapshot.

**Tech Stack:** Unreal Engine 5.8.2, C++20, UE Automation Tests, existing PinkCab StableId/DeterministicSeed/Taxi/Persistence contracts.

**Spec:** `docs/superpowers/specs/2026-09-14-pink-cab-passenger-identity-neural-design.md`

## Global Constraints

- No Passenger Actor owns persistent social state.
- All registries, memories, messages, reviews, and replay journals are bounded.
- Repeat clients use the ordinary `FPinkCabOrder` and `FPinkCabFareSession` pipeline.
- No driving or steering authority is granted to daughter/conductor hooks.
- Every production behavior is introduced through RED -> GREEN automation evidence.

---
### Task 1: Canonical passenger record and bounded registry

**Files:**
- Create: `Source/PinkCab/Public/Taxi/PinkCabPassengerRecord.h`
- Create: `Source/PinkCabTests/Private/Taxi/PinkCabPassengerRegistryTests.cpp`

**Interfaces:**
- Produces `FPinkCabPassengerRecord`, `FPinkCabPassengerRegistry`, `TryCreate(...)`, `Find(...)`, `GetReconstructionSignature()`.

- [ ] Write failing tests for stable ID/template/context validation, deterministic seed/mass/preferences, duplicate rejection, registry capacity, and reconstruction signature.
- [ ] Run `PinkCab.Taxi.PassengerRegistry` and verify RED because `PinkCabPassengerRecord.h` does not exist.
- [ ] Implement the minimal bounded record/registry using `FPinkCabDeterministicSeed` and stable IDs.
- [ ] Run fresh build plus `PinkCab.Taxi.PassengerRegistry` and `PinkCab.Taxi.Contracts`.
- [ ] Commit `feat(CD-800): add canonical passenger registry`.

### Task 2: Relationship and bounded ride memories

**Files:**
- Modify: `Source/PinkCab/Public/Taxi/PinkCabPassengerRecord.h`
- Test: `Source/PinkCabTests/Private/Taxi/PinkCabPassengerSocialTests.cpp`

**Interfaces:**
- Produces `RegisterPaidFareOnce`, `RegisterAuthoredEventOnce`, `AddRideMemoryOnce`, relationship getters, repeat eligibility, bounded memory/replay journals.

- [ ] Write RED tests for exactly-once paid fares/events, two-paid-fare promotion, authored promotion, relationship clamp/update, bounded ride-memory eviction, and duplicate memory IDs.
- [ ] Implement only the canonical mutations and bounded journals.
- [ ] Run `PinkCab.Taxi.PassengerSocial` plus `PinkCab.Taxi.NeuralFoundation` regressions.
- [ ] Commit `feat(CD-800): consolidate passenger social history`.
### Task 3: Bounded Neural contact state

**Files:**
- Modify: `Source/PinkCab/Public/Taxi/PinkCabPassengerRecord.h`
- Modify: `Source/PinkCab/Public/Taxi/PinkCabNeuralContactState.h`
- Test: `Source/PinkCabTests/Private/Taxi/PinkCabPassengerNeuralTests.cpp`

**Interfaces:**
- Produces permission/block state, `AddMessageOnce(MessageId, Text)`, bounded message history and replay journal.

- [ ] Write RED tests for permission, block/unblock, invalid identity/message, exactly-once messages, capacity limits, and Actor-independent state.
- [ ] Implement Neural as logical bounded data on the canonical record and keep the legacy contact API as an adapter-compatible surface.
- [ ] Run `PinkCab.Taxi.PassengerNeural` plus `PinkCab.Taxi.NeuralFoundation`.
- [ ] Commit `feat(CD-800): bound passenger neural state`.

### Task 4: Repeat-client ordinary order pipeline

**Files:**
- Modify: `Source/PinkCab/Public/Taxi/PinkCabPassengerRecord.h`
- Modify: `Source/PinkCab/Public/Taxi/PinkCabPassengerHistory.h`
- Test: `Source/PinkCabTests/Private/Taxi/PinkCabRepeatClientTests.cpp`

**Interfaces:**
- Produces `FPinkCabRepeatOrderFactory::Create(..., const FPinkCabPassengerRecord&, ...)` while preserving the legacy history overload.

- [ ] Write RED tests proving non-eligible records cannot create repeat orders and eligible records create ordinary valid orders using the same stable passenger ID.
- [ ] Verify the resulting order enters ordinary `FPinkCabFareSession::Boarding` with no special repeat fare runtime.
- [ ] Implement the record overload and compatibility delegation.
- [ ] Run repeat tests, `PinkCab.Taxi.Contracts`, and `PinkCab.Taxi.NeuralFoundation`.
- [ ] Commit `feat(CD-800): route repeat clients through normal fares`.
### Task 5: Logical passenger snapshot and conductor boundary

**Files:**
- Create: `Source/PinkCab/Public/Taxi/PinkCabPassengerSnapshot.h`
- Create: `Source/PinkCab/Public/Taxi/PinkCabConductorServiceHooks.h`
- Modify: `Source/PinkCab/Public/Persistence/PinkCabPersistenceService.h`
- Test: `Source/PinkCabTests/Private/Taxi/PinkCabPassengerSnapshotTests.cpp`

**Interfaces:**
- Produces versioned registry capture/restore, persistence passenger payload mapping, and bounded read/service-only conductor hooks with no driving/steering authority.

- [ ] Write RED tests for round-trip of full canonical record, atomic invalid-schema rejection, bounded restore validation, and conductor authority false.
- [ ] Implement logical snapshot codec; extend persisted passenger fields without moving disk/workday ownership out of CD-801.
- [ ] Run `PinkCab.Taxi.PassengerSnapshot`, `PinkCab.Persistence`, and relevant Vehicle load regressions.
- [ ] Commit `feat(CD-800): persist canonical passenger social state`.

### Task 6: Integrated social acceptance

**Files:**
- Create: `Source/PinkCabTests/Private/Taxi/PinkCabPassengerIntegratedAcceptanceTests.cpp`

**Interfaces:**
- Consumes all Task 1-5 APIs; produces only acceptance evidence.

- [ ] Write an end-to-end test: first paid fare -> relationship/memory update -> Actor stream-out simulation -> Neural message -> snapshot/restore -> repeat promotion -> ordinary order -> ordinary FareSession.
- [ ] Add failure coverage for duplicate events/messages, capacity limits, blocked Neural, invalid snapshot, and stream-out not deleting record.
- [ ] Run fresh build and `PinkCab.Taxi.PassengerAcceptance`.
- [ ] Run final matrix `PinkCab.Taxi`, `PinkCab.Persistence`, `PinkCab.Core` and affected `PinkCab.Vehicle` tests.
- [ ] Run `git diff --check`, staged check, commit `feat(CD-800): close passenger social acceptance`, attach evidence to Jira, transition CD-800 DONE, and fast-forward verified branch into local `main`.
