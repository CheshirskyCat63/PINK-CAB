# Legacy fast-delivery entry is retired

`scripts/fast-delivery.ps1` now refuses delivery with exit code **2**, before any engine, filesystem, package, environment or shortcut operation. The old parameters remain recognizable so existing callers receive the retirement message. `-SkipTests`, `-ForceRecook`, a custom `-TestFilter`, engine path or destination cannot reactivate it.

`powershell -NoProfile -File scripts/fast-delivery.ps1 -PlanOnly` returns a read-only JSON migration plan. It does not inspect or modify the checkout, require Unreal or Git, dispatch a workflow, build, launch, or accept a candidate. Success from this planning command is not delivery success.

The retired implementation cooked the old ChaosWeave map, changed the mutable iteration package before smoke/input checks, deleted its previous package, and exposed a test-skip switch. Fixing only the map would retain the unsafe promotion behavior. Its implementation remains in Git history; it is not an alternative delivery engine.

## Existing explicit delivery route

1. Use the existing **CD-648 P02 PHY-009 Drivetrain Continuity** workflow (`cd648-p02-phy009.yml`) on a candidate ref fixed to the reviewed full 40-character commit SHA. Record that SHA and verify the Actions run's source SHA matches it. This is a manual dispatch, not an automatic action performed by the retired script.
2. Explicitly select `deliver_human=true` only when delivery is intended. P02 verification must succeed before its existing call to `cd869-deliver.yml`, which receives that exact candidate SHA. A failed verification does not start delivery. Direct manual CD-869 dispatch also accepts `candidate_sha`, but does not perform the preceding P02 verification; it must not substitute for that gate.
3. The existing CD-869 route targets `/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight`, checks packaged map/vehicle/material smoke before creating delivery shortcuts, and reports **HUMAN_PENDING**. Retain the candidate SHA, Actions run, package path and smoke evidence together. A shortcut to the latest human candidate is not an accepted baseline or LastGood promotion.
4. Keep the previously accepted build until the owner accepts the exact new candidate. Technical smoke, physics checks, packaged input acceptance and human acceptance are distinct evidence. P03/V2 and bounded P04 are now owner accepted; original pending-delivery records remain historical evidence.

## Remaining delivery work under CD-559

Completed P04 protections: unique SHA/run/attempt directory, complete 53-file installed manifest/path/size/hash comparison, packaged smoke, verified visible window, atomic PINCKCAB-only publication and prior-link backup. Runs37149462470 and37151172013 prove those protections; additive owner receipt37152180059 records acceptance. Previous V2/P02 retained.

Normal coordinator: `.github/workflows/pinkcab-repository-verification.yml` — complete scope, repository checks, exact-source physics, same-run evidence plus five independent slope repeats, then aggregate technical gate. Main requires both `Repository verification` and `Gameplay acceptance gate`.

Human delivery is explicit: `cd648-p02-phy009.yml` with `deliver_human=true` on the reviewed candidate ref, then its verified `cd869-deliver.yml` call. Record the full SHA/run/attempt. The route checks packaged map/four-wheel/material smoke, installs to a unique SHA/run/attempt directory, compares payload paths/sizes/SHA256, verifies an interactive window and atomically replaces only `PINCKCAB.lnk`, retaining prior link and accepted package. Delivery initially records HUMAN_PENDING; owner acceptance is a subsequent separate record.

Retained G1 fast/human_gate/release_gate and inline definitions are auxiliary/history, not competing routine entry points. Policy and implementation now permit only P02-mediated owner-test delivery: CD-869 is reusable-only (`workflow_call`), has no direct `workflow_dispatch`, and its delivery job requires the verified P02 caller identity plus the same GitHub run ID and attempt. The retired local fast-delivery script refuses mutation. Full clean-source/full-project/packaged-input qualification remains CD-559 engineering. Cached-package source-marker/executable-presence checks are not a complete cache-provenance certificate.

Documentation-only administration does not require replacing the runtime package. PINK CAB and KUKURUZA registrations share one physical host; coordinate heavy Unreal work without stopping another project's processes.
