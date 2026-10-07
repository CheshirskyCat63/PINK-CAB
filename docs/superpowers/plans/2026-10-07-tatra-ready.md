# Tatra Ready Implementation Plan

> **For agentic workers:** Execute inline using superpowers:executing-plans. Owner approved the audit plan in chat on 2026-10-07. Final vehicle acceptance remains the owner's gate.

**Goal:** Deliver one correctly presented, fully interactive Tatra on a stable road with enjoyable manual Chaos handling.
**Architecture:** Preserve accepted P4 tag 6edea77. Use main 8322f9c as the integration baseline. Finish administration in PR69 before runtime changes; one Chaos dynamics owner, authored presentation follows gameplay state.
**Tech Stack:** Unreal 5.8, Chaos Vehicles, Blender, PowerShell, Python, GitHub Actions.
**Spec:** Owner-approved ten-stage audit plan in the 2026-10-07 PINKCAB conversation; handling boundaries also in docs/PINK_CAB_VEHICLE_FEEL_90.md.

## Global constraints
- Accepted fallback and installed build stay intact until a verified candidate is delivered.
- No ABS, TC, ESP, automatic countersteer or trajectory rescue.
- No wider world, traffic, taxi, economy or Neural development before owner acceptance.
- Model dimensions, visible contacts and physics must be reconciled before handling calibration.
- No declaration of visual or driving PASS from static inspection alone.

## Review focus
- New/untracked/renamed files must not bypass frozen-domain guards.
- Missing tags must return a structured FAIL, not a null-method exception.
- Correct animation state must reach authored visible parts, including at partial travel.
- Actual Chaos actuation must be distinguished from requested controls in evidence.
- Packaged visuals and driving must reproduce the tested candidate, not an older revision.

## Tasks
- [x] 0. Fix scripts/vehicle-feel-guard.ps1 path coverage, untracked/rename detection and native command failures; fix scripts/platform-status.ps1 missing refs; add behavioral regression tests in scripts/tests/test_vehicle_feel_guards.py; wire scope and authority checks into .github/workflows/verify.yml. Run tests and inspect CI before integrating PR69.
- [x] 1. Record Blender/export/import/profile/package identities and capture fixed-view baseline; measure geometry and physical contacts. Store evidence separately from acceptance claims.
- [ ] 2. Isolate road shimmer in exact-package captures; change one demonstrated cause at a time. Preserve contact topology and friction during visual diagnosis.
- [ ] 3. Reconcile authored model dimensions/materials/normals with scripts/import_tatra_rig06.py, scripts/configure_tatra_rig06_materials.py and PinkCabVehicleVisualProfile.cpp. Verify matched views in UE and package.
- [ ] 4. Inventory authored bones and gameplay actions; bind pedals, windows, doors, instruments and other approved controls through existing presentation ownership. Verify neutral/mid/full travel and normal input paths.
- [ ] 5. Establish wheel geometry, body collision, base/fuel/crew mass, rear-engine CoM and inertia. Verify static sag, contact and loaded fixtures before changing grip.
- [ ] 6. Resolve steering authority and real analog actuation requirements in Chaos; add executable regressions for direction, clutch coupling, handbrake dosage and drivetrain continuity.
- [ ] 7. Calibrate suspension, dry tyres, brakes and recoverable rear-wheel-drive handling with recorded before/after maneuvers.
- [ ] 8. Verify wet surfaces, load envelope, 30/60/120 FPS and sustained driving. No assist contributions.
- [ ] 9. Build and smoke-test one exact-source package, deliver atomically with rollback preserved, then obtain owner verdict.

## Execution record
- Baseline: source-only isolated checkout C:/workspace/pinkcab-tatra-ready-20261007 at 5d1c94d, branch feat/vehicle-feel-90. Original E: checkout untouched.
- Desktop Commander process request rejected: tool approval required, approval policy never. No retry through another Commander endpoint.
- Local baseline test attempt: 41 tests, 42 errors caused by inaccessible Python TemporaryDirectory fixtures in this restricted session. This is not a passing baseline and not a product regression. No ACL or sandbox changes attempted.
- Runtime stages remain blocked until actual UE/package execution and visual verification are available; source changes alone do not close them.
- Guard regressions reproduced before edits: 6 tests, 19 failed assertions. After fixes: 6/6 tests PASS, covering permitted calibration, frozen layouts, committed/staged/unstaged changes, renamed files, untracked files and missing baseline/tag behavior.
- Hosted CI run 37615419140 confirms all 47 script tests, authority, scope, code-health and hygiene PASS on 7aea47e. Local full-suite failures are restricted-session fixture errors; hosted proof is retained separately.
- Independent administrative review found no Critical/Important production issue. Its staged/unstaged fixture isolation weakness was corrected by advancing the test baseline after the committed case.
- Jira CD-559/CD-848/CD-641/CD-855/CD-921 and the two principal Confluence authority/program pages received the current checkpoint; prior records are explicitly historical. Runtime/owner acceptance remains open.
- Access resumed after the owner reconnected Desktop Commander Remote. The earlier access block above is historical. PR69 is merged at cc16d0c9a22fe74fe4eae01a86aa66da8848e730. Runtime work uses an isolated clone at C:/workspace/pinkcab-vf90-runtime-20261007, branch feat/tatra-ready-runtime. All 330 tracked LFS assets materialized with hashes checked; all 47 script tests pass in the actual Windows execution environment.
- UE 5.8 editor compilation and the three existing RIG06 runtime/profile tests passed before production edits. Set TMP=TEMP in the launched build process: Remote PowerShell lacks TMP, which otherwise leaves the UE SDK-query Build.bat waiting on the wrong lock path. No global environment or access-control changes were made.
- V22 FBX SHA256: 482aa2cbd89bdc35276a6029252e264e51ef9229df5342f163f99f21112e611f. Blender 5.2.2 measured wheelbase 310.7013 cm, tracks 152.0113 cm, tyre diameter 64.2600 cm. UE import metadata points to the V22 export folder. The imported bone transform probe used an unregistered component and is NOT valid transform evidence. Live PIE measurements replace it.
- Pedal defect reproduced through controller Q/W/E input: all three visible authored pedals stayed at 0 degrees. After binding presentation state to the V22 pivots, brake/clutch/throttle achieve 20/24/28 degrees and show intermediate release poses before returning to rest. All three input-driven runtime tests pass.
- Fixed-view D3D12 captures exposed the standing prototype mannequin protruding through the floor. A runtime regression failed before the visibility correction and passed afterward. Matched post-fix render verification is still required.
- Live PIE wheel-contact baseline: front contact lateral error about 14 cm on each side; rear error about 33.8 cm longitudinal plus 14 cm lateral. All four wheels report contact. This proves template physics geometry disagrees with both the declared 2980/1520 mm physical dimensions and the visible tyres. A narrowly scoped horizontal physical-geometry correction is under test; chassis vertical offsets are preserved. Handling calibration remains pending.
- The horizontal geometry correction passes the source-dimension contract and forward/reverse drive smoke. It exposed a second defect: presentation applied native compression downward from the authored Z instead of upward from native rest Z (front 25 cm, rear 26.5 cm). Presentation now reads the native rest center plus simulated suspension and converts it into the authored component frame. Nine targeted tests pass; measured tyre/contact residuals are 0.08-0.30 cm vertically and about 0.01 cm horizontally. Repeated D3D12 side/cockpit/exterior captures succeeded; side view confirms the mannequin is absent.
- The native root-body mass was measured, not inferred from the component aggregate: requested 1657 kg, simulated 1657 kg. Component GetMass reports 1924.377 kg including other skeletal bodies and must not be used as chassis-load proof. The root COM is currently at local origin, so rear-engine balance remains an open calibration requirement.
- Expanded Pawn test selection exposed a stale CockpitBridge unit fixture (instantiates the stock component although the bridge requires the project component) and obsolete external-partial-torque expectations. This test is NOT passing; the provider/bridge production files are unchanged from main. Analog clutch/handbrake actuation and consistent coverage remain open under task 6.
- Both import/material scripts now default to the actual V22 export folder while retaining the existing UE asset identity and explicit environment override. No asset reimport was performed as part of this correction.
- Task 1 closed only after administrative convergence: exact `57d7779` Task-1 gate 4/4 PASS; durable receipt `docs/vehicle_physics/evidence/VF90_TASK1_IDENTITY_BASELINE_2026-10-07.md`; Jira CD-855 comment 16559 and CD-648 comment 16560; Confluence program 22413538 footer comment 31260692. Installed accepted fallback remains `6edea774`; no Task 2+ or owner-acceptance claim is implied.
- Task 2 pre-integration guard reconciliation: the owner-approved road-shimmer task requires exactly three otherwise-frozen road visual surfaces. `vehicle-feel-guard.ps1` therefore admits only `PinkCabL1RoadChunkActor.cpp`, `PinkCabRoadMaterialAudit.cpp` and `M_PC_RoadMarkSurface.uasset`; all neighboring World/Content paths remain forbidden and are regression-tested. This is scope alignment, not Task 2 completion.
