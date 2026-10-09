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
- [x] 2. Isolate road shimmer in exact-package captures; change one demonstrated cause at a time. Preserve contact topology and friction during visual diagnosis.
- [x] 3. Reconcile authored model dimensions/materials/normals with scripts/import_tatra_rig06.py, scripts/configure_tatra_rig06_materials.py and PinkCabVehicleVisualProfile.cpp. Verify matched views in UE and package.
- [x] 4. Inventory authored bones and gameplay actions; bind pedals, windows, doors, instruments and other approved controls through existing presentation ownership. Verify neutral/mid/full travel and normal input paths.
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
- Task 2 closed only after administrative convergence: implementation 21911223a047987a175d14947b6d820e33b9b47f; durable receipt 67a7ebb / docs/vehicle_physics/evidence/VF90_TASK2_ROAD_SHIMMER_2026-10-07.md; Jira CD-648 comment 16563, CD-559 comment 16564, CD-869 boundary comment 16565; Confluence program 22413538 footer comment 31621122. Exact-source package build/audit passed; installed fallback remains 6edea774; Task 3+ and final owner acceptance remain open.
- Task 3 closed only after administrative convergence: implementation fe60cb1dc481deed1a1f945297be5705bc1ec419; durable receipt 5a751c7 / docs/vehicle_physics/evidence/VF90_TASK3_TATRA_V23_APPEARANCE_2026-10-07.md; Jira CD-648 comment 16566, CD-559 comment 16567, CD-855 comment 16568; Confluence program 22413538 footer 31359000 and pre-model page 15663105 footer 31129603. Installed fallback remains 6edea774; Task 4+ and commercial provenance remain open.
- Task 4 closed only after administrative convergence: implementation 3efab779901d2355964d4062ee6d30b347426235; durable receipt 1b45549 / docs/vehicle_physics/evidence/VF90_TASK4_CABIN_PARITY_2026-10-08.md; Jira CD-648 comment 16569, CD-559 comment 16570, CD-855 comment 16571, CD-604 comment 16572; Confluence program 22413538 footer 31817730 and cabin parity page 5931070 footer 31424524. Exact-source Task-4 runtime 11/11, focused runtime 9/9, tooling 52/52, D3D12 and package gates passed. Installed fallback remains 6edea774. Task 5 physical foundation is next; Task 6 WIP remains held until Task 5 closes.


## Current task contracts and reconciliation - 2026-10-09

This section refines the existing approved Tasks5-9; it does not create a new execution lane. Main at audit c8459ba / PR70 integrates Tasks0-4. Task5 branch 0b395c9 adds only a RED test/receipt. Earlier dated environment/access/candidate entries above remain historical. Preserve accepted installed fallback 6edea774.

### Task5 - physical implementation, not a renamed donor

Owners CD-648 (implementation), CD-748 (vehicle boundary), CD-855 (asset/provenance boundary). Existing files: PinkCabChaosTatraPawn.cpp physical rig/contact initialization; PinkCabChaosPhysicalProfile.h/.cpp and fingerprint; PinkCabTatraProfile.h; PinkCabVehicleLoadState.h/.cpp; PinkCabChaosLoadBridge.cpp; existing Tatra physical-foundation runtime test in the Task5 branch; selected project-owned physical skeletal/PhysicsAsset assets.

1. Use the recorded RED as the baseline; do not rerun identical checks in place of implementation. Identify root/body coordinate frame and collision envelope from the current 613 dimensions, not the visible material/name. Keep authored appearance, wheel geometry and installed fallback intact.
2. Correct the complete mass moments: base chassis/rear-engine distribution and crew positions plus fuel/passengers. A decomposed engine mass is a part of the base, not additional kilograms. Exactly once total and CoM/inertia for empty, reference1657 and declared max2107. Do not infer double mass from aggregate skeletal GetMass when root mass is1657.
3. Generate/apply a defensible physical body/rig and inertia; preserve native contacts and wheel travel. Do not rename PA_SportsCar or tune grip/steering/yaw to pass geometry/load tests.
4. Measure settled sag, all four contacts, axle reactions and CoM/inertia under the load envelope. Reference45/55 F/R is a project target, not historical car data. The current RED0.451691 rear spring fraction and origin CoM cannot serve as accepted final distribution.
5. Gate: actual physical envelope alignment, not only path strings; root/total mass; mass moments and inertia; static equilibrium and contact through travel; load add/remove/save round-trip with no duplication; same-source runtime/package evidence and admin closure. Keep handling unchanged until this gate closes.

### Task6 - actual controls-to-Chaos actuation

Owners CD-659/CD-653/CD-645/CD-646; preserve CD-643/CD-644/CD-649 contracts. Existing implementation surfaces: PinkCabChaosVehicleDynamicsProvider.cpp, PinkCabChaosCockpitBridge.cpp, PinkCabChaosVehicleMovementComponent.h/.cpp, control runtime/steering controller and their actual runtime fixtures.

First inspect the installed native Chaos integration boundary and choose one supported actuator owner. PR67's retired competing vehicle simulation is not silently restored. Do not invent a direct chassis-force path. The current provider's coupling>=0.95 gear switch and handbrake>epsilon boolean are the defects, not accepted analog behavior. Dormant clutch-model code/configuration is not proof of integration.

Prove driver input -> commanded state -> actual wheel steering/brake/drive response -> presentation. Include right/left forward/reverse; held steering while speed changes without new input; Q re-press/release timing; partial clutch at0.5/0.949/0.95/0.999/1.0; first/R and near limiter/load/thermal cases; handbrake25/50/100 rear braking and zero release; requested versus engaged gear and no buffered focus-return shift. Keep engine-Off combustion separate from mechanical coast/back-drive. No reverse-only30/35km/h governor.

Repair the existing stale CockpitBridge test's component/expectations; do not reintroduce engine-Off mechanical freeze or old external-partial torque merely to pass it. Extend the existing verification entry with applicable true-actuation regressions as their implementation lands. RED tests may remain on the Task5 branch until fixed; do not weaken contracts to force a merge.

### Task7 - measured dry handling

Owners CD-652/CD-650/CD-656/CD-654/CD-655/CD-641/CD-642. Use the existing versioned physical profile, suspension/wheel configuration and engine/steering response surfaces. One demonstrated cause per calibration delta. Establish declared comparison tolerances before measuring; no number invented by an administrative edit.

Same input/load/surface before/after: launch/coast/engine braking, low-speed precision, straight line, lane change/constant-radius, service braking, throttle/lift/handbrake breakaway and deliberate recovery. Record wheel contact/load/slip and physical response separately from visual expression. Target speeds are tested only where physically reached; not reaching a required performance target stays open, never extrapolated to PASS. No permanent rear-grip cheat, hidden boost, speed wall or automatic correction.

### Task8 - wet, load, state and soak

Owners CD-658/CD-657/CD-670/CD-722/CD-740/CD-559. Reuse the same car/single dynamics owner at30/60/120FPS and empty/reference/max loads; run the agreed30-minute normal-drive serviceability test, braking/clutch work-based thermal cases and focus/menu/save/reload boundaries. Compare tolerances, discrete gear/input outcomes, drift of counters/state, stuck input, NaN and unintended repair/mass duplication. Wet road fixtures qualify the car only; R05 later proves world-weather integration. Existing wider2-hour/vertical/city requirements are retained for the applicable product gate.

### Task9 - scoped owner milestone and release-safe handoff

Owners CD-559 delivery, CD-648 milestone, CD-658/CD-921 scoped evidence. Read current verify.yml/deliver.yml; do not resurrect archived workflows. Build one exact source/content/profile candidate; run applicable vehicle/cabin/road regressions and package/input/smoke checks. Keep full-product unexecuted rows explicit; no summing different SHA results. Deliver only after technical gates with rollback preserved, then record the owner's visual/driving ACCEPTED or REJECTED verdict. No automatic owner approval.

VF90_ACCEPTED unlocks R01/CD-869 without closing every broader CD-648/CD-658/CD-921 requirement. Unrelated wider profiles603/77, world/vertical, full cabin and public rights remain with existing owners. Rejection reopens only the failing current task.

### Audit disposition

314 product-filter Jira records were screened as inventory; full descriptions were inspected for active authorities/conflicts. Existing PHY48 and product97-row source/proof matrices remain scoped evidence. Closed historical/primitive tasks are not reopened wholesale or counted as complete player loops. CD-646 retires the forbidden governor; CD-869 is parked until Task9; CD-644/CD-649 historical DONE is not current physical acceptance. Correct current task descriptions and canonical entrypoints, preserve prior receipts and uncommitted trees, and perform no runtime/assets/install changes in this administrative transaction.


### Task5 execution ruling - vehicle coordinate persistence only / 2026-10-09

The approved Task5 load/restore requirement cannot preserve physical CoM and inertia if the existing snapshot serializes only longitudinal positions. Schema4 therefore adds lateral/vertical vehicle-load coordinates while retaining explicit legacy v1/v2/v3 handling and thermals. This does not change campaign, fare, economy, workday or world-save policy. The broad persistence scope freeze has an exact four-file exception: PinkCabPersistence Private/Public Persistence/PinkCabVehicleSnapshot.cpp/.h and PinkCab Private/Public Persistence/PinkCabVehicleSnapshotArchive.cpp/.h. A failing-then-passing guard test permits only these four existing vehicle serializers and still rejects neighbouring persistence files. No directory-wide exception or disabled verification is authorized.

The original two-wall-clock-second single-frame static balance sample varied around its upper threshold. Keep its53%-57% acceptance unchanged, but evaluate an interval from3?4 seconds of actual simulation with continuous four-wheel contact. The separate empty/reference/max/reference live-load test checks physical mass/principal frame/inertia after native recalculation and repeated restore, suspension travel reserve and unload recovery. Raw native SpringForce is not mislabeled as SI newtons.
