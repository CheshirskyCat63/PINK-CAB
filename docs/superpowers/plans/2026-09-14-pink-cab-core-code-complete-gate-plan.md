# CD-802 Core Code-Complete Gate Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Prove the exact integrated PINK CAB Core head is code-complete through one deterministic primitive-only cross-system chain, bounded endurance evidence, and live PIE/Chaos runtime evidence.

**Architecture:** Add test-only orchestration that composes existing authoritative production owners; do not duplicate gameplay policy. Production code remains unchanged unless a RED test exposes a real missing integration boundary, in which case stop, diagnose the owner boundary, amend this plan with the smallest production API, and resume TDD.

**Tech Stack:** Unreal Engine 5.8.2, C++20, UE Automation Framework, latent PIE automation, Git worktrees.

**Spec:** `docs/superpowers/specs/2026-09-14-pink-cab-core-code-complete-gate-design.md`

## Global Constraints

- Work only in `.worktrees/cd802-core-code-complete` on `feat/CD-802-core-code-complete`.
- Test layer is orchestration-only; authoritative mutations must go through production APIs.
- Same seed + same inputs must reproduce the same integrated signature.
- Replay/capacity failures must fail closed with no partial economic or logical mutation.
- Core 100% does not mean Mechanics Freeze; post-Core Administrative / Design Reconciliation remains a separate gate.
- Automation evidence is authoritative only when `Saved/Logs/PinkCab.log` shows discovery plus `Test Completed. Result={Success}` for every discovered test.

---
### Task 1: Logical cross-system exact-head scenario

**Files:**
- Create: `Source/PinkCabTests/Private/Core/PinkCabCoreCodeCompleteScenario.h`
- Create: `Source/PinkCabTests/Private/Core/PinkCabCoreCodeCompleteAcceptanceTests.cpp`

**Interfaces:**
- Consumes: existing FareLoop, Economy, Passenger, Enforcement, Service, World, Vertical, Persistence and Recovery public APIs.
- Produces: `FCoreCodeCompleteScenario`, `FCoreCodeCompleteResult`, `RunCanonicalScenario(uint64 RootSeed, FCoreCodeCompleteResult& OutResult)`.

- [ ] **Step 1: Write RED acceptance tests** for the canonical chain: two successful fares, payment-before-exit, repeat/Neural callback, decline, off-meter evasion, enforcement/fine, ServiceNode, moving fuel, L1/L2, checkpoint/restore, replay no-ops, terminal recovery, next-workday reset.
- [ ] **Step 2: Run** `Automation RunTests PinkCab.Core.CodeComplete.Acceptance` and confirm RED is caused by missing test-only scenario implementation or a concrete production boundary, not test syntax.
- [ ] **Step 3: Implement the test-only scenario driver** by calling only production APIs; construct stable IDs and deterministic fixtures but do not calculate gameplay outcomes independently.
- [ ] **Step 4: Run** `PinkCab.Core.CodeComplete.Acceptance` and require every discovered test to report `Result={Success}`.
- [ ] **Step 5: Run regressions** `PinkCab.Taxi`, `PinkCab.Persistence`, `PinkCab.Service`, `PinkCab.World`, `PinkCab.Vertical`, and enforcement prefix; require zero failures.
- [ ] **Step 6: `git diff --check`, stage, `git diff --cached --check`, commit** `feat(CD-802): add integrated core acceptance scenario`.

If Step 2 exposes a missing production integration API, stop before GREEN, identify the authoritative owner, add the smallest exact API to this task's Files/Interfaces, write a dedicated RED assertion for it, then implement it with no policy duplication.

---
### Task 2: Determinism and bounded endurance

**Files:**
- Create: `Source/PinkCabTests/Private/Core/PinkCabCoreCodeCompleteDeterminismTests.cpp`
- Reuse: `Source/PinkCabTests/Private/Core/PinkCabCoreCodeCompleteScenario.h`

**Interfaces:**
- Consumes: `RunCanonicalScenario` and the authoritative observable result record from Task 1.
- Produces: deterministic integrated signature comparison and a fixed-iteration bounded endurance proof.

- [ ] **Step 1: Write RED determinism test** that executes fresh fixtures twice with the same root seed and asserts identical integrated signatures and stable consequence/replay IDs; execute once with a different seed and assert invariants still hold.
- [ ] **Step 2: Write RED endurance test** with a fixed iteration budget that repeatedly exercises short fare/passenger/traffic/checkpoint operations using intentionally small capacities and checks every authoritative collection remains `<=` its configured maximum.
- [ ] **Step 3: Run** `Automation RunTests PinkCab.Core.CodeComplete.Determinism` and `.Endurance`; confirm failures identify scenario/signature/capacity wiring rather than elapsed-time assumptions.
- [ ] **Step 4: Add only test-side signature/capacity observation helpers**; do not add gameplay mutations or unbounded containers.
- [ ] **Step 5: Rerun both prefixes** and require all tests to pass; rerun `PinkCab.Core`, `PinkCab.Taxi`, `PinkCab.Persistence`, and traffic/world bounded suites.
- [ ] **Step 6: VCS gate and commit** `test(CD-802): prove deterministic bounded core runtime`.

---

### Task 3: Exact-head PIE / native Chaos runtime gate

**Files:**
- Create: `Source/PinkCabTests/Private/Core/PinkCabCoreCodeCompleteRuntimeTests.cpp`
- Reuse patterns from: `Source/PinkCabTests/Private/Vehicle/PinkCabChaosRuntimeSmokeTests.cpp`
- Reuse presentation contract from: `Source/PinkCabTests/Private/World/PinkCabVerticalAcceptanceTests.cpp`

**Interfaces:**
- Consumes: `AutomationOpenMap`, latent automation commands, native Tatra pawn/dynamics provider, vertical acceptance authority marker.
- Produces: `PinkCab.Core.CodeComplete.Runtime.PIE` evidence on the same branch head.

- [ ] **Step 1: Write RED latent runtime test** that opens the existing primitive/Chaos acceptance map, finds the native Tatra pawn, applies controls, observes live telemetry movement, and verifies vertical presentation remains `NON_AUTHORITATIVE_GEOMETRY`.
- [ ] **Step 2: Run** `Automation RunTests PinkCab.Core.CodeComplete.Runtime` and confirm the RED cause is an actual runtime integration gap or missing test wiring.
- [ ] **Step 3: Implement only latent test orchestration**, reusing the existing live movement/dynamics provider; do not add a second vehicle controller.
- [ ] **Step 4: Require** `PinkCab.Core.CodeComplete.Runtime` PASS plus existing `PinkCab.Vehicle.ChaosBaseline.Runtime.DriveSmoke` PASS and `PinkCab.Vertical` PASS.
- [ ] **Step 5: VCS gate and commit** `test(CD-802): add exact-head PIE core runtime gate`.

---
### Task 4: Final exact-head verification and Core closure

**Files:**
- Modify only if needed: CD-802 Jira evidence/status; no production file changes are expected in this task.

**Interfaces:**
- Consumes: committed Tasks 1-3 on one feature head.
- Produces: verified CD-802 feature head, safe ff-only main integration, and Core 100% administrative evidence.

- [ ] **Step 1: Forced clean build** with UE 5.8.2 `Build.bat ... -Clean`, then `scripts/build.ps1`; require `Result: Succeeded`.
- [ ] **Step 2: Run final matrix**: `PinkCab.Core.CodeComplete`, `PinkCab.Core`, `PinkCab.Taxi`, `PinkCab.Persistence`, `PinkCab.Vehicle`, `PinkCab.Service`, `PinkCab.World`, `PinkCab.Vertical`, and enforcement tests; parse `PinkCab.log` for discovery/success/failure counts.
- [ ] **Step 3: Run runtime evidence** again after the clean build: dedicated CodeComplete PIE gate plus existing Chaos drive smoke.
- [ ] **Step 4: Run `git diff --check`, verify clean worktree, capture exact feature HEAD and merge-base against main.**
- [ ] **Step 5: Attach exact build/test/runtime/endurance evidence to CD-802 and set summary to `100%`; transition DONE only after all gates above are green.**
- [ ] **Step 6: Fast-forward local `main` with `git merge --ff-only feat/CD-802-core-code-complete`.**
- [ ] **Step 7: On merged `main`, repeat forced clean build and the full final matrix/runtime gate.**
- [ ] **Step 8: After merged-result verification, remove the temporary worktree/feature branch and update parent Core epic evidence/status to 100% if its workflow/fields allow it.**

Expected completion state: local `main` clean on the verified CD-802 head; CD-802 DONE/100%; Core code-complete = 100%; Administrative / Design Reconciliation becomes the next gate, not Mechanics Freeze.

---

## Plan self-review mapping

- Canonical cross-system chain: Task 1.
- Same-seed reconstruction and bounded populations/replay/checkpoints: Task 2.
- Real PIE/native Chaos and non-authoritative presentation assertion: Task 3.
- Forced clean build, complete subsystem regressions, exact-head merge verification, Jira/Core closure: Task 4.
- No task grants tests their own gameplay authority; no task treats documentation alone as acceptance.

