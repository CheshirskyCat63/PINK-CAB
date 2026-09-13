# PINK CAB L1 / L2 Vertical Gameplay Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete primitive-only Level 1 wallride/freight/magnet gameplay and Level 2 suspended-bus/metro traversal state on top of native Chaos and the shared RoadGraph.

**Architecture:** Native Chaos remains the ordinary road solver. Vertical gameplay is a separate pure-data contact/transit layer that consumes stable RoadGraph IDs and authoritative vehicle mass, emits bounded constraint/transition requests, and never owns hidden steering/braking assists. Primitive UE presentation consumes those requests but does not own gameplay truth.

**Tech Stack:** UE 5.8.2 C++, native containers, Automation tests, existing `FPinkCabRoadGraph`, `FPinkCabVehicleLoadState`, `FPinkCabWallrideController`, CityCode/materialization infrastructure.

**Spec:** `docs/superpowers/specs/2026-09-12-pink-cab-core-gameplay-code-complete-design.md` section 5.

## Global Constraints

- Native Chaos remains the ordinary road solver; contact gameplay stays explicit and separate.
- Residual magnet timeout: `<=1657 kg = 5.0 s`; `1657–2107 kg = 5.0-(mass-1657)/450`; `2107 kg = 4.0 s`; mass above 2107 kg is rejected.
- No K11-K13 production geometry numerics are invented; route/contact identity is stable while primitive dimensions remain non-authoritative.
- Metro is not a default driveable surface.
- Collision/abort/reconstruction must not leave a stuck contact state.
- Far/world presentation never becomes authoritative gameplay state.

---
## File map

- Modify `Source/PinkCab/Public/Vehicle/PinkCabWallrideController.h`: reject unsupported mass and expose explicit reset/abort behavior.
- Create `Source/PinkCab/Public/World/PinkCabVerticalContactRegistry.h`: stable L1 wall/freight/receiving/gap hook identities bound to RoadGraph lanes.
- Create `Source/PinkCab/Public/Vehicle/PinkCabL1TraversalState.h`: deterministic L1 contact/traversal state machine.
- Create `Source/PinkCab/Public/World/PinkCabSuspendedBusRuntime.h`: bounded L2 suspended-bus route/obstacle state.
- Create `Source/PinkCab/Public/World/PinkCabMetroTransitRuntime.h`: station graph, timing and approved Tatra transition/contact state.
- Create `Source/PinkCab/Public/World/PinkCabVerticalAcceptanceCourse.h` and private implementation: primitive eyes-on acceptance presentation only.
- Add focused tests under `Source/PinkCabTests/Private/Vehicle` and `World`.

### Task 1: Authoritative residual-magnet and L1 contact identity

**Interfaces:** `FPinkCabWallrideController::TryComputeResidualTimeoutSeconds(MassKg, OutSeconds)` rejects unsupported mass. `FPinkCabVerticalContactRegistry` stores bounded stable semantic contact IDs on existing lane IDs.

- [ ] RED tests: exact 1657/1882/2107 kg timing, >2107 rejection, non-finite rejection, deterministic contact IDs, duplicate/cap rejection and reconstruction signature.
- [ ] Run build and confirm RED only on the new API/header.
- [ ] Implement `TryComputeResidualTimeoutSeconds`; keep current convenience calculation only for validated fixtures.
- [ ] Implement pure-data contact registry with kinds `WallLeft`, `WallRight`, `FreightCeiling`, `ReceivingStrip`, `PoplarGapHook`; no geometry dimensions.
- [ ] Add explicit `Abort()` reset so collision/reload cannot retain residual/contact state.
- [ ] Run `PinkCab.Vehicle.Wallride` + `PinkCab.Vertical.L1.Identity`; commit `feat(CD-798): harden vertical contact identity`.

### Task 2: Deterministic Level 1 traversal state machine

**Interfaces:** `FPinkCabL1TraversalState::Step(Input)` consumes contact kind, contact ID, mass, delta and abort/collision flags; emits immutable transition/constraint result.
- [ ] RED tests: legal road→wall→residual→reacquire, freight-ceiling contact, receiving-strip completion, gap-hook transfer, invalid cross-kind transitions and collision/abort cleanup.
- [ ] Prove all ordinary road state remains outside the contact layer and no result claims hidden force application.
- [ ] Implement bounded state with current/previous stable contact IDs and deterministic failure reason enum.
- [ ] Reuse `FPinkCabWallrideController` for residual timing rather than duplicating magnet math.
- [ ] Run `PinkCab.Vertical.L1.Traversal` + existing Vehicle regressions; commit `feat(CD-798): add level one vertical traversal state`.

### Task 3: L1 route hooks and load-fixture acceptance

**Interfaces:** L1 traversal hooks reference RoadGraph lane IDs/contact IDs only; route lookup remains owned by `FPinkCabRoadGraph`/`FPinkCabRouteService`.

- [ ] RED fixture with wallride, freight, receiving and poplar-gap hooks across light/reference/mid/max declared mass fixtures.
- [ ] Verify same CityCode/version reconstructs identical hook and traversal signatures.
- [ ] Verify every declared mass remains moving through legal contact/residual cases; unsupported mass fails closed.
- [ ] Verify detour/re-entry uses graph-valid lane transitions and no geometry numeric becomes route authority.
- [ ] Run `PinkCab.Vertical.L1` + `PinkCab.World.Routing`; commit `feat(CD-798): integrate level one route hooks`.

### Task 4: Level 2 suspended-bus runtime

**Interfaces:** `FPinkCabSuspendedBusRuntime` consumes stable route/segment IDs, positive route speed and bounded obstacle windows; it emits route phase/obstacle/contact eligibility only.

- [ ] RED tests: deterministic route progress, positive moving speed, obstacle ordering, bounded obstacle count, invalid/duplicate IDs and abort reset.
- [ ] Implement pure-data route cursor with stable tie/order behavior and no Actor ownership.
- [ ] Add approved Tatra contact eligibility states without steering/braking authority.
- [ ] Verify reconstruction gives identical route cursor/signature for identical logical state.
- [ ] Run `PinkCab.Vertical.L2.Bus`; commit `feat(CD-798): add suspended bus traversal runtime`.
### Task 5: Level 2 metro/station timing and Tatra transition state

**Interfaces:** `FPinkCabMetroTransitRuntime` owns station/segment/timing state only. Metro surfaces are non-driveable unless an explicit approved Tatra transition state is active.

- [ ] RED tests: deterministic station graph, segment timing, missed/valid transition windows, non-driveable default metro state, approved contact entry/exit and abort/reload cleanup.
- [ ] Implement stable station/segment IDs with bounded graph storage and deterministic schedule progression.
- [ ] Implement explicit Tatra states `Road`, `TransitionEligible`, `TransitContact`, `ExitPending`, `Aborted`; no implicit metro driveability.
- [ ] Verify reconstruction across identical CityCode/version reproduces station/timing/transition signatures.
- [ ] Run `PinkCab.Vertical.L2.Metro`; commit `feat(CD-798): add metro station traversal runtime`.

### Task 6: Primitive eyes-on vertical acceptance gate

**Interfaces:** one developer acceptance fixture composes RoadGraph + contact registry + L1 traversal + suspended bus + metro runtime + authoritative vehicle mass. `APinkCabVerticalAcceptanceCourse` is presentation only.

- [ ] RED/GREEN integrated tests for L1 wall/freight/receiving/gap sequence, all four authorized mass fixtures, L2 bus traversal and metro transition.
- [ ] Add built-in-primitives acceptance course/map showing separate L1 and L2 lanes/contact surfaces without canonizing production dimensions.
- [ ] Add map acceptance proving presentation IDs resolve back to logical contact/route IDs and MapCheck is clean.
- [ ] Verify collision/abort/reload resets every contact/transit owner to a legal non-stuck state.
- [ ] Fresh UE build; run `PinkCab.Vertical`, `PinkCab.World`, `PinkCab.Vehicle`, `PinkCab.Traffic` and affected Taxi regressions.
- [ ] `git diff --check`, commit final integration, attach evidence to CD-798 and transition DONE only when all gates are green.

## Self-review

- Spec coverage: residual magnet, wallride/contact bands, freight ceiling, receiving strips, detach/reacquire/failure, poplar-gap hooks, suspended bus, metro/station timing, explicit Tatra transitions and abort/reload cleanup each have an owning task.
- Ownership: RoadGraph stays topology authority; Chaos stays road physics authority; vertical systems emit state/requests only.
- Geometry: plan introduces semantic IDs and primitive acceptance presentation but no production K11-K13 dimensions.
- Optimization: all logical stores are bounded pure data; no Actor-per-contact or per-frame UObject allocations.
- Placeholder scan: no deferred implementation/TODO requirements remain.
