# PINK CAB · PRE-FGEAR Execution Index

**Status:** DEDICATED REPOSITORY ESTABLISHED / CD-558 CLOSURE EVIDENCE PENDING
**Date:** 2026-09-11
**Game Studio product lane:** Jira `CD-519`
**Repository cut-over gate:** Jira `CD-558`
**Target production repository:** `CheshirskyCat63/PINK-CAB`
**Migration source:** `CheshirskyCat63/DEADRACE`
**Authority base in migration source:** `a24d697a494aa81b0639a9f3149dfc5a2f1c7308`
**Staging/admin branch in migration source:** `dev/pinkcab-pre-fgear-20260911`
**Program:** Jira `CD-754`
**Execution Confluence:** page `12222532`
**Repository authority Confluence:** page `12451841`

## Repository truth

`CheshirskyCat63/PINK-CAB` is the dedicated PINK CAB execution repository. `DEADRACE` is legacy/migration-source only and must not receive new PINK CAB runtime implementation.

PF-00 remains blocked until `CD-558` is closed with exact target-SHA and verification evidence.

Mechanical migration is defined by `docs/PINK_CAB_REPOSITORY_CUTOVER.md`.

## Git documents to carry forward

- Design: `docs/superpowers/specs/2026-09-11-pink-cab-pre-fgear-development-design.md`
- Implementation plan: `docs/superpowers/plans/2026-09-11-pink-cab-pre-fgear-development-plan.md`
- Execution index: `docs/PINK_CAB_PRE_FGEAR_EXECUTION_INDEX.md`
- Repository cut-over manifest: `docs/PINK_CAB_REPOSITORY_CUTOVER.md`

## Jira map

| Package | Jira | Purpose |
| --- | --- | --- |
| Product lane | `CD-519` | PINK CAB Game Studio product root |
| Repo cut-over | `CD-558` | create/migrate dedicated `PINK-CAB` repo |
| Program | `CD-754` | zero-paid-assets development corridor |
| PF-00 | `CD-755` | Development Bootstrap |
| PF-01 | `CD-756` | Core Contracts |
| PF-02 | `CD-757` | Input + Cabin Interaction |
| PF-03 | `CD-758` | Tatra Logical Vehicle Contracts |
| PF-04 | `CD-759` | Vehicle Health Foundation |
| PF-05 | `CD-760` | Taxi + Passenger Logical Foundation |
| PF-06 | `CD-761` | Economy + Exactly-Once Transactions |
| PF-07 | `CD-762` | Persistence Foundation |
| PF-08 | `CD-763` | CityCode + Road Graph Foundation |
| PF-09 | `CD-764` | Logical Traffic Foundation |
| PF-10 | `CD-765` | Level 1 Greybox Zero |
| PF-11 | `CD-766` | Wallride State Logic |
| PF-12 | `CD-767` | Automotive ServiceNodes Foundation |
| PF-13 | `CD-768` | Moving Refueling Logic |
| PF-14 | `CD-769` | Enforcement + Neural Foundation |
| PF-15 | `CD-770` | Pre-FGear QA / Evidence Gate |
| PF-16 | `CD-771` | FGear Purchase Gate |

## Execution dependency

Jira `Blocks` links are authoritative for task gating.

Repository entry gate:

`CD-558 repo cut-over -> PF-00`

Development path:

`PF-00 -> PF-01 -> input/vehicle/taxi/economy -> persistence -> CityCode -> traffic/greybox -> wallride/services/refuel/neural -> PF-15 -> PF-16`.

PF-00 may not begin in any repository until `CD-558` passes. PF-16 may not begin until PF-15 passes.

## Zero-paid-assets rule

Before PF-16:

- paid dependencies introduced by this program: **0**;
- temporary production hero-vehicle solver: **forbidden**;
- temporary production deformation replacement: **forbidden**;
- speculative content/asset purchase: **forbidden**;
- built-in primitives and synthetic test doubles: **allowed only for contract/greybox verification**.

## First purchase gate

FGear is purchased only after PF-15 evidence is green and PF-16 proves that the next acceptance test is blocked solely by the absence of the approved production `IPinkCabVehicleDynamicsProvider` implementation.

## Immediate handoff boundary

Repository creation is complete. The remaining `CD-558` closure sequence is:

1. verify this target repository authority and record exact SHA;
2. freeze the DEADRACE PINK CAB staging branch as migration history;
3. close `CD-558` with evidence;
4. then begin PF-00 in this dedicated repository.

The target repository was created by the product owner. No credential extraction or cross-project repository reuse was used.

## Truth rule

This index certifies planning/execution administration only. It does not mark the repository cut-over or PF-00 implemented, does not change the current BASE-100 score, and does not convert any OPEN owner row to LOCKED.
