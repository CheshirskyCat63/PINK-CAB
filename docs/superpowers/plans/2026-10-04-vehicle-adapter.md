# Universal Vehicle Adapter Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace Tatra-specific runtime hardcoding with a cooked, data-driven vehicle definition plus one author/validate workflow, while preserving the current hero-car behavior.

**Architecture:** A JSON authoring manifest is compiled by Unreal Editor tooling into `UPinkCabVehicleDefinition` primary data assets. Generic runtime code converts the selected definition into the existing visual/articulation/cockpit runtime; vehicle selection is data/config driven.

**Tech Stack:** Unreal Engine 5.8 C++, UPrimaryDataAsset, Unreal Python, PowerShell, Chaos Vehicles, existing PinkCab automation.

**Spec:** `docs/superpowers/specs/2026-10-04-vehicle-adapter-design.md`

## Global Constraints

- No new pawn subclass per car.
- No Tatra asset paths or name heuristics in generic runtime after migration.
- Existing Tatra Chaos behavior and cockpit contracts remain authoritative.
- One source model per car; do not create duplicate model variants for runtime states.
- Authoring fails closed on unresolved assets, ambiguous semantic parts, invalid pivots/axes, duplicate articulation membership or missing four-wheel anchors.
- Cooked Unreal data assets own all runtime asset references.
- TDD for runtime/schema behavior; authoring scripts have contract tests and idempotent receipts.

## Review Focus

- Manifest references an asset that imports but is not cook-reachable -> definition validation must fail before delivery.
- Two articulations claim the same part -> validation must fail.
- Left/right or front/rear wheel anchors are duplicated/missing -> validation must fail.
- Re-running authoring on unchanged input -> same definition contents/receipt, no duplicate assets.
- A second non-Tatra definition -> applies without edits to pawn/runtime source.

---

### Task 1: Runtime vehicle definition contract

**Files:**
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleDefinition.h`
- Create: `Source/PinkCab/Private/Vehicle/PinkCabVehicleDefinition.cpp`
- Create: `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleDefinitionTests.cpp`

**Interfaces:**
- Produces: `UPinkCabVehicleDefinition::IsValid(FString* OutReason) const`
- Produces: `FPinkCabVehicleVisualProfile BuildVisualProfile() const`
- Produces data for physics carrier, wheels, materials, articulations, steering and driver anchors.

- [ ] Write failing tests for valid definition, duplicate part id, duplicate articulation membership, bad hinge axis, missing wheel anchor, and unresolved required soft path.
- [ ] Run targeted automation and verify RED because the definition class does not exist.
- [ ] Implement reflected structs + `UPinkCabVehicleDefinition` validation and profile conversion.
- [ ] Build editor and run targeted tests GREEN.
- [ ] Commit runtime definition contract.

### Task 2: Vehicle selection and pawn decoupling

**Files:**
- Create: `Source/PinkCab/Public/Vehicle/PinkCabVehicleSettings.h`
- Create: `Source/PinkCab/Private/Vehicle/PinkCabVehicleSettings.cpp`
- Modify: `Source/PinkCab/Public/Runtime/PinkCabChaosTatraPawn.h`
- Modify: `Source/PinkCab/Private/Runtime/PinkCabChaosTatraPawn.cpp`
- Modify: `Source/PinkCab/Private/Runtime/PinkCabChaosTatraPawnVisual.cpp`
- Test: `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleLivePawnRuntimeTests.cpp`

**Interfaces:**
- Consumes: `UPinkCabVehicleDefinition`
- Produces: selected definition from settings or command-line override.

- [ ] Write failing runtime test that applies two definitions sequentially without a pawn subclass/change.
- [ ] Verify RED on current hard-coded `Tatra613ScenePreserved()` path.
- [ ] Add settings/override loader and make pawn apply definition-derived visual/physics presentation.
- [ ] Remove vehicle-specific asset selection from pawn.
- [ ] Run runtime + existing physics/cockpit suites GREEN.
- [ ] Commit pawn decoupling.

### Task 3: Generic authoring manifest and validator

**Files:**
- Create: `Config/Vehicles/schema.vehicle.json`
- Create: `Config/Vehicles/tatra613.vehicle.json`
- Create: `Config/Vehicles/fixture.vehicle.json`
- Create: `scripts/vehicles/vehicle_manifest.py`
- Create: `scripts/tests/test_vehicle_manifest.py`

**Interfaces:**
- Produces: parsed/normalized manifest + deterministic content digest.
- Consumes no Unreal APIs, so schema tests run under normal Python.

- [ ] Write RED tests for required fields, unique semantic ids, hinge axis validity, four unique wheels, material semantic names, deterministic normalization.
- [ ] Implement parser/validator.
- [ ] Migrate current Tatra hinges/wheels/material semantics into manifest.
- [ ] Add engine/basic fixture manifest proving non-Tatra shape.
- [ ] Run Python suite GREEN.
- [ ] Commit manifest contract.

### Task 4: Unreal author command

**Files:**
- Create: `scripts/vehicles/author_vehicle.py`
- Create: `scripts/vehicles/author-vehicle.ps1`
- Create: `scripts/tests/test_author_vehicle_contract.py`
- Modify only if needed: `scripts/import_tatra_scene_preserved.py`, `scripts/author_tatra_hero_materials.py`

**Interfaces:**
- Consumes normalized manifest.
- Produces/updates `/Game/Dev/Vehicles/<slug>/DA_PC_Vehicle_<slug>` and a JSON receipt.

- [ ] Write RED contract tests for exact Unreal invocation, idempotent destination, fail-closed behavior, no delete/recreate of unrelated vehicle roots.
- [ ] Implement author script using existing import/material helpers refactored into generic functions.
- [ ] Author Tatra definition from manifest.
- [ ] Re-run authoring and verify no duplicate assets and stable digest.
- [ ] Author fixture definition through same command.
- [ ] Commit authoring workflow.

### Task 5: Migrate Tatra hero branch to generic definition

**Files:**
- Modify: `Source/PinkCab/Private/Runtime/PinkCabVehicleVisualProfile.cpp`
- Modify: `Source/PinkCab/Public/Runtime/PinkCabVehicleVisualProfile.h`
- Modify: hero/profile/runtime tests.
- Add generated Tatra data asset and receipt.

**Interfaces:**
- Consumes Tatra definition authored by Task 4.
- Removes Tatra paths, part-name assignments and material path heuristics from generic runtime C++.

- [ ] Add RED scan/test asserting generic runtime contains no `Tatra613` asset paths and no per-part Tatra leaf-name assignments.
- [ ] Switch Tatra runtime to `DA_PC_Vehicle_tatra613`.
- [ ] Preserve current 6 articulations, reverse hood, 131 material overrides, steering anchor and Manny seat transform.
- [ ] Run profile/articulation/cockpit automation GREEN.
- [ ] Commit Tatra migration.

### Task 6: Prove second-car zero-C++ onboarding

**Files:**
- Test: `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleDefinitionRuntimeTests.cpp`
- Test assets: fixture definition only.

**Interfaces:**
- Consumes fixture definition authored by Task 4.
- Produces evidence that no vehicle-specific runtime branch is required.

- [ ] Write runtime test loading Tatra definition then fixture definition through the same pawn/adapter.
- [ ] Assert definition ids, presentation part inventory, wheels and articulation state change without recompiling vehicle-specific C++.
- [ ] Run targeted automation GREEN.
- [ ] Run stale-reference/static scan proving generic runtime has no Tatra branches.
- [ ] Commit zero-C++ proof.

### Task 7: Re-run VEHICLE-HERO visual/physics gate

**Files:**
- Modify audit only if required for generic definition ids; no production behavior changes.

**Interfaces:**
- Consumes generic Tatra definition.
- Produces seven-stage render audit, physics/cockpit regression and owner-gate candidate.

- [ ] Build PinkCabEditor GREEN.
- [ ] Run targeted definition/profile/articulation/cockpit automation GREEN.
- [ ] Run 7-stage rendered Tatra audit with useful camera angles.
- [ ] Inspect closed, doors, reverse hood, rear lid, all-open, driver and hero-material frames.
- [ ] Run full project code-health + vehicle physics suite.
- [ ] Commit/review branch.

### Task 8: Integrate, package and launch owner build

**Files:** no feature expansion.

**Interfaces:**
- Consumes reviewed branch.
- Produces protected main integration, Shipping package, one desktop shortcut and running owner-gate build.

- [ ] Merge only after required checks.
- [ ] Build/package exact main.
- [ ] Verify manifest/definition assets are cooked.
- [ ] Install using existing PINK CAB delivery path; preserve rollback.
- [ ] Launch and verify responding process.
- [ ] Leave the new build running for the owner's visual gate.
