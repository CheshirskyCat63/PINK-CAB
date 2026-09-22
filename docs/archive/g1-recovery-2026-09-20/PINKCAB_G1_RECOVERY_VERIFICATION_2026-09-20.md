# PINK CAB G1 Recovery Verification Report — 2026-09-20

Status: VERIFIED RECOVERY LEDGER  
Owner: CD-865  
Source snapshot: `8a70dfa5f4120fce63ab6f0cfc9235128987da77`

## Executive result

- **Administrative readiness to continue development: 100%.** CD-867 entry reconciliation is DONE; current authorities/worktree/dirty-state/PR/gate order are durable.
- **Structural recovery of the intended G1 planning package: 100%.** All declared task/requirement/asset identities and the missing contract chapters needed to resume execution are durably materialized.
- **Byte-for-byte recovery of the never-materialized original ZIP: impossible by definition.** The original `PINKCAB_G1_Plan.zip`, original generated `tasks.json`, `assets.json` and original `sources/` directory were not found as completed artifacts.
- **Verbatim long-body task fidelity is explicitly incomplete:** 5/83 task bodies have exact/partial long-body text materialized; the remaining 78 retain exact identity/dependencies/gate/scope plus contract/source pointers and are marked structural-only instead of being invented.

These four statements are deliberately separate. Administrative readiness and structural recovery can be 100% while byte-identical artifact recovery remains impossible.

## Counted recovery matrix

| Component | Target | Durable recovered | Structural coverage | Verbatim/detail coverage | Evidence |
|---|---:|---:|---:|---:|---|
| Task identities | 83 | 83 | **100%** | 100% identity/deps/gate/scope | `docs/recovery/tasks.json` |
| Task dependency targets | all | 0 missing | **100%** | n/a | tasks.json validation |
| Task dependency cycles | 0 | 0 | **100%** | n/a | tasks.json validation |
| Detailed task long bodies | 83 | 5 exact/partial, 78 structural-only | 100% represented | **6.02% exact/partial** | per-record `body_recovery_state` |
| Requirements | 42 | 42 | **100%** | **100% row-level traceability** | requirements JSON / Confluence 48B |
| Asset records | 159 | 159 | **100%** | **100% detailed rows** | `docs/recovery/assets.json` |
| Asset families | 12 | 12 | **100%** | 100% counts | assets.json validation |
| Missing target chapters from interrupted package | 8 | 8 | **100%** | **100% recovered/stiched from source chunks** | docs/recovery/chapters 03–08,10,12 |
| Separate Sol 5.6 handoff | 1 | 1 | **100%** | 100% | handoff recovery |
| Source/evidence index | required | 32 indexed sources | **100% as authority map** | original binary/source snapshot fidelity varies | `docs/recovery/sources/index.json` |
| Gate ladder | 11 gates | 11 | **100%** | 100% | chapter 07 + execution ledger |
| QA scenarios | QA-G1-001..034 | 34 | **100% structurally** | 100% in recovered chapter 07 | chapter 07 |
| Original ZIP binary | 1 | 0 | not applicable | **0% byte-identical** | original never found/materialized |

## Registry validation

### tasks.json

- records: 83
- unique IDs: 83
- expected: 83
- missing dependency targets: 0
- dependency cycles: 0
- exact/partial task-body records: 5
- structural-only records: 78
- Git blob SHA: `582265e5b4c0ed4390c2893e6053d288c45f376d`

The exact queue/dependency/gate/scope structure is complete. Missing task-specific long-body fields are null/structural-only; no acceptance text was fabricated.

### assets.json

- records: 159
- unique IDs: 159
- family counts:
  - MOD-CAR 23
  - ANIM 10
  - ROAD 27
  - SIGN 9
  - MARK 4
  - WORLD 17
  - TRAFFIC 6
  - MAT 16
  - SND 30
  - VFX 7
  - UI 4
  - DATA 6
- family-count validation: PASS
- Git blob SHA: `7d715f46acce1edd75faee6f15fb7cd9a947bd22`

Rights and inventory states remain fail-closed until evidence. Marathon remains rights-blocked.

### source map

- source records: 32
- unique IDs: 32
- required index fields missing: 0
- Git blob SHA: `23d3e931c4020cc628c69843201f2a6f2fb0bf4b`

Local/private sources are indexed by locator/state, not copied into Git. Historical snapshots are distinguished from current authority.

## Recovered chapters

Verified fetched back from recovery branch:

1. `03_INPUT_AND_CABIN_MATRIX.md`
2. `04_VEHICLE_CAMERA_AUDIO.md`
3. `05_CITY_LEVEL1.md`
4. `06_ASSETS_AND_LICENSES.md`
5. `07_GATES_AND_QA.md`
6. `08_ADMIN_AND_RELEASE.md`
7. `10_ASSET_REGISTER.md`
8. `12_DETAILED_PROTOCOLS.md`

The 159-row asset register and detailed protocols are present; the original gate order has been restored to the execution ledger.

## Development-entry proof

Current durable entry snapshot:

- worktree: `E:\CHESHIRE_DIVISION\Games\PINK-CAB\.worktrees\cd848-human-gate`
- branch: `fix/CD-848-human-gate-regressions`
- head/upstream at preflight: `8a70dfa5f4120fce63ab6f0cfc9235128987da77`
- tracked dirty files observed: 0
- untracked `.superpowers/`: preserve
- PR #5: open/draft
- current mechanics authority: Confluence page 16744449
- CD-867: DONE

Therefore **administrative entry readiness = 100%** for continuing development, while runtime gates remain evidence-bound.

## Current runtime gate — T-BASE

Fixed denominator = 10.

Fresh PASS:
1. exact source snapshot
2. worktree isolation
3. dirty-state protection
4. code-health
5. UE 5.8.2 authority
6. PinkCabEditor build

Unverified due connector quota:
7. full PinkCab automation final result
8. Win64 package
9. packaged smoke/critical scan
10. build manifest + desktop shortcuts

**T-BASE = 6/10 = 60%.**

This is an access blocker, not a project-code failure. T-MOVE must not start until the existing T-BASE run is inspected/finished and T-BASE is either green or systematically debugged.

## What is permanently unrecoverable unless another original artifact appears

- byte-identical `PINKCAB_G1_Plan.zip`;
- byte-identical originally generated tasks/assets JSON files;
- original exact source-snapshot folder layout/checksums if those files never materialized;
- exact long-body wording for task records that File Library did not expose in retrievable chunks.

These losses do not prevent controlled development because task identity/dependency/gates, contract chapters, requirements, assets, authority map, QA matrix and handoff are durable.

## Completion criterion

CD-865 is complete when this report is durably stored and linked from Jira/Confluence/Git. CD-866 then packages the durable recovery set with SHA256 manifest; CD-868 performs final admin closeout.
