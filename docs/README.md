# PINK CAB · Documentation Index

**Status:** CURRENT / DEVELOPMENT-READY CONTROL PLANE  
**Active product:** Jira `CD-519`  
**Canonical Git:** `CheshirskyCat63/PINK-CAB` → `main`  
**Current mechanics owner:** `CD-848`  
**Control-plane cleanup:** `CD-868` — DONE  
**Owner-accepted runtime baseline:** `8168d72406af6934ab20eace583c2b895f0620b7` / Actions run `35809749568`

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
- There are no active implementation PRs at this baseline.
- New work uses one short-lived Jira-keyed task branch → one PR → verification → merge.
- Native Unreal Engine 5.8 Chaos Vehicles is the sole production hero-car road-dynamics owner.
- FGear/VDS are archived research only and must never be treated as current dependencies.
- Code-health debt baseline is zero.
- The accepted vehicle/control runtime remains `8168d724…`; later sanitation/CI commits do not retroactively rename that binary.

## Delivery lanes

The canonical workflow is `.github/workflows/pinkcab-g1-github-control-plane.yml`.

- **fast** — normal engineering iteration: exact-head preflight, zero-debt checks, standalone Game build/sign, cooked-base overlay, packaged Windows runtime/input smoke.
- **human_gate** — same lightweight code-only dev-build path, delivered as `PINKCAB Latest.lnk`; no full recook and no machine-level signing requirement. If the latest merge changes cook-sensitive `Content/`, `Config/`, `Plugins/` or `.uproject`, this lane fails closed and requires `release_gate`. Result is technical PASS + `HUMAN_PENDING`.
- **release_gate** — expensive release evidence only: Editor modules, full automation, fresh cook/package and packaged runtime. Windows machine-level trust / external trusted signing belongs here, not in ordinary development.

Smart App Control / UMCI is therefore a **release-host infrastructure boundary**, not a gameplay acceptance criterion for everyday PINK-CAB development.

## FIRST EURO execution corridor

The current finite Mechanics Freeze queue is owned by `CD-848`:

`CD-869 → CD-870 → CD-871 → CD-872 → CD-873 → CD-874 → CD-875 → CD-876 → CD-877 → CD-878 → CD-879`

These close, in order: representative L1↔L2 route/streaming; pickup/dropoff + parking; garage/parts/repair; moving refuel; wet-weather driving; bounded traffic/incidents; repeat-client persistence; daughter bounded role; minimal Neural; payment receipt/fines; terminal evidence audit.

## Asset provenance

The current donor Tatra is valid for internal development evidence only. Commercial modification/redistribution permission is not proven. `docs/ASSET_LICENSE_LEDGER.csv` records `TATRA-DONOR-ARCHIVE = BLOCKED_NO_COMMERCIAL_PERMISSION`. This is owned by `CD-855` and blocks public/commercial use of that donor asset, **not internal gameplay development**.

## Truth rule

`CANON → SPECIFIED → IMPLEMENTED → VERIFIED`

Confluence = durable product/design authority. Jira = live work/dependencies/evidence. Git = code/tests/build history. Exact executable evidence decides runtime truth.
