# PINK CAB · Project Setup

## Canonical checkout

Repository: `CheshirskyCat63/PINK-CAB`

Primary local checkout:

`E:\CHESHIRE_DIVISION\Games\PINK-CAB`

Integration target: `main`.

Do not use restored copies, package worktrees or historical research worktrees as development authority.

## Unreal Engine

Engine family: UE 5.8.

Default workstation engine root:

`C:\Program Files\Epic Games\UE_5.8`

Override with `PINKCAB_UE_ROOT` or the scripts' `-EngineRoot` parameter.

## Free road content

The current P4 road does not depend on the MetaRoad authoring framework.

`scripts/ci/prepare_metaroad.py` retains MetaRoad 3.2.0 only as frozen content-only vendor material for the baked road. On a clean workstation, set `PINKCAB_METAROAD_PACKAGE` to the local authorized MetaRoad 3.2.0 package.

The active project does not require MetaRoad source modules, MetaRoadEditor or StructUtils.

## Build

Editor:

`pwsh -NoProfile -File scripts/build.ps1`

Package:

`pwsh -NoProfile -File scripts/build.ps1 -Package`

Generate IDE files:

`pwsh -NoProfile -File scripts/build.ps1 -GenerateProjectFiles`

## Verify

Local focused runtime:

`pwsh -NoProfile -File scripts/verify-runtime.ps1`

GitHub:

`.github/workflows/verify.yml`

The workflow runs static contracts first, then the exact-head UE build and focused runtime suite on the self-hosted PINKCAB runner.

## Deliver

GitHub manual workflow:

`.github/workflows/deliver.yml`

Implementation:

`scripts/deliver.ps1`

Delivery requires a clean exact HEAD, editor build, focused runtime, package and packaged smoke before atomically updating `PINCKCAB_BUILD` and `PINCKCAB.lnk`.

## History

Old P00–P04 drivetrain experiments, CD-648/CD-869 workflows and road-authoring programs are archived history. Use `archive/20261006/*` tags when historical evidence is needed; do not restore those systems into the active baseline without a new explicit design decision.
