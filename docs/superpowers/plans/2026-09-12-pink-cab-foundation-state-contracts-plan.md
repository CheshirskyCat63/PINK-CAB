# PINK CAB Foundation / State Contracts Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete `CD-794` by giving every later gameplay subsystem one deterministic identity, event, replay and reset foundation.

**Architecture:** Keep existing `StableId`, `SchemaVersion`, `Result` and economy behavior. Add small header-only Core primitives for deterministic seeds, event envelopes, generic exactly-once replay and reset/service conventions; then make economy transaction IDs reuse Core stable-ID validation instead of maintaining a second identity rule.

**Tech Stack:** Unreal Engine 5.8.2, C++20/UE Core containers, Unreal Automation Framework.

**Spec:** `docs/superpowers/specs/2026-09-12-pink-cab-core-gameplay-code-complete-design.md`

## Global Constraints

- Native Unreal 5.8.2 only; no new third-party dependency.
- Determinism must not depend on wall-clock time, pointer values or platform hash implementation.
- Duplicate operations return the originally stored result and never execute the mutation twice.
- Reset operations must be safe to call repeatedly.
- Existing public gameplay APIs remain source-compatible where practical.
- Final art/content is outside this code-only gate.

---### Task 1: Deterministic seed contract

**Files:**
- Create: `Source/PinkCab/Public/Core/PinkCabDeterministicSeed.h`
- Modify: `Source/PinkCabTests/Private/Core/PinkCabCoreContractTests.cpp`

**Interfaces:**
- Produces: `FPinkCabDeterministicSeed::FromText(const FString&) -> uint64`
- Produces: `FPinkCabDeterministicSeed::Derive(uint64, const FPinkCabStableId&, const FString&) -> uint64`

- [ ] Write failing tests proving same input gives same seed, different domain/id changes it, and zero is avoided.
- [ ] Run `PinkCab.Core.DeterministicSeed` and verify RED because the header/type does not exist.
- [ ] Implement explicit UTF-16 FNV-1a mixing with a fixed non-zero fallback; do not call `GetTypeHash`.
- [ ] Build and run the targeted test to GREEN.
- [ ] Commit `feat(CD-794): add deterministic seed contract`.

### Task 2: Event envelope and runtime build identity

**Files:**
- Create: `Source/PinkCab/Public/Core/PinkCabEventEnvelope.h`
- Modify: `Source/PinkCab/Public/Core/PinkCabBuildIdentity.h`
- Modify: `Source/PinkCabTests/Private/Core/PinkCabCoreContractTests.cpp`
- Modify: `Source/PinkCabTests/Private/PinkCabBootstrapTests.cpp`

**Interfaces:**
- Produces: `FPinkCabEventEnvelope::Create(...)`
- Produces: `FPinkCabEventEnvelope::IsValid() const`

- [ ] Write failing tests for deterministic event metadata and for runtime identity containing `native-chaos` / no `pre-fgear` marker.
- [ ] Verify RED.
- [ ] Implement event type, event ID, correlation ID, schema, sequence and deterministic seed fields with validation.
- [ ] Update build identity to the active core-gameplay/native-Chaos channel.
- [ ] Build and run Core + Bootstrap tests to GREEN.
- [ ] Commit `feat(CD-794): add event envelope and active runtime identity`.### Task 3: Generic exactly-once replay store

**Files:**
- Create: `Source/PinkCab/Public/Core/PinkCabExactlyOnceStore.h`
- Modify: `Source/PinkCabTests/Private/Core/PinkCabCoreContractTests.cpp`

**Interfaces:**
- Produces: `EPinkCabReplayDisposition { Executed, Replayed, InvalidId }`
- Produces: `TPinkCabReplayOutcome<TResult>`
- Produces: `TPinkCabExactlyOnceStore<TResult>::Execute(const FPinkCabStableId&, Callable)`

- [ ] Write RED test where first operation mutates a counter and returns a receipt; duplicate call must return the original receipt without executing the second lambda.
- [ ] Add RED coverage for invalid IDs and store reset.
- [ ] Implement a `TMap<FString, TResult>`-backed deterministic replay store.
- [ ] Run targeted tests to GREEN and verify reset enables a fresh execution only after explicit clear.
- [ ] Commit `feat(CD-794): add exactly-once replay foundation`.

### Task 4: Shared reset / state-service convention

**Files:**
- Create: `Source/PinkCab/Public/Core/PinkCabStateService.h`
- Create: `Source/PinkCab/Public/Core/PinkCabStateKernel.h`
- Modify: `Source/PinkCabTests/Private/Core/PinkCabCoreContractTests.cpp`

**Interfaces:**
- Produces: `EPinkCabResetScope { Runtime, Workday, Profile }`
- Produces: `IPinkCabStateService::GetServiceId()` and `ResetState(EPinkCabResetScope)`
- Produces: `FPinkCabStateKernel::NextSequence()`, `GetRootSeed()`, `ResetRuntime()` and `ResetWorkday(uint64)`

- [ ] Write RED tests proving sequence generation is deterministic and reset is idempotent.
- [ ] Verify RED.
- [ ] Implement the minimal state-service interface and state kernel; resets restore canonical zero sequence and preserve or replace root seed only according to scope.
- [ ] Run targeted tests to GREEN.
- [ ] Commit `feat(CD-794): add state reset contracts`.

### Task 5: Centralize transaction identity and close gate

**Files:**
- Modify: `Source/PinkCab/Public/Economy/PinkCabTransaction.h`
- Modify: `Source/PinkCabTests/Private/Economy/PinkCabEconomyTests.cpp`
- Modify: `Source/PinkCabTests/Private/Core/PinkCabCoreContractTests.cpp`

**Interfaces:**
- `FPinkCabTransactionId` retains its public constructor / `IsValid()` / `GetValue()` API but delegates validation to `FPinkCabStableId`.

- [ ] Add RED test proving whitespace-only/newline transaction IDs follow Core stable-ID validation.
- [ ] Refactor transaction ID storage to use `FPinkCabStableId` without changing economy call sites.
- [ ] Run `PinkCab.Core` and `PinkCab.Economy` filters to GREEN.
- [ ] Run full `scripts/build.ps1` and exact-head Automation smoke.
- [ ] Run `git diff --check`, commit the gate, attach evidence to `CD-794`, and move it to DONE only on fresh clean evidence.