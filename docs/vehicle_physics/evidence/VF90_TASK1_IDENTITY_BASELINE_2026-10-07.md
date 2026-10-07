# VF90 Task 1 — identity / baseline measurement receipt — 2026-10-07

Status: **TECHNICALLY VERIFIED; administrative convergence pending at creation time**.

Scope is only Task 1 from `docs/superpowers/plans/2026-10-07-tatra-ready.md`: record Blender/export/import/profile/package identities, preserve fixed-view baseline evidence, and measure geometry plus physical contacts. This receipt does **not** accept Task 2+, final visuals, handling, or a replacement package.

## Source / package identities

- Runtime work branch: `feat/tatra-ready-runtime`.
- Exact verified current source: `57d777988698336a9d7e985e6fcd39d547e59068`.
- Reflog proves the isolated checkout remained at `cc16d0c9a22fe74fe4eae01a86aa66da8848e730` from 15:37:40 +03:00 until the first runtime commit at 16:20:38 +03:00. The fixed-view baseline and RED contact measurements were therefore captured before the runtime correction on that baseline identity.
- Installed owner-accepted fallback remains source `6edea7747d3a8433188c9fb394b98ae9c320d49b`, confirmed by `C:\Users\CheCat\Desktop\PINCKCAB_BUILD\SOURCE_HEAD.txt` and the `PINCKCAB.lnk` description.
- Installed fallback launcher `PinkCab.exe` SHA-256: `94FC74A1C8EF8FF4CC43BF1D50C3ABC2F5977811705D2901080C8E3DC0545DB9`.
- Installed fallback `PinkCab-Windows.pak` SHA-256: `F3C1512732FCEBF7C21617D416169D7988029C2CAE7FF2C75FE912D46D7C9F89`.
- No candidate package is claimed by Task 1.

## Blender / export identity

- Blender: **5.2.2 LTS**.
- FBX: `E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\export\TATRA613_RIG22_UE\TATRA613_CHAOS_RIG_06_TEXTURED_OPENABLES.fbx`.
- FBX SHA-256: `482AA2CBD89BDC35276A6029252E264E51EF9229DF5342F163F99F21112E611F`.
- Blender measurement receipt SHA-256: `ADB11AE840DF61902D9A6F7E8BE72EE0E50F9E0649E0361E90274A1013C1FE40`.
- Measured authored geometry: wheelbase **310.70129 cm**, front track **152.01133 cm**, rear track **152.01124 cm**, tyre diameter **64.2600 cm**.
- Measurement boundary remains explicit: Blender reimport evidence is not UE/package acceptance.

## UE import / profile identity

- Imported skeletal mesh: `/Game/Dev/Vehicles/Tatra613Rig06/SK_Tatra613_Rig06.SK_Tatra613_Rig06`.
- UE import metadata source points to the same V22 FBX path above.
- UE import measurement receipt SHA-256: `9A901EA5B158DD77E7402CA80162398AE0B27C872D287372F1C59BCAFABC4AD4`.
- Physical profile: `PINKCAB_TATRA613_CHAOS`, model `TATRA_613`.
- Profile schema: **1**; calibration: **7**.
- Unit contract: `PINKCAB_PHYSICS_UNITS_V1`.
- Provenance set: `PINKCAB_TATRA613_BASELINE_2026_09_26`.
- Compatibility: `PINKCAB_CHAOS_PROFILE_V1`; migration: `PINKCAB_TATRA613_PROFILE_V1`.

## Fixed-view baseline

The pre-runtime-change fixed views are retained separately from later candidate captures:

- `Saved/VF90/BaselineCaptures/cockpit.png` SHA-256 `7CC60F325594EB143DCCDFEDBC45A0D59F441A4EF0D96DC9B3C6F9551C4BCEB6`.
- `Saved/VF90/BaselineCaptures/exterior.png` SHA-256 `560E7C4CC38FF54FD10BBB67556278E4B723F8A57DED93753EDB8DF710E85DEE`.
- `Saved/VF90/BaselineCaptures/side.png` SHA-256 `0D4B0E24EE820BEBD8E0F57BAEE46A9014BB0C4BA4134E5885BA519566A2E30A`.

These are baseline evidence, not a visual PASS claim.

## Geometry / contact measurement

RED baseline on the pre-runtime-change checkout:
- front lateral mismatch approximately **14.0 cm** each side;
- rear mismatch approximately **33.8 cm longitudinal + 14.0 cm lateral**;
- all four wheels still reported ground contact.

Exact `57d7779` verification after the geometry/presentation correction:
- `PinkCab.Vehicle.AssetContract.Tatra613V12` — PASS;
- `PinkCab.Vehicle.Visual.Tatra613Rig06Profile` — PASS;
- `PinkCab.Vehicle.ChaosBaseline.Pawn.TatraWheelGeometryBinding` — PASS;
- `PinkCab.Vehicle.Visual.TatraWheelContacts` — PASS;
- total **4/4 PASS**.

Current live contact residuals on exact `57d7779`:
- FL: horizontal <= 0.002 cm, vertical 0.182 cm;
- FR: horizontal <= 0.002 cm, vertical 0.190 cm;
- BL: horizontal <= 0.002 cm, vertical 0.033 cm;
- BR: horizontal <= 0.002 cm, vertical 0.041 cm;
- all four grounded.

Exact build log SHA-256: `4C2420B0EF65EA006E2469B9210B8CE6A70E28A932B2BFD3B6DAA28BF22C1661`.
Exact runtime log SHA-256: `C9BF896EE3429A75ED356BDB172E8397BA82D32B6898500ACA80E76B524005E8`.

## Boundary

Task 1 may be marked complete only after Jira/Confluence/Git administrative truth records this receipt. Task 2 road shimmer, Task 3 material/normal reconciliation, Task 4 full authored-control parity, Task 5 CoM/inertia/load foundation, Task 6 analog actuation/steering, and later handling/package gates remain open.
