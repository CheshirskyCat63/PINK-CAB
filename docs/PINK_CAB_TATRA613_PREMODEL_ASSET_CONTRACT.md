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

### Controlled source custody · 2026-10-03

The selected internal source intake is now retained outside Desktop, game
`Saved` directories and temporary worktrees:

`E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\intake-37103855369\`

The source-intake step of the CD-951 acceptance run verified this set as
`SOURCE_INTAKE_VERIFIED`. Its source custody and authored-scene export checks
are separate from the subsequent worktree-restoration and studio-installation
steps. Consult CD-951 for the overall operation result; do not infer it from
this source-intake result alone.

| Retained location | Role |
| --- | --- |
| `original-donor/` | Original recovered donor archive, glTF and required external buffer, plus accompanying source/license files; original names and bytes retained. |
| `authored/TATRA613.blend` | The reviewed input to the scene-preserved export, SHA-256 `493e1caea8b672b9896ef80ccfc5042da713981d2d6400bb86cc8a9c50132dd1`. |
| `historical-working-variants/` | Preserved intermediate material from the dirty CD-855 worktree; not a new canonical game checkout. |
| `historical-export/` | The prior scene-preserved export and report, kept distinct from the new validation output. |
| `recipe-snapshot/` | Exact existing source/export/import recipes and this contract as captured from PINK-CAB commit `ecbedf4419a8b3b24f224d9ab200dd970318cd9e`. |
| `manifest.json` | Original-to-retained path mapping, source SHA-256 values, tool receipts, unresolved rights and intake result. |
| `export-contract.json` | Actual input identity, recipe revision/settings, Blender version/build and produced GLB identity. |
| `verification/` | Isolated source dependency inspection, export and GLB reimport evidence; not production game content. |

The preserved native Blender scene was inspected with automatic embedded
scripts disabled. No unpacked external Blender dependency was required by the
verified intake. The exact existing `scripts/export_tatra_scene_preserved.py`
recipe was then run from a new isolated project layout against that retained
`.blend`; the generated GLB header/length and source/runtime mesh report were
validated, and the generated GLB was independently reimported into Blender.
The authored source hash and canonical game checkout remained unchanged.

For a custom receipt directory, invoke the versioned `Blender.ps1` entrypoint
with one explicit `-OutputRoot`. The installed `BLENDER_STUDIO.cmd` convenience
wrapper already supplies `-OutputRoot`; passing it again is a parameter error.
Record the actual tool version and executable hash from each receipt rather
than assuming a Steam-managed Blender version is pinned permanently.

This accepts **source custody and the authored-blend-to-GLB transport only**.
It does not reconstruct the donor-to-authored-blend editing history, execute
or accept the game-specific Unreal importer, validate in-game scale/axes/
pivots/materials, approve final presentation, or clear modification and
redistribution rights. Those gates remain owned by CD-855. The preserved
source payload is internal: do not publish it to GitHub to make a path resolve.

The source set, recipes, provenance and recovery receipts have an indefinite
source/evidence hold under the studio retention contract. A same-drive copy
is not off-device disaster recovery. Earlier retained or failed intake
receipts are historical evidence, not competing current source authorities.

### Historical recovery locations and surviving recipes

The original intake path below is historical and was **not present** on the
studio host at the 2026-10-03 read-only recovery checkpoint. Do not treat it as
a working import command:

`C:\Users\CheCat\Downloads\tatra_613_1975-1996\scene.gltf`

The donor bytes were recovered at both of these existing locations. They are
historical source evidence, not canonical source-art checkouts; their cleanup
must follow the source-preserving disposition in CD-951:

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

The original authored scene was found at
`C:\Users\CheCat\Desktop\TATRA613.blend` (4,579,522 bytes, SHA-256
`493e1caea8b672b9896ef80ccfc5042da713981d2d6400bb86cc8a9c50132dd1`).
The initial Blender 5.2.2 LTS inspection read 136 mesh objects in background
mode with automatic scripts disabled and left the source hash unchanged.
Use the controlled `authored/` input above for the retained export contract;
source readability does not constitute model or handling acceptance.

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

The historical export was found under
`E:\CHESHIRE_DIVISION\Games\PINK-CAB\.worktrees\cd855-tatra-playable-foundation\Saved\Tatra613ScenePreserved\Tatra613_ScenePreserved.glb`
(SHA-256 `50c0a07715257ffcd504490e696855e2e9b217b373ded5f3a993bb03a0e48482`).
The corresponding main-checkout `Saved/Tatra613ScenePreserved/` export was
absent at the read-only checkpoint. Do not delete retained worktree `Saved/`
files as disposable cache or claim that game-main reimport was accepted by the
later isolated Blender-only export test.

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
