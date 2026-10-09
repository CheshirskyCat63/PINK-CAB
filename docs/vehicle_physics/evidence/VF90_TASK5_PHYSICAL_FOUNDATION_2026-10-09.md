# VF90 Task5 - physical foundation implementation / 2026-10-09

Status: LOCAL TECHNICAL PASS; PR/hosted review/integration remain pending. Not owner driving acceptance and not Tasks6-9 completion.

## Exact identity

Final implementation/test head: `e0dee4e3bb18e2bf6615ed27e1a4d13098dcba31`. Runtime/asset implementation: `2238b6cf955a8dfec8c2c0f8bdcac962a5e99fd7`; the following commit changes only a test to distinguish retained v3 from XYZ v4. Full BuildCookRun and package smoke were executed at final implementation/test head. Evidence-only receipt commits are not relabelled executable source.

Accepted installed fallback remains `6edea7747d3a8433188c9fb394b98ae9c320d49b`; no installer/shortcut replacement. Separate local package: `Artifacts/Package/Windows`. Previous local archive retained as `Artifacts/Package_retained_before_Task5_20261009`. Source, cooked payload and log SHA256 manifest: `VF90_TASK5_LOCAL_PROOF_2026-10-09.json` beside this receipt.

## Production change

- New physical Tatra skeletal mesh and PhysicsAsset: five bones, one chassis body, two measured convex envelopes, zero donor wheel constraints. The source-derived physical proxy is separate from the unchanged accepted RIG24 appearance; four wheel rest positions preserve the approved contact geometry.
- Complete base/rear-assembly/fuel/crew/passenger mass moments in XYZ, native Chaos mass-property combination and principal inertia, applied to the real body including native recalculation. Rear assembly275kg is included within1450kg, never added again.
- Explicit calibration version2 includes geometry-derived body/crew positions and declared internal mass-volume/height seeds. The latter are not factory measurements. Reference45/55 is project design, not historical evidence; no grip/steering/yaw/force compensation was added.
- Snapshot schema4 stores XYZ; old longitudinal v3 and divergent v2/v1 layouts remain readable with thermal/damage semantics preserved. Only four existing vehicle serializers are exempted from the broad persistence freeze; neighbour files still fail the guard.

## Verification actually executed

Editor build PASS; physical/LiveState/contact group16/16 PASS; canonical focused suite20/20 PASS including original nine tests; persistence regression34/34 PASS; script tests54/54 PASS; code-health/hygiene zero violations; authority/scope and whitespace guards PASS. These are separately identified runs, not a combined full-product test count. Twenty named tests now execute through the existing verify-runtime.ps1; no new workflow.

Native package build/cook/stage/archive SUCCESS, AutomationTool0. Headless packaged smoke exit0 and both new physical assets are loaded in the packaged game. This is asset/startup proof, not a driving-feel or visual owner verdict.

## Live load sequence

The fixture restores each load twice, invokes native body mass recalculation, then measures a settled interval of simulation while preserving velocity and contacts.

|Fixture|Body mass kg|CoM XYZ cm|Mean suspension compression cm|
|---|---:|---|---:|
|Empty|1450|(-33.775,0,45.948)|1.39|
|Reference|1657|(-28.900,-0.259,46.858)|2.88|
|Max test load|2107|(-43.444,-0.204,52.548)|6.16|
|Reference restored|1657|(-28.900,-0.259,46.858)|2.85|

Values above identify the first successful envelope run in `task5-envelope-runtime.log`; that run separately exposed the old baseline sampling defect and is not presented as wholly green. Final successful runs and their exact logs are recorded in the JSON manifest. All four contacts and suspension travel reserve survive each load; actual principal-frame/inertia match computed values after rebuild. Reference settled rear spring share was about55.31%, consistent with the project55% target. Raw native SpringForce is not advertised as SI normal-force measurement; absolute wheel-force calibration remains a separate limitation.

## Failures retained and corrected

Original Task5 RED (SportsCar/zero CoM/front-heavy) is preserved. Three-axis moment RED existed before implementation. Resume fixed private-field test compilation and matching-header order, not acceptance limits. The old baseline sampled one frame after two wall-clock seconds and varied across its upper bound; it now samples3-4 simulation seconds with contact assertions, retaining the53%-57% criterion. Persistence regression exposed a stale CurrentSchemaVersion==3 assertion; the test now explicitly retains longitudinal v3 and requires separate XYZ v4. No physics behavior was changed merely to satisfy that stale version assertion.

## Remaining gates / boundaries

PR review, hosted exact-head verification, integration and administrative closeout must precede Task5 closure. Tasks6 analog clutch/handbrake and steering,7 dry feel,8 wet/load/FPS/30-minute robustness and9 owner package are not complete. Wider profiles603/77, world/vertical/full cabin and public donor rights remain with their existing owners. No second vehicle solver, hidden force, speed governor, ABS/TC/ESP or driving rescue was introduced.

Delivery safety finding for CD-559/Task9: current deliver.ps1 deletes PINCKCAB_BUILD.old after swapping. Do not use real-desktop delivery until retention and failed-swap recovery are proved. No installed bytes were changed in this task.
