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
- [ ] 0. Fix scripts/vehicle-feel-guard.ps1 path coverage, untracked/rename detection and native command failures; fix scripts/platform-status.ps1 missing refs; add behavioral regression tests in scripts/tests/test_vehicle_feel_guards.py; wire scope and authority checks into .github/workflows/verify.yml. Run tests and inspect CI before integrating PR69.
- [ ] 1. Record Blender/export/import/profile/package identities and capture fixed-view baseline; measure geometry and physical contacts. Store evidence separately from acceptance claims.
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
