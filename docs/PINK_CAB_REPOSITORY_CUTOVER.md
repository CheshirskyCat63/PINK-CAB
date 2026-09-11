# PINK CAB · Dedicated Repository Cut-over Manifest

**Status:** COMPLETE / VERIFIED ADMINISTRATIVE CUT-OVER
**Date:** 2026-09-11
**Jira product lane:** `CD-519`
**Cut-over gate:** `CD-558`
**PRE-FGEAR program:** `CD-754` / `CD-755..CD-771`
**Source repository:** `CheshirskyCat63/DEADRACE`
**Source staging branch:** `dev/pinkcab-pre-fgear-20260911`
**Target production repository:** `CheshirskyCat63/PINK-CAB`

## 1. Authority rule

`CheshirskyCat63/PINK-CAB` becomes the sole technical source of truth for PINK CAB after this manifest is executed and verified.

`DEADRACE` is not a second PINK CAB production repository. It is migration-source / legacy-salvage history only after cut-over.

No PINK CAB runtime implementation may begin in `DEADRACE` while `CD-558` remains open.

## 2. Required source evidence before migration

Record before transfer:

- source repository exact default-branch SHA;
- source staging branch exact SHA;
- `scripts/check-authority.sh` result and exit code on the staging SHA;
- list of files copied to target;
- list of legacy files intentionally excluded;
- target repository initial SHA after migration.

## 3. Carry-forward set

The target repository must receive the current PINK CAB authority and development-preparation material required to execute PF-00.

Required carry-forward categories:

- `.github/` studio governance files that remain applicable;
- `.gitattributes` and `.gitignore` after PINK CAB-specific review;
- `CONTRIBUTING.md` after repository-name correction;
- PINK CAB active authority under `docs/`;
- PRE-FGEAR design, implementation plan and execution index;
- `scripts/check-authority.sh` and any directly required authority validation helpers;
- asset/license ledger needed for the zero-paid-assets rule;
- active README content rewritten for dedicated PINK CAB identity.

## 4. Explicitly exclude from active target truth

Do not migrate as active product authority:

- DEADRACE pursuit/combat-era runtime or gameplay source;
- contradictory/superseded historical snapshots unless copied under an explicit legacy archive;
- unrelated DEADCORN/DEADBALL source or content;
- obsolete branches, temporary artifacts, caches or generated build output;
- any secret, credential, local-machine path or user token.

If historical evidence is needed, prefer stable links/SHA references over copying entire obsolete trees.

## 5. Target root contract

The dedicated repository is prepared for this root shape:

```text
.github/
Config/              # created by PF-00 when Unreal bootstrap begins
Content/             # created by PF-00 / later content work
Data/                # product-owned data/config assets when introduced
Source/              # created by PF-00
Scripts/             # build/verification helpers when introduced
docs/
.gitattributes
.gitignore
CONTRIBUTING.md
README.md
PinkCab.uproject      # created by PF-00, not by repository cut-over itself
```

Empty future Unreal directories do not need to be committed during cut-over.

## 6. Target README minimum

The first target README must state:

- product: PINK CAB;
- Jira lane: `CD-519`;
- PRE-FGEAR program: `CD-754`;
- repository authority: this repo is the sole active PINK CAB technical truth;
- FIRST EURO: PC / single-player / full L1+L2;
- zero-paid-assets rule before PF-16;
- FGear and VDS ownership doctrine without claiming either is installed;
- `CANON -> SPECIFIED -> IMPLEMENTED -> VERIFIED` truth rule.

## 7. Mechanical cut-over procedure

1. Create `CheshirskyCat63/PINK-CAB` with a default branch and no generated template content that conflicts with migration.
2. Clone/fetch the new target into a dedicated local directory separate from all other active games.
3. Export/copy only the carry-forward set from source staging SHA.
4. Rewrite repository-name references from `DEADRACE` to `PINK-CAB` only where they refer to active PINK CAB technical authority; preserve historical SHA/evidence references verbatim.
5. Mark source-repository references explicitly as `legacy/migration-source` where retained.
6. Run the authority guard in the target repository and fix only migration-induced path/name mismatches; do not alter owner decisions.
7. Commit the migration baseline with a message equivalent to `chore: establish dedicated PINK CAB repository authority`.
8. Record exact target SHA in `CD-558`, `CD-519`, `CD-754` and Confluence repository authority page `12451841`.
9. Update `docs/PINK_CAB_PRE_FGEAR_EXECUTION_INDEX.md` so the dedicated repository is the execution location and DEADRACE is historical source only.
10. Freeze/close the PINK CAB staging branch in DEADRACE after verifying all required material exists in target.
11. Verify no active PINK CAB implementation branch remains in DEADRACE.
12. Close `CD-558` only with full Closure Log evidence.
13. Only then unblock `CD-755 / PF-00`.

## 8. Verification commands after target exists

Run from the dedicated target checkout:

```bash
git status --short
git rev-parse HEAD
git remote -v
git diff --check
bash scripts/check-authority.sh
```

Required results:

- working tree clean after migration commit;
- `origin` points to `CheshirskyCat63/PINK-CAB`;
- `git diff --check` exit `0`;
- authority guard exit `0`;
- no false current reference that makes DEADRACE the active PINK CAB repository;
- no paid dependency introduced.

## 9. CD-558 closure record

Before DONE record:

- source default SHA;
- source staging SHA;
- target initial/migration SHA;
- exact target repository URL;
- authority-guard command/result;
- diff-check result;
- migrated-file inventory;
- excluded legacy categories;
- confirmation that DEADRACE is legacy/migration-source only;
- confirmation that PF-00 is now permitted in the dedicated repo.

## 10. Closure state

Cut-over completed 2026-09-11.

- target baseline: `a807db88baed881c526f2df63f87d01e913f41ef`;
- source staging migration SHA: `1a4d7e05f1430a6e558b3f697aee3fbf412785bb`;
- source staging freeze marker: `bff83382e3b567e17fa01029f698204c7bce7872`;
- `CD-558`: DONE with Closure Log;
- `DEADRACE`: legacy/migration-source only;
- PF-00 may proceed in `PINK-CAB`.
