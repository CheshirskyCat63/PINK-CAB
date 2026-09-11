# PF-01 Core Contracts Verification — 2026-09-11

Jira: `CD-756`
Branch: `feat/CD-756-pf01-core-contracts`
Engine: Unreal Engine 5.8.2, CL 56702186

## TDD evidence
- RED: canonical `scripts/build.ps1` failed with exit 6 because `Core/PinkCabFeatureConfig.h` did not exist.
- Production Core contracts were added only after that RED.
- A header-only ABI issue was isolated: `PINKCAB_API` on inline value types caused unresolved imported symbols; removing that annotation fixed the root cause without weakening tests.

## Fresh GREEN evidence
- canonical `scripts/build.ps1`: PASS, exit 0.
- `PinkCab.Core.*`: 4 tests discovered, 4 Success, 0 Fail, automation exit 0.
- authority guard: PASS.
- `git diff --check`: PASS.
- public Core vendor-header scan: 0 hits.
- StableId UObject/Actor identity scan: 0 hits.

## Contracts delivered
- `FPinkCabStableId`: deterministic opaque logical ID equality + text round-trip.
- `FPinkCabSchemaVersion`: explicit semantic version parsing/comparison with invalid-input rejection.
- `FPinkCabResult`: explicit success/failure + error vocabulary.
- `FPinkCabFeatureConfig`: Core-owned boundary for `L2 / Damage / Neural / MovingFuel / ServiceNodes`.

No paid dependency or production vehicle solver was added.
