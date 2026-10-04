# PINK CAB Universal Vehicle Adapter Design

## Goal

Make the current Tatra 613 the first data-driven vehicle definition so later cars can be admitted with one manifest + one author/validate command, without new pawn subclasses, hard-coded mesh paths, per-car C++ material switches, or duplicated vehicle systems.

## Non-negotiables

- One generic Chaos/cockpit/interaction runtime remains authoritative.
- A new car must not require editing the pawn, articulation component, cockpit runtime, or material runtime.
- Existing Tatra physics, steering, cockpit controls and presentation behavior remain the regression baseline.
- Source models are never duplicated just to make runtime variants.
- Vehicle-specific data lives outside generic C++.
- Missing/ambiguous pivots, wheel anchors, required assets or duplicate semantic bindings fail closed.
- Cooker-visible asset references must be held by a cooked Unreal asset; runtime JSON-only references are not sufficient.
- Debug controls may call the same generic articulation API, but they are not the production interaction architecture.

## Architecture

### 1. Canonical authoring manifest

Each vehicle has one human-editable manifest under `Config/Vehicles/<slug>.vehicle.json`. It records:
- stable vehicle id and source provenance;
- imported content root;
- physics carrier mesh + physics asset;
- four Chaos wheel bones and four visual wheel part ids;
- presentation root transform;
- semantic material palette and optional per-part overrides;
- part list or discovery rules;
- articulations: id, pivot, axis, angle, travel time, member parts;
- steering part + pivot/axis;
- driver seat/head transform and optional hand/foot targets;
- cockpit bindings.

Tatra becomes `Config/Vehicles/tatra613.vehicle.json`; its current validated hinges (DoorFL/FR/RL/RR, FrontLid reverse hinge, RearLid), wheel positions and material semantics are migrated verbatim.

### 2. Cooked runtime definition

Create `UPinkCabVehicleDefinition : UPrimaryDataAsset`. It stores soft references to imported meshes/materials plus all validated transforms and semantics. This gives the cooker an explicit dependency graph and removes Tatra paths from generic runtime code.

The existing `FPinkCabVehicleVisualProfile` and articulation runtime remain the low-level runtime representation. A generic adapter converts one `UPinkCabVehicleDefinition` into the existing runtime profile; the pawn consumes only the definition/adapter.

### 3. One author/validate command

`scripts/vehicles/author_vehicle.py` is the editor-side adapter. Given a manifest it:
1. validates schema and source identity;
2. imports/reuses the scene under the manifest content root;
3. resolves semantic parts and materials;
4. creates/updates generic hero materials only where the manifest asks for defaults;
5. authors/updates one `DA_PC_Vehicle_<slug>` asset;
6. validates referenced assets, unique parts, hinge axes/pivots, wheel anchors, steering/driver anchors and material coverage;
7. writes a machine-readable receipt.

A PowerShell wrapper `scripts/vehicles/author-vehicle.ps1` launches the exact UnrealEditor-Cmd invocation. Re-running is idempotent.

### 4. Selection

`UPinkCabVehicleSettings` exposes a soft reference to the default definition, configured in `DefaultGame.ini`. A command-line override can select another definition for development/audits. Swapping the car means selecting a different data asset, not compiling a different pawn.

### 5. Driver and cockpit

Manny stays the placeholder driver. The definition supplies the driver seat/head transform and steering anchor. Existing cockpit controls remain semantic and generic. The first version keeps the proven pose system but removes Tatra coordinate literals from the pawn; future IK/animation assets can replace the placeholder without changing the vehicle definition contract.

### 6. Materials

Replace `HeroMaterialForMeshPath` with semantic material slots stored in the vehicle definition: BodyPaint, Chrome, Glass, RubberPlastic, InteriorVinyl, Fabric, Mirror, LightWhite, LightRed, LightAmber. Tatra's current materials become the first palette. New vehicles inherit defaults and may override any slot in the manifest.

## Onboarding flow for the next car

1. Put the source model in `SourceAssets/Vehicles/<slug>/`.
2. Copy the manifest template and specify source/content root plus semantic anchors.
3. Run one author command.
4. Fix only validation errors reported by the receipt (usually object names/pivots/material semantics).
5. Select the produced `DA_PC_Vehicle_<slug>` and run the generic vehicle audit.
6. No C++ changes are expected.

## Proof that the abstraction is real

A test fixture vehicle using engine/basic meshes is authored from a second manifest and loaded through the same adapter. CI asserts that the generic runtime contains no Tatra asset paths and that both the Tatra and fixture definition validate/apply without vehicle-specific C++ branches.

## Current VEHICLE-HERO handoff

The existing in-progress Tatra materials, articulation component, reverse hood correction, Manny presentation and visual audit are preserved. Migration to the definition system happens before merging the hero-car branch, so the shipped Tatra itself proves the generic path.
