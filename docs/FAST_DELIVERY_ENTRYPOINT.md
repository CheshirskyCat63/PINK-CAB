# Legacy fast-delivery entry is retired

`scripts/fast-delivery.ps1` now refuses delivery with exit code **2**, before any engine, filesystem, package, environment or shortcut operation. The old parameters remain recognizable so existing callers receive the retirement message. `-SkipTests`, `-ForceRecook`, a custom `-TestFilter`, engine path or destination cannot reactivate it.

`powershell -NoProfile -File scripts/fast-delivery.ps1 -PlanOnly` returns a read-only JSON migration plan. It does not inspect or modify the checkout, require Unreal or Git, dispatch a workflow, build, launch, or accept a candidate. Success from this planning command is not delivery success.

The retired implementation cooked the old ChaosWeave map, changed the mutable iteration package before smoke/input checks, deleted its previous package, and exposed a test-skip switch. Fixing only the map would retain the unsafe promotion behavior. Its implementation remains in Git history; it is not an alternative delivery engine.

## Existing explicit delivery route

1. Use the existing **CD-648 P02 PHY-009 Drivetrain Continuity** workflow (`cd648-p02-phy009.yml`) on a candidate ref fixed to the reviewed full 40-character commit SHA. Record that SHA and verify the Actions run's source SHA matches it. This is a manual dispatch, not an automatic action performed by the retired script.
2. Explicitly select `deliver_human=true` only when delivery is intended. P02 verification must succeed before its existing call to `cd869-deliver.yml`, which receives that exact candidate SHA. A failed verification does not start delivery. Direct manual CD-869 dispatch also accepts `candidate_sha`, but does not perform the preceding P02 verification; it must not substitute for that gate.
3. The existing CD-869 route targets `/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight`, checks packaged map/vehicle/material smoke before creating delivery shortcuts, and reports **HUMAN_PENDING**. Retain the candidate SHA, Actions run, package path and smoke evidence together. A shortcut to the latest human candidate is not an accepted baseline or LastGood promotion.
4. Keep the previously accepted build until the owner accepts the exact new candidate. Technical smoke, physics checks, packaged input acceptance and human acceptance are distinct evidence. P03 remains rejected/pending corrective work; this administrative retirement changes none of those decisions.

## Remaining delivery work under CD-559

The existing route is documented accurately, not newly certified by this retirement. CD-869 currently checks a cached package's source marker and executable presence; it does not verify a complete package hash manifest or run the packaged OS input gate. Its delivery directory also omits the run attempt, so rerunning that workflow can replace a directory from the same Actions run. Do not treat these properties as immutable, input-verified promotion.

A future promotion implementation must retain a unique candidate directory per SHA/run/attempt, record executable and complete cooked-payload hashes plus the source/cooked-base identities, validate the required packaged smoke and input evidence for those exact bytes, and only then update the promotion pointer. Failure must preserve the previous accepted package and pointer. It must use the existing delivery workflow rather than recreate an independent local delivery engine. Clean-checkout reproducibility, complete candidate regression, package validation and human acceptance remain open until supported by their own evidence.

Documentation-only administration does not require producing a replacement runtime package. Existing accepted artifacts keep their original SHA and identity.
