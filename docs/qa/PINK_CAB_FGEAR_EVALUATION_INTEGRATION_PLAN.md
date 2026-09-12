# PINK CAB · FGear Evaluation & Integration Plan

Date: 2026-09-12
Program: `CD-784`
Upstream completed gate: `CD-771 / PF-16`
Canonical base: `f7fa6f976fa1fa1e83acff9287e92588f0d0efce`
State: `ZERO_COST_EVALUATION_FIRST`

## Fixed execution rules

- PRE-FGEAR program is complete; do not reopen PF-00..PF-16 for new implementation.
- Spend remains EUR 0 until `CD-782` receives explicit owner approval.
- Use only official seller/Fab demos, docs, videos, Example Project and an official evaluation build if offered.
- Never extract plugin binaries/source from packaged demos and never use mirrors or pirated distributions.
- Production `main` must continue to build/package without FGear until an official plugin integration branch is authorized.
- FGear remains sole production hero-Tatra road-dynamics owner.
- Do not build a Chaos/custom production substitute.
- VDS remains deferred until FGear integration is green and deformation becomes the next verified blocker.

## Task pack

| Order | Jira | Deliverable |
| --- | --- | --- |
| 1 | `CD-777` | Official FGear evaluation intake + license boundary |
| 2 | `CD-778` | Isolated non-production Evaluation Lab |
| 3 | `CD-779` | FGear ↔ PINK CAB adapter mapping report |
| 4 | `CD-780` | Tatra FGear calibration matrix + acceptance targets |
| 5 | `CD-781` | Official plugin smoke + provider integration gate |
| 6 | `CD-782` | Purchase decision + owner approval gate |
| Deferred | `CD-783` | VDS evaluation gate after FGear integration |

## Dependency graph

`CD-771 DONE -> CD-784`

`CD-777`, `CD-779`, and `CD-780` must complete before `CD-781` can become executable.
`CD-781` blocks `CD-782` unless `CD-777` proves that no official installable evaluation build exists; in that case the absence itself is recorded as purchase-gate evidence.
`CD-781` also blocks `CD-783`: VDS cannot advance before FGear provider integration is green.

`CD-778` may run in parallel with mapping/calibration using only legally available official evaluation artifacts.

## Zero-cost evaluation boundary

Official packaged demos are behavior references, not a source of production plugin binaries.
The Example Project may be inspected for content/setup patterns, but it becomes executable only with an officially supplied FGear plugin.
If the vendor supplies an official internal evaluation build/license, install it only in the Evaluation Lab/integration branch and record its license limits.
If no installable trial exists, continue docs/demo/calibration work until the first executable provider test is the only remaining blocker.

## Provider integration contract

The existing `IPinkCabVehicleDynamicsProvider` remains the only PINK CAB-facing dynamics boundary.
The FGear adapter may translate controls, configuration and telemetry, but it must not take ownership of taxi, economy, persistence, passenger logic, Vehicle Health, UI or world generation.
Wallride logic emits an external constraint request; the future FGear adapter applies the approved dynamics-side operation without creating a second solver.
## Purchase stop condition

No paid transaction is allowed merely because PF-16 is green.
A purchase request is raised only when:

1. vendor-independent prerequisites remain green;
2. official free evaluation work is exhausted or an evaluation plugin proves compatibility;
3. FGear is still the sole missing production road-dynamics provider;
4. exact UE-version compatibility and product source are recorded;
5. the owner explicitly approves the purchase.

After approval, acquire FGear through the official channel, install it for the pinned UE line, implement the thin adapter, then rerun build, complete `PinkCab.*` automation, package, Tatra fixtures and canonical map acceptance before merging.

## Evidence rule

Every task closes with exact source URLs/artifact versions, engine version, Git SHA, commands/tests run and pass/fail evidence. Unknown vendor API or calibration values remain explicit unknowns until verified; they are never promoted to canon by implementation convenience.

This plan does not authorize purchase and does not install FGear or VDS.