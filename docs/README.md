# PINK CAB · Documentation Index

**Status:** CURRENT / PROTECTED INTEGRATION / P4 + RIG06 OWNER ACCEPTED / VEHICLE FEEL 90 ACTIVE
**Active product:** Jira `CD-519`  
**Canonical Git:** `CheshirskyCat63/PINK-CAB` → `main`  
**Current mechanics owner:** `CD-848`; performance lane `CD-641`
**Control-plane cleanup:** `CD-868` — DONE; historical P04 admin closeout `CD-952` — DONE
**Owner-accepted fallback:** `accepted/p4-rig06-20261007` / `6edea7747d3a8433188c9fb394b98ae9c320d49b`

## Start here

1. [`CONTROL_PLANE.md`](CONTROL_PLANE.md) — one execution path and delivery lanes.
2. [`AUTHORITY.yaml`](AUTHORITY.yaml) — machine-readable current authority.
3. [`PINK_CAB_ACTIVE_BASELINE.md`](PINK_CAB_ACTIVE_BASELINE.md) — concise product/runtime baseline.
4. [`PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`](PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md) — current vehicle/input mechanics.
5. [`PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md`](PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md) — sole production vehicle stack.
6. [`PINK_CAB_BASE100_CODE_ARCHITECTURE.md`](PINK_CAB_BASE100_CODE_ARCHITECTURE.md) — implementation architecture.
7. [`FIRST_EURO_SCOPE.md`](FIRST_EURO_SCOPE.md) — first-year product boundary.
8. [`PROGRAM_ROADMAP.md`](PROGRAM_ROADMAP.md) — current execution order.
9. [`PINK_CAB_VEHICLE_FEEL_90.md`](PINK_CAB_VEHICLE_FEEL_90.md) — immediate hero-car calibration gate.
10. [`VERIFICATION_MATRIX.md`](VERIFICATION_MATRIX.md) — executable evidence.
11. [`OPEN_DECISIONS.md`](OPEN_DECISIONS.md) — unresolved design locks only.

## Current development state

The recovery/admin freeze is over. Gameplay work is allowed.

- `main` is the only integration branch.
- P4 is owner accepted and retained as immutable rollback evidence; it is not reopened by feel calibration.
- RIG06 presentation is accepted for internal development on top of the closed P4 core.
- Immediate work is `Vehicle Feel 90`; R01 world/streaming and every broader gameplay lane wait behind its owner gate.
- New work uses one short-lived branch → one PR → `verify.yml` → merge.
- Native Unreal Engine 5.8 Chaos Vehicles is the sole production hero-car road-dynamics owner.
- FGear/VDS are archived research only and must never be treated as current dependencies.
- Code-health debt baseline is zero.
- `scripts/platform-status.ps1` is the local environment preflight; `scripts/vehicle-feel-guard.ps1` prevents the vehicle-calibration lane from drifting into world/taxi/etc.

## Delivery lanes

Normal verification is `.github/workflows/verify.yml`: hosted static contracts first, then an exact-head `PinkCabEditor` build and the focused playable runtime suite on the PINKCAB self-hosted runner.

Human delivery is `.github/workflows/deliver.yml`. Its single entry point is `scripts/deliver.ps1`, which requires a clean exact HEAD, builds the editor, runs the focused runtime suite, packages with BuildCookRun, smoke-tests the package, and only then atomically publishes `PINCKCAB_BUILD` plus the `PINCKCAB.lnk` shortcut.

The old CD-648/CD-869/P00–P04 workflow graph, causality evidence programs, calibration fixtures and road-authoring gates are historical evidence only. They are not active build, verification or delivery authority.

Documentation-only administration does not require replacing the runtime package. PINK CAB and KUKURUZA registrations share one physical host; coordinate heavy Unreal work without stopping another project's processes.

## Current execution checkpoint

Accepted fallback: `accepted/p4-rig06-20261007` / `6edea77`. The simplified P4 vehicle/input core is closed; RIG06 is the accepted internal presentation shell.

Current execution: **Vehicle Feel 90**. Finish the car’s measurable steering, drivetrain, mass/CoM/inertia, suspension, tyre/brake, wet/storm, load and runtime behavior to a representative 80–90% before resuming R01. See `PINK_CAB_VEHICLE_FEEL_90.md`.

Historical accepted packages and runs remain evidence/fallbacks only. Archive tags under `archive/20261006/*` preserve the old research branch tips and dirty P04 states.

## Broader FIRST EURO execution corridor (after current physics priorities)

The current finite Mechanics Freeze queue is owned by `CD-848`:

`CD-869 → CD-870 → CD-871 → CD-872 → CD-873 → CD-874 → CD-875 → CD-876 → CD-877 → CD-878 → CD-879`

These close, in order: representative L1↔L2 route/streaming; pickup/dropoff + parking; garage/parts/repair; moving refuel; wet-weather driving; bounded traffic/incidents; repeat-client persistence; daughter bounded role; minimal Neural; payment receipt/fines; terminal evidence audit.

## Asset provenance

The owner identified the Tatra 613 download as the Sketchfab model `c554d6fdaf8749e291b25cbf487f82f8`, **Tatra 613 1975-1996**, uploaded by **Mercedesiarz_2025 (@szymonpasterczyk)**. Its page declares **CC BY 4.0**, which permits commercial sharing and adaptation subject to attribution and the other licence terms. Source evidence, a base attribution notice and the remaining mapping checks are recorded in [`provenance/tatra613-sketchfab-c554d6f.json`](provenance/tatra613-sketchfab-c554d6f.json) and `ASSET_LICENSE_LEDGER.csv` as `TATRA-613-SKETCHFAB-C554D6F = SOURCE_LICENSE_VERIFIED_ASSET_MAPPING_PENDING`.

The older `TATRA-DONOR-ARCHIVE = BLOCKED_NO_COMMERCIAL_PERMISSION` record concerns the **TM-Modding / Assetto Corsa Marathon** source family described in Confluence archive 48C. That restriction must not be assigned automatically to the distinct Sketchfab 613, and the Sketchfab licence must not be assigned automatically to Marathon or other unidentified components. The earlier wildcard attribution of all `Tatra613*` imports to one restricted donor was not established by the cited archive.

`CD-855` still owns source-to-authored-scene and current/historical LFS mapping, including the separately required V12Clean wheel, applicable release attribution and game-specific import/presentation acceptance. No current or historical binary receives blanket public/commercial clearance from this documentation correction. These checks do **not block internal gameplay development**.

## Truth rule

`CANON → SPECIFIED → IMPLEMENTED → VERIFIED`

Confluence = durable product/design authority. Jira = live work/dependencies/evidence. Git = code/tests/build history. Exact executable evidence decides runtime truth.
