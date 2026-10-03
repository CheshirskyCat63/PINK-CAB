# PINK CAB · Documentation Index

**Status:** CURRENT / PROTECTED INTEGRATION / P03 CORRECTION OPEN
**Active product:** Jira `CD-519`  
**Canonical Git:** `CheshirskyCat63/PINK-CAB` → `main`  
**Current mechanics owner:** `CD-848`  
**Control-plane cleanup:** `CD-868` — DONE  
**Owner-accepted runtime baseline:** `8d68e456d1944be295281535cf9fd103ecf05d52` / Actions run `36868646970`

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
- PR #52 is the active P03 corrective implementation; PR #47 remains a separate draft tire diagnostic.
- New work uses one short-lived Jira-keyed task branch → one PR → verification → merge.
- Native Unreal Engine 5.8 Chaos Vehicles is the sole production hero-car road-dynamics owner.
- FGear/VDS are archived research only and must never be treated as current dependencies.
- Code-health debt baseline is zero.
- The accepted vehicle/control runtime remains `8168d724…`; later sanitation/CI commits do not retroactively rename that binary.

## Delivery lanes

The canonical workflow is `.github/workflows/pinkcab-g1-github-control-plane.yml`.

- **fast** — normal engineering iteration: exact-head preflight, zero-debt checks, standalone Game build/sign, cooked-base overlay, packaged Windows runtime/input smoke.
- **human_gate** — same lightweight code-only dev-build path, delivered as `PINKCAB Latest.lnk`; no full recook and no machine-level signing requirement. If any change since the proven cooked base changes cook-sensitive `Content/`, `Config/`, `Plugins/` or `.uproject`, this lane fails closed and requires `release_gate`. Result is technical PASS + `HUMAN_PENDING`.
- **release_gate** — expensive release evidence only: Editor modules, full automation, fresh cook/package and packaged runtime. Windows machine-level trust / external trusted signing belongs here, not in ordinary development.

Smart App Control / UMCI is therefore a **release-host infrastructure boundary**, not a gameplay acceptance criterion for everyday PINK-CAB development.

## Current execution checkpoint

- Integration: protected `main`; exact current SHA is read from GitHub, not inferred from an old document.
- Accepted runtime: P02 `8d68e456d1944be295281535cf9fd103ecf05d52`, run `36868646970`, accepted 2026-10-01 and integrated by PR #49. P00-P02 and road R1-R5 remain frozen.
- Active gameplay correction: PR #52 / CD-649 + CD-659. P03 is HUMAN REJECTED; P04 is BLOCKED until corrective automation, packaged delivery and renewed owner acceptance.
- PR #47 / CD-650 is a separate draft tire diagnostic, not an accepted calibration or next road stage.
- Infrastructure: CD-559 remains IN PROGRESS. PR #53 integrated CI trust/scope repairs and explicit delivery control; full regression and release reproducibility are not thereby certified.
- Preserved preparation branches: the bootstrap branch contains integrated local candidate `86c2da3`; earlier test snapshot `4554285` and separate P03 snapshot `95dafd4` remain historical evidence. These candidates are unaccepted and must be reconciled into existing PR #52 before renewed gameplay acceptance. Pure build/CI preparation is maintained separately from gameplay changes.
- Preparation evidence: 436/446 latest selected test outcomes passed on the bootstrap branch; 10 failed. Separate P03 correction: 47/47 physics and 30/30 control tests passed. These are different source trees and must not be added together as full-suite proof.

## Broader FIRST EURO execution corridor (after current physics priorities)

The current finite Mechanics Freeze queue is owned by `CD-848`:

`CD-869 → CD-870 → CD-871 → CD-872 → CD-873 → CD-874 → CD-875 → CD-876 → CD-877 → CD-878 → CD-879`

These close, in order: representative L1↔L2 route/streaming; pickup/dropoff + parking; garage/parts/repair; moving refuel; wet-weather driving; bounded traffic/incidents; repeat-client persistence; daughter bounded role; minimal Neural; payment receipt/fines; terminal evidence audit.

## Asset provenance

The current donor Tatra is valid for internal development evidence only. Commercial modification/redistribution permission is not proven. `docs/ASSET_LICENSE_LEDGER.csv` records `TATRA-DONOR-ARCHIVE = BLOCKED_NO_COMMERCIAL_PERMISSION`. This is owned by `CD-855` and blocks public/commercial use of that donor asset, **not internal gameplay development**.

## Truth rule

`CANON → SPECIFIED → IMPLEMENTED → VERIFIED`

Confluence = durable product/design authority. Jira = live work/dependencies/evidence. Git = code/tests/build history. Exact executable evidence decides runtime truth.
