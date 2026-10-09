# VF90 Task 2 — road shimmer diagnosis and package evidence — 2026-10-07

Status: **TECHNICALLY VERIFIED; administrative convergence pending at creation time.**

Scope is only Task 2 from `docs/superpowers/plans/2026-10-07-tatra-ready.md`: isolate road shimmer in exact-package evidence, change one demonstrated cause, and preserve road contact topology and friction. This receipt does not accept Task 3+, vehicle handling, final visuals, or a replacement installed package.

## Identities

- Task-1 administrative close: `453862440e971361d22a7a50ced6e9794ce39f90`.
- Task-2 implementation candidate: `21911223a047987a175d14947b6d820e33b9b47f`.
- Runtime branch: `feat/tatra-ready-runtime`.
- Installed owner-accepted fallback remains `6edea7747d3a8433188c9fb394b98ae9c320d49b`; Desktop delivery was not replaced.
- Candidate road-mark material: `/Game/World/L1/Road/M_PC_RoadMarkSurface.M_PC_RoadMarkSurface`.
- Candidate material asset SHA-256: `AC9B6833BB8976DD2DACF36079EE2ECA2B628AEB2739BC3A72481133033D172C`.

## Diagnosis

The accepted package uses the MetaRoad 3.2 road-mark asset `/MetaRoad/MetaRoad/Materials/M_Mark.M_Mark` on 50 actual `UStaticMeshComponent` road-mark strips.

Fresh UE inspection proved:

- vendor `M_Mark`: material domain `MD_DEFERRED_DECAL`, blend `BLEND_TRANSLUCENT`;
- candidate `M_PC_RoadMarkSurface`: material domain `MD_SURFACE`, blend `BLEND_OPAQUE`;
- road surface mesh is a flat Z=0 plane;
- road-mark meshes have authored/runtime Z separation of 3.0 cm;
- therefore coplanar road/mark z-fighting is not the demonstrated cause.

Two exact-package renderer A/B checks were deliberately rejected as fixes:

- `r.Streaming.MipBias=1` did not reduce measured road high-frequency energy;
- `r.MaxAnisotropy=16` did not reduce measured road high-frequency energy.

No global mip bias, anisotropy override, blur, geometry lift, collision change, or friction change is retained.

The demonstrated defect is the use of a DeferredDecal/Translucent material as the material of real mesh-strip markings. Task 2 changes only that render-domain mismatch by binding a project-owned Surface/Opaque road-mark material while retaining the same mark geometry and authored 3 cm separation.

## Runtime regression gate

Exact candidate `2191122`:

- build: PASS;
- targeted Task-2 runtime gate: **10/10 PASS**:
  - `PinkCab.World.L1EndlessRoad.Runtime.ChunkBinding`;
  - `PinkCab.World.L1EndlessRoad.Runtime.MarkSurfaceMaterial`;
  - `PinkCab.World.L1EndlessRoad.Runtime.ChunkValidation`;
  - `PinkCab.World.L1EndlessRoad.Streamer.DirectionWindow`;
  - `PinkCab.World.L1EndlessRoad.Streamer.BoundedLongRun`;
  - `PinkCab.World.L1EndlessRoad.Streamer.MaterializationPolicy`;
  - `PinkCab.World.L1EndlessRoad.Physics.SeamCrossing`;
  - `PinkCab.Vehicle.ChaosBaseline.PhysicsOnly.DriveSmoke`;
  - `PinkCab.Vehicle.ChaosBaseline.PhysicsOnly.ReverseDriveSmoke`;
  - `PinkCab.Vehicle.Visual.TatraWheelContacts`.
- standard focused runtime gate: **9/9 PASS**.
- all 50 native road-mark meshes bind the project Surface/Opaque material;
- mark collision remains `NoCollision`;
- mark Z remains 3.0 cm;
- road asphalt material and dry-asphalt physics remain unchanged;
- seam crossing, forward/reverse motion and wheel contacts remain green.

Evidence hashes:

- `Saved/VF90/task2-runtime-gate.log`: `E9D0588C0817BC8526718D7024B83A6E759A541B782971B663463CE5619363C5`;
- `Saved/FocusedAcceptance/focused-runtime.log`: `D6B388BE136D5560CD99D67391594F0C196C8D540E5C8F3D04DAC1FAA551FD15`;
- fresh D3D12 fixed-view log `Saved/VF90/task2-d3d12-capture3.log`: `A4F31B7464F03C867DA7D0CEF06166A70E410F4067D1944485A599618272A3F9`.

Fresh D3D12 `PinkCab.Vehicle.Visual.FixedViewCapture` completed PASS after the candidate material was discoverable in a clean process. No `CDO Constructor ... Failed to find M_PC_RoadMarkSurface` remains in the successful final capture.

Matched D3D12 capture hashes:

- cockpit: `CDC1952EE74D4A7448F511B37528FD0ED5C9AFE400D856953950262CF328208B`;
- exterior: `C6B5B7C444CF950DD82118732C5080BB2932E7F706B16722AA35A7A0CBAAF62C`;
- side: `51A0FCB38B0FC03B04E02E8862260D5E1E7F612B0B601ED4FD28A5A40DC8DF5A`.

## Exact-package evidence

Accepted fallback package baseline capture:

- source: `6edea7747d3a8433188c9fb394b98ae9c320d49b`;
- screenshot SHA-256: `D03F054CB713ACDBEB328809B25743F692E30F011E807B5030B2D22853058D3A`;
- log SHA-256: `38D575DB1BE8471268EC332582E2B71BC4BE5FC8DC8052D20963AB24CCDBC08A`.

Candidate package was built from clean exact source `21911223a047987a175d14947b6d820e33b9b47f` with `scripts/build.ps1 -Package`; AutomationTool exited 0.

Candidate package hashes:

- `PinkCab.exe`: `94FC74A1C8EF8FF4CC43BF1D50C3ABC2F5977811705D2901080C8E3DC0545DB9`;
- `PinkCab-Windows.pak`: `2A1867E24F449ECDE4D3BABC86E55B0473BBC778669FB802E920DEEC5B3FA76E`;
- `PinkCab-Windows.ucas`: `629AEBDE64A970BEEEAA3C2C5FAFCAD8D966893E35EB4370BB609A5883F01552`;
- `PinkCab-Windows.utoc`: `27C31171E91240BEC4BCF74D84B3ACCA29E172331E2E20D732B05FD0A2CE73DC`;
- package build log: `53EEB83F2E96D6A4E7EFAF1E2182D675B9F605A1D07DE4FEBE002A65E3FB0BF5`.

Candidate package was launched directly from `Artifacts/Package/Windows`, not installed. It exited 0 and reported:

`PINKCAB_ROAD_MATERIAL_AUDIT=PASS materials=PASS textures8k=PASS`

Candidate package screenshot SHA-256:

`68533B918549AB31EE71CD9B42339EE3CF4A4AB611A45A707FE5AAB5500B9C17`

Candidate package runtime log SHA-256:

`DC1FEEA784D30235D89A4049D354A6FA06296A54E541D950EB9F7405EE1F73DF`.

The package evidence proves the replacement material is cooked and bound in the executable candidate while road topology/contact/friction stay unchanged. Static captures are evidence of presentation preservation; they are not misrepresented as a standalone temporal-video proof.

## Boundary

Task 2 may be marked complete only after Jira/Confluence/Git administrative truth records this receipt. Task 3 model dimensions/materials/normals, Task 4 authored controls, Task 5 CoM/inertia/load foundation, Task 6 analog actuation/steering, and later handling/package owner gates remain open.
