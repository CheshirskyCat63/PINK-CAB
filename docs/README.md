# PINK CAB · Documentation Index

**Status:** CURRENT / PROTECTED INTEGRATION / P04 CANDIDATE HUMAN ACCEPTED / ADMIN CD-952
**Active product:** Jira `CD-519`  
**Canonical Git:** `CheshirskyCat63/PINK-CAB` → `main`  
**Current mechanics owner:** `CD-848`  
**Control-plane cleanup:** `CD-868` — DONE  
**Owner-accepted runtime baseline:** `52239b61bc80e5a63716b9b09b87c520fa09fd05` / delivery `37149462470` attempt 1

## Start here

1. [`CONTROL_PLANE.md`](CONTROL_PLANE.md) — one execution path and delivery lanes.
2. [`AUTHORITY.yaml`](AUTHORITY.yaml) — machine-readable current authority.
3. [`PINK_CAB_ACTIVE_BASELINE.md`](PINK_CAB_ACTIVE_BASELINE.md) — concise product/runtime baseline.
4. [`PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`](PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md) — current vehicle/input mechanics.
5. [`PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md`](PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md) — sole production vehicle stack.
6. [`PINK_CAB_BASE100_CODE_ARCHITECTURE.md`](PINK_CAB_BASE100_CODE_ARCHITECTURE.md) — implementation architecture.
7. [`FIRST_EURO_SCOPE.md`](FIRST_EURO_SCOPE.md) — first-year product boundary.
8. [`PROGRAM_ROADMAP.md`](PROGRAM_ROADMAP.md) — current execution order.
9. [`VERIFICATION_MATRIX.md`](VERIFICATION_MATRIX.md) — executable evidence.
10. [`OPEN_DECISIONS.md`](OPEN_DECISIONS.md) — unresolved design locks only.

## Current development state

The recovery/admin freeze is over. Gameplay work is allowed.

- `main` is the only integration branch.
- PR #52/P03 and PR #62/bounded P04 are integrated; CD-641 owns remaining P04 engineering. PR #47 is a separate tyre diagnostic.
- New work uses one short-lived Jira-keyed task branch → one PR → verification → merge.
- Native Unreal Engine 5.8 Chaos Vehicles is the sole production hero-car road-dynamics owner.
- FGear/VDS are archived research only and must never be treated as current dependencies.
- Code-health debt baseline is zero.
- Accepted runtime is bounded P04 52239b61; previous V2edf75e1b and P02 remain retained. Administrative commits do not rename binaries.

## Delivery lanes

Normal coordinator: `.github/workflows/pinkcab-repository-verification.yml` — complete scope, repository checks, exact-source physics, same-run evidence plus five independent slope repeats, then aggregate technical gate. Main requires both `Repository verification` and `Gameplay acceptance gate`.

Human delivery is explicit: `cd648-p02-phy009.yml` with `deliver_human=true` on the reviewed candidate ref, then its verified `cd869-deliver.yml` call. Record the full SHA/run/attempt. The route checks packaged map/four-wheel/material smoke, installs to a unique SHA/run/attempt directory, compares payload paths/sizes/SHA256, verifies an interactive window and atomically replaces only `PINCKCAB.lnk`, retaining prior link and accepted package. Delivery initially records HUMAN_PENDING; owner acceptance is a subsequent separate record.

Retained G1 fast/human_gate/release_gate and inline definitions are auxiliary/history, not competing routine entry points. Policy permits only P02-mediated owner-test delivery. The legacy direct CD-869 workflow_dispatch trigger still exists and does not verify upstream P02 attestation; direct dispatch is prohibited by policy, not technically prevented. Closing that trigger/attestation gap remains CD-559 engineering. The retired local fast-delivery script refuses mutation. Full clean-source/full-project/packaged-input qualification remains CD-559 engineering. Cached-package source-marker/executable-presence checks are not a complete cache-provenance certificate.

Documentation-only administration does not require replacing the runtime package. PINK CAB and KUKURUZA registrations share one physical host; coordinate heavy Unreal work without stopping another project's processes.

## Current execution checkpoint

Accepted P04 measured gearing runtime: **52239b61bc80e5a63716b9b09b87c520fa09fd05**, delivery **37149462470**, attempt 1; explicitly owner accepted on 2026-10-03. PR #62 integrated it as **4a313d38f0674a3e5048f832a07428defa31ab62**. Read GitHub for later main commits; integration/admin commits never rename accepted executable bytes.

Previous accepted fallbacks: P03/V2 `edf75e1b` / delivery `37117735294`, and P02 `8d68e456` / delivery `36868646970`. Road R1-R5 and no-assist input grammar remain frozen. Sole root desktop game entry: `PINCKCAB`; studio entries are Editor and explicitly named Rollback_V2.

Coordinator `37148042881`, gated delivery `37149462470` and installed audit `37151172013` passed: 79 script tests, complete ControlRuntime, 63 physics tests, 240 D3 cases, five slope repeats and 53 installed payload files. Additive owner decision is `OWNER_ACCEPTANCE.json`, retained by run `37152180059`; original HUMAN_PENDING handoff receipts remain unchanged historical evidence. Administrative closeout: CD-952.

CD-648 remains the single vehicle umbrella. CD-641 owns remaining P04 performance calibration and the non-monotonic intermediate-input observation; P05-P11 and full FIRST EURO remain unfinished. CD-559 retains clean-source/full-project/packaged-input engineering, not an absent P03 acceptance. PR #47/CD-650 remains diagnostic only. CD-855 placeholder polish/replacement is deferred until vehicle calibration and does not block internal physics/admin; public asset rights stay separate.

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
