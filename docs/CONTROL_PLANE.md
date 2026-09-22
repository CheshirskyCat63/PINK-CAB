# PINK-CAB Control Plane

Status: **ADMIN FREEZE / CLEANUP ACTIVE**  
Date: **2026-09-22**

## One current execution path

- Product root: Jira **CD-519**
- Mechanics / recovery owner: Jira **CD-848**
- Administrative hygiene owner: Jira **CD-868**
- Canonical mechanics authority: Confluence **47 · Control & Vehicle Mechanics Release Contract** (page 16744449)
- Canonical pre-model handoff: Confluence page **15663105** (sole current “46”)
- Git target branch: **main**
- Single active implementation PR: **#7**
- Single active implementation branch: **fix/CD-848-recovery-r1-input-contract**
- Production vehicle technology: **Unreal Engine 5.8.2 native Chaos Vehicles**
- FGear/VDS: **archived research only**

Older PRs #3–#6 are closed historical evidence. Recovery pages 48/48A…48G are archived planning/provenance, not current status.

## Current technical truth

Last runtime-affecting candidate before administrative-only cleanup:

`8bed51e5c47e037eb45a670f9d6c1c2cdd28b712`

GitHub Actions run `35746083559`:

- strict code health: PASS
- PinkCabEditor build: PASS
- focused recovery automation: PASS
- full PinkCab automation: PASS
- Win64 package: PASS
- packaged smoke / critical scan: PASS
- packaged Windows OS-input route: **FAIL**
- failure point: throttle dosing
- immutable HUMAN_PENDING promotion: not executed
- current HUMAN acceptance: **NO**

Administrative commits after that SHA do not claim gameplay improvement.

## Recovery Input Contract R1

- mouse = steering by default
- Space = gaze/look; release returns mouse to steering
- Q = clutch
- W = brake
- E = throttle
- Q/W/E + wheel = analog dosing; overlap priority E → W → Q
- 1/2/3/4 = quick target recall only; recall does not actuate
- RMB = acquire/retain current Gearbox/Handbrake target; RMB alone must not move the lever or steal steering
- RMB+LMB = lever manipulation
- while RMB+LMB is held, mouse XY belongs to the lever and steering holds
- releasing LMB returns XY to steering while RMB may remain held
- releasing RMB ends grip
- focus/menu loss clears transient capture
- no hidden auto-throttle, rev-match, countersteer, yaw rescue, ABS or ESP

## Resource policy

There is one GitHub Actions workflow: `.github/workflows/pinkcab-g1-github-control-plane.yml`.

It is **manual-only**.

Dispatch modes:

### fast — default

Use during normal iteration.

Runs:
- exact-head preflight
- zero-debt code health
- incremental PinkCabEditor build
- five focused recovery suites:
  - `PinkCab.G1.HInp`
  - `PinkCab.Cockpit.Input.Recovery`
  - `PinkCab.Vehicle.ChaosBaseline.PhysicsOnly`
  - `PinkCab.Vehicle.Visual.WheelPresentationChaosSync`
  - `PinkCab.UI.SystemMenu`

Does **not** package or deliver a human build.

### human_gate

Use only when a build is genuinely ready for owner testing.

Adds:
- full recovery matrix
- full `PinkCab` automation regression
- Win64 package
- packaged smoke
- Windows synthetic OS-input route
- immutable delivery
- HUMAN_PENDING shortcut/artifacts

Rule: **do not spend package/runtime-gate resources on documentation/admin-only changes.**

## Jira active surface

During cleanup, the non-backlog PINK-CAB execution surface is exactly:

- CD-519 — product root
- CD-848 — mechanics/recovery
- CD-868 — control-plane cleanup

CD-855 and CD-860 are BACKLOG. Old mechanics cards moved out of UNDER REVIEW remain historical/backlog until reconciled with R1.

## Confluence precedence

1. Product Family / Authority Index — page 6586369
2. Control & Vehicle Mechanics Release Contract — page 16744449
3. Native Chaos production authority — page 13303842
4. Canonical pre-model handoff — page 15663105
5. Detailed current subsystem pages only where they do not conflict with the above

Pages 48/48A…48G are **RECOVERY ARCHIVE**.  
Former duplicate page 15826945 is **ARCHIVE**, not a competing page 46.  
Former 28A FGear-lineage matrix is archived.


## SaaS / cost policy

PINK-CAB is **FREE-FIRST / RESOURCE-CONSTRAINED**.

- Jira target plan: **Free**, provided the site remains within Free eligibility (10 Jira users or fewer and storage within the Free allowance).
- Jira is a production tracker, not the build/test execution engine.
- Do not depend on paid Jira audit logs, Rovo/AI, advanced permission editing or high automation quotas for the game pipeline.
- Existing permission schemes may remain preserved after downgrade but Free must not require routine permission-scheme editing.
- Prefer GitHub + self-hosted Windows UE runner for deterministic technical automation.
- New paid SaaS/plugin/tool dependencies require explicit owner approval and a concrete blocker they solve.
- If a workflow can be implemented reliably with current free/native tools, do not introduce a paid dependency.

## Branch hygiene

Unique material from superseded branches is preserved under `docs/archive/`.

Remote branch cleanup completed on 2026-09-22. Exactly two remote branches remain:
- `main`
- `fix/CD-848-recovery-r1-input-contract`

Unique material from superseded branches remains preserved under `docs/archive/`.

New rule after cleanup:

`main → one task branch → one PR → verify → merge → delete branch`

No parallel ownerfix/final/recovery/integration branch chains.

## Resume gate

Gameplay work resumes only when:

1. The remaining Jira billing human gate (Standard → Free) is acknowledged/completed.
2. PR #7 remains the sole active implementation PR.
3. No current Confluence page contradicts Recovery R1.
4. Exact runtime changes use the **fast** lane first.
5. A candidate intended for owner testing passes **human_gate** lane.
6. Owner performs the human gate and explicitly accepts/rejects it.

Technical PASS never equals Human Acceptance.
