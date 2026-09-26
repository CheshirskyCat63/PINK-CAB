# PINK CAB · Vehicle Physics source-package convergence map · 2026-09-26

## Source package

- Uploaded package: `PINKCAB_PHYSICS_DETAILED_PLAN_20260926.zip`
- SHA-256: `b0e6b17f2013595084277cd48486507af10b897c0dc5b7516189c434af039ca7`
- Package task registry: 48 PHY rows.
- Package test registry: 96 rows.
- Source package status: **INPUT / RECOVERED PLANNING EVIDENCE**, not an independent live authority.

The package was fully reviewed against current Git/Jira/Confluence. Its useful content is migrated into the single canonical Vehicle Physics program. Verbatim stale snapshots are intentionally not promoted to live authority.

## File-by-file disposition

| Uploaded file | Canonical destination / disposition |
|---|---|
| `00_README.md` | absorbed by `docs/vehicle_physics/README.md` and this convergence map |
| `01_AUDIT_AND_CONFLICTS.md` | absorbed by the canonical P00–P11 program + parameter ledger + Jira/Confluence convergence |
| `02_SCOPE_AND_LOCKS.md` | absorbed by the canonical program's non-negotiable mechanic locks and city/world exclusion |
| `03_MASTER_IMPLEMENTATION_PLAN.md` | absorbed by the canonical P00–P11 / PHY-001..048 program |
| `04_ENGINE_AND_DRIVETRAIN.md` | P01/P02/P04 + parameter ledger |
| `05_HANDLING_CALIBRATION.md` | P03/P05/P06/P07 |
| `06_TATRA_PROFILES_AND_CABIN.md` | P09/P10 |
| `07_GATES_AND_DELIVERY.md` | P11 + `TESTS.csv` |
| `08_ADMIN_AND_FULL_AUDIT.md` | `ADMIN_AND_HANDOFF_2026-09-26.md` + Jira/Confluence authority cleanup |
| `09_SOURCE_REGISTER.md` | this file + parameter provenance ledger |
| `10_AGENT_HANDOFF.md` | `ADMIN_AND_HANDOFF_2026-09-26.md` |
| `MANIFEST_SHA256.json` | package provenance retained here through package SHA; live source revisions are recorded in Git/Jira/Confluence |
| `TASKS.csv` | normalized into canonical `TASKS.csv`; stale FGear-era owner references were not reintroduced |
| `TESTS.csv` | normalized into canonical `TESTS.csv`; every row remains `NOT RUN` |
| `sources/AUTHORITY.yaml` | treated as historical source snapshot; current Authority Index + CD-843/CD-648/CD-921 win |
| `sources/CD-649.md` | historical Jira snapshot; live CD-649 wins |
| `sources/CD-659.md` | historical Jira snapshot; live CD-659 wins |
| `sources/CD-722.md` | historical snapshot; live CD-722 was repaired to Native Chaos authority |
| `sources/CD-848.md` | historical snapshot; live CD-848 + current comments win |
| `sources/CONFLUENCE_5963818.md` | historical page snapshot; live page 5963818 wins |
| `sources/GIT_README.md` | historical repository snapshot; current `main` wins |
| `sources/PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md` | current control contract remains authoritative; package copy is evidence only |
| `sources/PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md` | reconciled with CD-843 / current Native Chaos authority |

## Authority after convergence

1. **Git technical truth:** `main:docs/vehicle_physics/` plus exact runtime source.
2. **Jira execution truth:** CD-648 is the single Vehicle Physics execution umbrella; existing subsystem cards remain domain owners. CD-921 is terminal integration/evidence only.
3. **Confluence durable program:** page 22413538. Page 22413517 is archived as a superseded duplicate.
4. **Runtime truth:** exact executable/profile/telemetry/tests. No PHY runtime test is considered passed by this import.

## World freeze

The current city, MetaRoad and accepted R1/R2/R3 world behavior are frozen outside this workstream. No vehicle-physics task may modify world geometry, road mechanics or city-generation behavior unless the owner explicitly reopens that scope.
