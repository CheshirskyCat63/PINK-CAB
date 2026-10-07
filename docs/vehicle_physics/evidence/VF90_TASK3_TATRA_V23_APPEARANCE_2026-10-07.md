# VF90 Task 3 — Tatra V23 dimensions / materials / normals receipt — 2026-10-07

Status: **TECHNICALLY VERIFIED; administrative convergence pending at creation time.**

Scope is only Task 3 from `docs/superpowers/plans/2026-10-07-tatra-ready.md`: reconcile authored model dimensions, material/texture policy and normals with the RIG06 authoring/import scripts and `PinkCabVehicleVisualProfile`, then verify matched views in UE and package. This receipt does not close Task 4+, vehicle handling, commercial provenance, or owner acceptance.

## Exact implementation

- Branch: `feat/tatra-ready-runtime`.
- Task-2 close baseline: `6217d86`.
- Exact Task-3 implementation: `fe60cb1dc481deed1a1f945297be5705bc1ec419`.
- Worktree was clean and synchronized with origin for exact-SHA verification.
- Installed owner-accepted fallback remains `6edea7747d3a8433188c9fb394b98ae9c320d49b`; it was not replaced.

## Blender / FBX reconciliation

Canonical source remains:
`E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\TATRA613_CHAOS_RIG_22_FINAL.blend`

Blender: **5.2.2 LTS**.

V22 FBX:
- SHA-256 `482AA2CBD89BDC35276A6029252E264E51EF9229DF5342F163F99F21112E611F`.

V23 FBX:
- `E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\export\TATRA613_RIG23_UE\TATRA613_CHAOS_RIG_06_TEXTURED_OPENABLES.fbx`
- SHA-256 `0187261CDD68A51CBA9DE9DF619E820248F0FB51B5FE7FC0528D0D8CA41DF244`.
- export report SHA-256 `C7D823743A537010DB46C650F587A62AE31BE3CDA0E75B8005DB2F463504D183`.

The V23 authoring report records identical pre/post geometry+bone digest and only adds UV0 to the 12 SimPedals donor meshes.

Independent clean-FBX Blender reimport proved V22 and V23 are geometrically identical:
- meshes: **212 / 212**;
- bones: **74 / 74**;
- wheelbase: **3.1070129 m / 3.1070129 m**;
- front track: **1.5201133 m / 1.5201133 m**;
- rear track: **1.5201124 m / 1.5201124 m**;
- required bones missing: **0 / 0**;
- non-corner authored normals on real meshes: **0 / 0**;
- real meshes without UV0: **12 in V22 → 0 in V23**.

The apparent `Cube/Material` found by an earlier audit was a Blender `--factory-startup` default cube in the probe scene, not FBX vehicle geometry. Correct empty-scene reimport reports 212 meshes and no such vehicle object. No geometry was deleted to resolve it.

Evidence:
- `Saved/VF90/task3-clean-v22.json` SHA-256 `A6D2FA67871EECD4199122BC062C0C9ADDA8363BA550DC6BDB35B3FEDF869F3C`;
- `Saved/VF90/task3-clean-v23.json` SHA-256 `82834293597F31502FE71A7BC4A0F44B15CA4035032BB6AE4006807E914ECDD3`.

## UE import / material truth

Exact `6217d86` baseline was materialized from its committed LFS object before audit.

Baseline UE asset:
- source = V22 FBX;
- normals = `FBXNIM_COMPUTE_NORMALS`;
- generation = `MIKK_T_SPACE`;
- weighted recomputed normals = **true**;
- material slots = **18**;
- `T_PC_Carpet_ORM` = **sRGB true / TC_DEFAULT**.

Exact `fe60cb1` candidate:
- source provenance = V23 FBX;
- normals = **`FBXNIM_IMPORT_NORMALS`**;
- tangents = **MikkTSpace**;
- weighted recomputed normals = **false**;
- material slots remain **18**;
- skeletal bounds are byte-for-value identical at the audited logical level;
- `T_PC_Carpet_ORM` = **sRGB false / TC_MASKS**, matching the source manifest convention R=AO, G=Roughness, B=Metallic.

No other audited texture property changed between baseline and candidate.

The import script now reimports the existing skeletal asset in place instead of deleting the whole RIG06 destination folder. On reimport it does not recreate materials/textures, and it verifies source provenance plus normal/tangent policy after import. The material script saves only dirty assets and normalizes all authored BaseColor/Normal/ORM texture metadata, including currently unused texture sets. `PinkCabVehicleVisualProfile` retains the accepted transform unchanged; only the V23 source identity is documented.

Logical audit hashes:
- baseline: `0A5F75C6083C0A3B7F3D4ADE7FC4203F70A287BCEFEB5F30AC0C459A79672022`;
- candidate: `B3AE8FC977CE3D25B786CDF657602859869F1685CB1453DB269A9542316989A3`.

## Automated verification

Exact candidate:
- authoring/tooling suite: **51/51 PASS**;
- vehicle-feel scope guard: PASS;
- `git diff --check`: PASS;
- UE Editor build: PASS;
- Task-3 RIG06 runtime gate: **9/9 PASS**:
  - Tatra613V12 asset contract;
  - Tatra wheel geometry binding;
  - brake/clutch/throttle authored pedal travel;
  - RIG06 openables;
  - RIG06 profile;
  - RIG06 steering presentation;
  - Tatra wheel contacts.
- standard focused runtime gate: **9/9 PASS**.

Evidence hashes:
- script suite: `9EDC3928535911A8D3B3721F0B3C90E0B4C007D52C5EBA6E309DEF272A4D54F8`;
- build: `F34ABAF26DEFD915DA9E297DA1C503183A1DDFDB470CFA5E04943F5DF7FB2B06`;
- Task-3 runtime: `5450FE3E25BC046C96C83A8D6C3550A2317A68E1AE1189E237852323FB77D907`;
- focused runtime: `6B2882216924FD7993399E180DC9F88ED552E8C10B1A2EBA1100BB576D1A6A4C`.

## D3D12 matched views

Fresh exact-`fe60cb1` D3D12 `PinkCab.Vehicle.Visual.FixedViewCapture` completed PASS.

Baseline `6217d86` capture hashes:
- cockpit `CDC1952EE74D4A7448F511B37528FD0ED5C9AFE400D856953950262CF328208B`;
- exterior `C6B5B7C444CF950DD82118732C5080BB2932E7F706B16722AA35A7A0CBAAF62C`;
- side `51A0FCB38B0FC03B04E02E8862260D5E1E7F612B0B601ED4FD28A5A40DC8DF5A`.

Candidate capture hashes:
- cockpit `F8A1C2D7116DB886AE129262CE5DB5C15230D20DDA7FA95C7813F716D7D93403`;
- exterior `961DE43CDFBEB753783E25CE50BE831B28F3418BB6B6B43536FCE252842D4BC7`;
- side `25B50F5489D7F8E3020BB9B8CD17B546F2E8CFE63645C8C0EF01C070AB62D4D2`.

The camera transforms, vehicle transform and wheel placement are unchanged. The visible difference is the intended authored-normal shading: flat body panels are no longer artificially rounded by UE weighted normal recomputation.

D3D12 log SHA-256:
`3E18E8A39C5C955D92618C0782D144BA8733C549DB86424FFE260B9AE8AA411E`.

## Exact package verification

`scripts/build.ps1 -Package` on clean exact `fe60cb1` completed successfully; AutomationTool ExitCode 0.

Package hashes:
- `PinkCab.exe`: `94FC74A1C8EF8FF4CC43BF1D50C3ABC2F5977811705D2901080C8E3DC0545DB9`;
- `PinkCab-Windows.pak`: `05378E81B02E1A152CCEF8C187E1734895A70FFE25BBAFDC76F6D2EE422C2E14`;
- `PinkCab-Windows.ucas`: `D7557C9B82CFBDB587D061A9016E94E8056CD89515D3AA62BDC9960C7C555B0B`;
- `PinkCab-Windows.utoc`: `8932D37544261503703FF9B3C58C029AF5CC61867D6A74A481E8DCA7AEE65CCC`.

Package build log SHA-256:
`20DCA1B384E23AF805C0A2AA280E0705CD7F35B173113199DC64CD58F9064D0C`.

The candidate package was launched directly from `Artifacts/Package/Windows`; exit 0. It reported:
`PINKCAB_ROAD_MATERIAL_AUDIT=PASS materials=PASS textures8k=PASS`.

Package log SHA-256:
`FBEF1FF8F0BAD7B79AEF9DEE99603F8E9A7277FF01E4F3D92300811F569705A0`.

The package log contains no Tatra/material asset load failure. Optional profiler DLL warnings (`aqProf`, VTune, WinPix capturer) are engine startup diagnostics and are not asset failures.

Matched package evidence uses the same L1 map, 1600x900 render-offscreen HighResShot command and startup camera:
- accepted fallback baseline screenshot SHA-256 `D03F054CB713ACDBEB328809B25743F692E30F011E807B5030B2D22853058D3A`;
- Task-3 candidate screenshot SHA-256 `AD6C23BBD8F3E19F6CEC4113077976684B9458EBBFF5965BAF8B391DF08B886B`.

Package automation modules are intentionally not cooked; a direct packaged `FixedViewCapture` request correctly reported 0 matching tests. No production capture hook was added. Built-in DebugCamera/Spectator probes were diagnostic only and are not counted as acceptance evidence.

## Boundaries

This closes only the internal Task-3 Blender/export/import/material/normal reconciliation. CD-855 remains open for Task 4 presentation/cabin parity and its independent commercial provenance/public exposure requirements. No permission/redistribution claim is created by this work. Task 4+, mass/CoM, controls/actuation, handling and final owner acceptance remain open.
