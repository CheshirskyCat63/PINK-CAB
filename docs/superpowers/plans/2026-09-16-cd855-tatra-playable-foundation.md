# CD-855 Tatra Playable Vehicle Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete the currently specified hero-vehicle logic and make final Tatra art replaceable without rewriting Chaos, cockpit, health, load, taxi, economy or persistence code.

**Architecture:** `APinkCabChaosTatraPawn` remains native-Chaos authority. A separate presentation profile/shell binds donor exterior/cabin assets while the physics chassis, semantic input, health/load state and persistence remain asset-independent. Locked authority is implemented; OPEN/PROPOSED owner rows are not silently promoted.

**Tech Stack:** Unreal Engine 5.8.2, UE C++, Chaos Vehicles, Automation Tests.

**Spec:** `docs/AUTHORITY.yaml`, `docs/PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md`, `docs/PINK_CAB_ACTIVE_BASELINE.md`, Confluence `6619137` for current cockpit/identity clauses, `CD-855`.

## Global Constraints
- Native Chaos is the sole production hero road-dynamics solver.
- Hero identity remains bespoke early/Gen-1 Tatra 603-family; a 613 is donor presentation only.
- CD-825 input grammar is unchanged.
- Presentation cannot alter authoritative forces/trajectory/grip/collision.
- C++ owns authoritative state/adapters; art composition stays replaceable.
- Do not implement OPEN owner rows by assumption.
- New runtime behavior follows RED → GREEN → fresh verification.

### Task 1 — Replaceable presentation seam
- [ ] Extend `FPinkCabPrototypeVisualProfile` with optional exterior/cabin donor paths, transforms and first-person visibility intent.
- [ ] RED: profile tests require physics mesh identity to remain separate from presentation paths.
- [ ] GREEN: add `UPinkCabVehicleVisualShellComponent` owning cheap exterior/cabin visual components and fallback behavior.
- [ ] Wire shell into `APinkCabChaosTatraPawn` without changing Chaos wheel/physics ownership.
- [ ] Verify focused profile/pawn tests.

### Task 2 — Authoritative load state applied to live Chaos
- [ ] RED: pawn tests require canonical crew/full-fuel load state to produce 1657 kg and runtime load changes to update Chaos mass/CG input.
- [ ] GREEN: pawn owns `FPinkCabVehicleLoadState`, exposes bounded setters/group APIs, and synchronizes movement mass/COM override only on mutation.
- [ ] Preserve `FPinkCabTatraProfile::Canonical()` as identity source; no tick recomputation.
- [ ] Verify load, pawn and persistence tests.

### Task 3 — Vehicle Health applied to live pawn
- [ ] RED: tests require pawn-owned health, authored hit application, terminal capability classification and asset-independent damage state.
- [ ] GREEN: pawn owns `FPinkCabVehicleHealthState/Service`; exposes hit/snapshot state; zero-capability fail-closed control gating only, with no invented partial degradation curves.
- [ ] Cosmetic hits never change functional controls.
- [ ] Verify health + playable input regressions.

### Task 4 — Snapshot bridge and visual independence
- [ ] RED: capture/restore tests require pawn vehicle state round-trip without serializing donor mesh identity.
- [ ] GREEN: add pawn capture/restore helpers using existing `FPinkCabVehicleSnapshotCodec`; restore reapplies mass/health to runtime atomically.
- [ ] Confirm changing presentation profile does not mutate snapshot schema/state.
- [ ] Verify persistence suite.

### Task 5 — Cockpit-ready asset substitution
- [ ] Keep all 22 stable cockpit slots and existing mesh-override/transform metadata.
- [ ] Add explicit camera/cockpit-root transforms to presentation profile application, without hardwiring donor coordinates into interaction semantics.
- [ ] Runtime test: 22/22 slots, active driver camera, donor shell/presentation component present, input contract unchanged.

### Task 6 — Evidence and integration
- [ ] Run focused `PinkCab.Vehicle`, `PinkCab.Cockpit`, `PinkCab.Persistence.VehicleSnapshot` automation.
- [ ] Run full PinkCab automation matrix.
- [ ] Run clean UE 5.8.2 PinkCabEditor build and `scripts/check-authority.sh` + `git diff --check`.
- [ ] Run playable/package smoke only after tests/build are green.
- [ ] Commit in reviewable slices and record exact evidence on CD-855; do not claim model/art completion without an actual licensed imported Tatra asset.
