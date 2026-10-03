# PINK CAB · Tatra 613 PRE-MODEL Asset Contract

**Status:** CURRENT PRE-MODEL IMPLEMENTATION CONTRACT
**Gate:** CD-855 / PRE-MODEL playable vehicle foundation
**Engine:** Unreal Engine 5.8.2 / native Chaos Vehicles
**Validator:** `PinkCab.Vehicle.AssetContract.Tatra613V12`

## 1. Scope and canon boundary

This contract defines the deterministic donor/import boundary used by the current playable Tatra foundation. It does **not** reopen the higher-level hero-car canon or declare the donor archive to be the final production art direction.

Gameplay systems may depend on validated PINK CAB vehicle profiles and semantic cockpit slots. Gameplay code must not depend on donor mesh names, donor hierarchy, imported material names, or raw archive transforms.

Native Chaos remains the sole road-dynamics owner. The current Epic SportsCar skeletal mesh/physics asset is a temporary hidden Chaos carrier only; its wheel-bone geometry is not authoritative for the Tatra.

## 2. Donor source recovery and current tooling

The original intake path below is historical and is **not present** on the
studio host as of 2026-10-03. Do not treat it as a working import command:

`C:\Users\CheCat\Downloads\tatra_613_1975-1996\scene.gltf`

The donor bytes were recovered at both of these existing locations. Neither
location is a new canonical source-art checkout; retain both until the CD-855
source intake and CD-951 worktree preservation work is complete:

- `E:\Development\дедкорн\DESKTOP_SNAPSHOT\DEADRACE\ZAGLUSHKA_TATRA_613\tatra_613_1975-1996\`
- `E:\CHESHIRE_DIVISION\Games\PINK-CAB\.worktrees\cd855-tatra-playable-foundation\Saved\Temp\Tatra613_Work\gltf\`

Read-only verification on 2026-10-03 found the same SHA-256 in both copies for
the glTF document and its only external buffer. The document contains 372
meshes, 94 materials and no image entries; this check does not establish texture
completeness, license clearance or equivalence to the later authored scene.

| File | SHA-256 in both recovered copies |
| --- | --- |
| `scene.gltf` | `c3b0a12daf610607bb3ea13c315f519f61fbadd6921ad3d5887ce229f0045e17` |
| `scene.bin` | `b3f78f5d4757897b7aba90dde5c693caca4b8d96c01fa2a993f18965897f219e` |

The authored scene used by the scene-preserved export report is still present
at `C:\Users\CheCat\Desktop\TATRA613.blend` (4,579,522 bytes, SHA-256
`493e1caea8b672b9896ef80ccfc5042da713981d2d6400bb86cc8a9c50132dd1`).
Blender 5.2.2 LTS opened it in background mode with automatic scripts disabled:
136 mesh objects were read and its source hash was unchanged. That is source
readability evidence, not model or handling acceptance.

Current tracked authoring/import entrypoints are:

- `scripts/export_tatra_scene_preserved.py`: requires the reviewed authored
  `.blend` already loaded; writes `Saved/Tatra613ScenePreserved/` beside that
  script's checkout. Do not run it on the factory startup cube or substitute
  the raw 372-mesh donor for the authored scene.
- `scripts/import_tatra_scene_preserved.py`: imports that derived GLB into
  `/Game/Dev/Vehicles/Tatra613DesktopScene`. It deletes and replaces this asset
  root, so retain a recovery point and use the existing game-specific review
  and acceptance route before an import.
- `scripts/build_tatra_v12_wheel.py` and `scripts/import_tatra_v12_wheel.py`:
  the separate V12 wheel route; inspect their required environment variables
  before invoking them.

The preserved export exists under
`E:\CHESHIRE_DIVISION\Games\PINK-CAB\.worktrees\cd855-tatra-playable-foundation\Saved\Tatra613ScenePreserved\Tatra613_ScenePreserved.glb`
(SHA-256 `50c0a07715257ffcd504490e696855e2e9b217b373ded5f3a993bb03a0e48482`).
The corresponding main-checkout `Saved/Tatra613ScenePreserved/` export is
absent. Do not delete these worktree `Saved/` files as disposable cache or claim
the current main checkout can reimport without preparing the reviewed source
and output route. No donor was regenerated or reimported during this audit.

Three entrypoints named by the earlier revision are absent from the current
checkout: `scripts/import_tatra_v12_clean.py`,
`scripts/report_v12_unreal_assets.py`, and
`scripts/inspect_tatra_v12_blender.py`. They are historical references, not
available commands. Use the existing tracked routes above; do not create a
competing importer. CD-855 still owns source/provenance closure and the final
game-specific recipe acceptance.

Current scene presentation uses the scene-preserved root above together with
the **active V12Clean wheel asset** below. `PinkCabVehicleVisualProfile.cpp`
uses its `WheelPath` for all four wheels in `Tatra613ScenePreserved()`; the
asset-contract automation and `Config/DefaultGame.ini` cook rule require this
wheel subtree. It must remain available for runtime and packaged builds.

- current required wheel: `/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Wheel/StaticMeshes/Tatra613_V12_Wheel.Tatra613_V12_Wheel`

The original V12 body and steering paths below are historical context, not the
current scene-preserved body/steering route. The surviving V12 wheel tools must
not be mistaken for a complete body/steering reconstruction:

- historical body: `/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Body/StaticMeshes/Tatra613_V12_Body.Tatra613_V12_Body`
- historical steering: `/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Steering/StaticMeshes/Tatra613_V12_Steering.Tatra613_V12_Steering`

## 3. Coordinate and scale contract

PINK CAB runtime vehicle space:

- Unreal units: `1 uu = 1 cm`;
- logical vehicle forward: `+X`;
- vehicle right: `+Y`;
- up: `+Z`;
- presentation transforms must be finite and use strictly positive scale on all axes;
- negative scale / mirrored presentation is rejected;
- logical steering sign and Chaos steering sign are separate coordinate spaces; handedness conversion belongs only to the Chaos adapter boundary.

The active scene-preserved presentation is exposed through `FPinkCabVehicleVisualProfile::Tatra613ScenePreserved()` rather than by leaking source coordinates into gameplay code.

## 4. Source geometry contract

Current Tatra 613 source geometry used by the PRE-MODEL physics/model profile:

- wheelbase: `2980 mm`;
- front track: `1520 mm`;
- rear track: `1520 mm`;
- tyre: `205/70 R14`;
- derived tyre outer diameter: `642.6 mm`;
- physical wheel radius: `32.13 cm`;
- physical tyre width: `20.5 cm`.

External references checked during reconciliation on 2026-09-17: Tatra 613 technical data from German Wikipedia / EuroOldtimers for wheelbase and track, and 205/70 R14 fitment/diameter references. The executable automation contract is the implementation authority after those values are mirrored into code.

Handling values such as spring rate, damping, friction multiplier, rollbar scaling, brake torque and steering response remain calibration/design parameters. Wheelbase, track, tyre radius and tyre width are geometry/source parameters and must not be silently tuned for feel.

## 5. Wheel anchor contract

Exactly four semantic presentation wheel anchors are required:

- `WheelFL`
- `WheelFR`
- `WheelRL`
- `WheelRR`

Their centers must be unique and produce:

- `298.0 cm` wheelbase;
- `152.0 cm` front track;
- `152.0 cm` rear track.

The clean imported wheel mesh is allowed to require a positive non-uniform presentation scale. The current scale is derived from imported mesh bounds so the displayed tyre matches the source `205/70 R14` physical geometry. Hardcoded scale is acceptable only inside the vehicle visual profile; gameplay must never consume it.

## 6. Chaos physical binding

The temporary hidden skeletal carrier must expose:

- `Phys_Wheel_FL`
- `Phys_Wheel_FR`
- `Phys_Wheel_BL`
- `Phys_Wheel_BR`
- a valid physics asset.

Those carrier bones are implementation anchors only. `APinkCabChaosTatraPawn` computes each `FChaosWheelSetup::AdditionalOffset` so the actual Chaos resting wheel position follows the Tatra semantic wheel centers instead of the carrier skeleton's original SportsCar geometry.

Binding is fail-closed: if all four Tatra wheel contracts cannot be resolved, the pawn clears the Chaos wheel setups rather than running silently with incorrect geometry.

## 7. Cockpit / interaction contract

The donor steering mesh must bind through the canonical semantic slot:

`EPinkCabCockpitSlot::SteeringWheel`

All interaction semantics remain slot-driven. Donor asset names and coordinates must not become interaction IDs.

The broader cockpit retains the existing stable semantic slot contract; steering, gearbox, handbrake, pedals, meter, doors, camera and interaction behavior remain independent from donor hierarchy.

## 8. Collision ownership

For the current PRE-MODEL foundation:

- authoritative vehicle collision and Chaos simulation remain on the hidden skeletal carrier + physics asset;
- donor body/wheel/steering meshes are presentation assets;
- presentation is forbidden from silently altering forces, trajectory, grip, mass, collision ownership or save schema;
- replacement of the temporary carrier with a final vehicle-specific skeletal/physics asset must preserve this contract and pass the same validator/regression suite.

## 9. Fail-closed validation

`PinkCab.Vehicle.AssetContract.Tatra613V12` must reject at least:

- missing body, wheel or steering assets;
- degenerate/non-finite mesh bounds;
- negative/mirrored or non-finite transforms;
- missing/duplicate semantic wheel anchors;
- wheelbase/track drift from the source profile;
- physical tyre geometry that is marked as calibration instead of source authority;
- visible tyre dimensions that disagree with the physical source tyre geometry;
- missing steering semantic binding;
- missing temporary carrier physics asset or required wheel bones;
- anything other than four bound Chaos wheel setups.

Additional executable geometry proof:

`PinkCab.Vehicle.ChaosBaseline.Pawn.TatraWheelGeometryBinding`

This test verifies actual Chaos resting wheel positions, not only profile numbers.

## 10. Required regression before model-gate PASS

After any change to donor import, vehicle geometry, wheel scale, carrier skeleton, physics profile or cockpit binding, rerun at minimum:

- `PinkCab.Vehicle.AssetContract.Tatra613V12`
- `PinkCab.Vehicle.ChaosBaseline.Pawn.TatraWheelGeometryBinding`
- `PinkCab.Vehicle`
- `PinkCab.Cockpit`
- clean `PinkCabEditor Win64 Development` build

The PRE-MODEL gate is not complete from documentation or successful compilation alone; fare/persistence/economy/core/streaming acceptance must also remain green.
