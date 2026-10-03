# PINK-CAB Control Plane

Status: **PROTECTED INTEGRATION / P03 CORRECTION OPEN**

## Current execution checkpoint

- Integration: protected `main`; exact current SHA is read from GitHub, not inferred from an old document.
- Accepted runtime: P02 `8d68e456d1944be295281535cf9fd103ecf05d52`, run `36868646970`, accepted 2026-10-01 and integrated by PR #49. P00-P02 and road R1-R5 remain frozen.
- Active gameplay correction: PR #52 / CD-649 + CD-659. P03 is HUMAN REJECTED; P04 is BLOCKED until corrective automation, packaged delivery and renewed owner acceptance.
- PR #47 / CD-650 is a separate draft tire diagnostic, not an accepted calibration or next road stage.
- Infrastructure: CD-559 remains IN PROGRESS. PR #53 integrated CI trust/scope repairs and explicit delivery control; full regression and release reproducibility are not thereby certified.
- Preserved preparation branches: `fix/CD-559-development-bootstrap-20261002` at `4554285` and `fix/CD-659-p03-readiness-20261002` at `95dafd4`. They are unmerged evidence/candidates, not competing integration branches. Reconcile them into PR #52 before a new gameplay acceptance.
- Preparation evidence: 436/446 latest selected test outcomes passed on the bootstrap branch; 10 failed. Separate P03 correction: 47/47 physics and 30/30 control tests passed. These are different source trees and must not be added together as full-suite proof.

## Accepted executable and rollback

`PINKCAB Latest.lnk` was verified on 2026-10-02 to point to `E:\CHESHIRE_DIVISION\Builds\PINKCAB\CD869_ENDLESS_8d68e456_RUN36868646970`. The accepted executable is `8d68e456d1944be295281535cf9fd103ecf05d52`, not whichever branch is currently checked out. Keep the accepted build available when explicitly delivering an unaccepted candidate.

Historical 2026-09-23 baseline `8168d724` / run `35809749568` remains retained evidence. It was superseded by later accepted road/vehicle integrations; it is not the latest accepted P02 binary.

## Current input / cockpit contract

- mouse = steering by default;
- Space = gaze/look; release returns mouse to steering;
- Q = clutch;
- W = brake;
- E = throttle;
- Q/W/E + wheel = analog dosing; overlap priority **E → W → Q**;
- 1/2/3/4 = ephemeral quick access for turn signals / horn / gearbox / handbrake;
- quick-access prompt disappears when the number key is released;
- RMB = optional contextual capture/retain of a valid control; RMB alone never actuates it;
- LMB and mouse wheel may perform the authored contextual action directly without an RMB prerequisite;
- Gearbox/Handbrake may be directly manipulated by LMB on the active/contextual target; RMB remains useful when the driver wants the target retained;
- during lever manipulation mouse XY belongs only to the lever and the current steering command is held;
- after manipulation ends, mouse XY returns to steering;
- sustained same-direction wheel input progressively accelerates; pause or direction reversal resets the burst;
- steering is manual/no-assist: heavy at standstill, lighter once rolling, calmer rather than sharper at high speed;
- focus/menu loss clears transient interaction ownership;
- no hidden auto-throttle, rev-match, countersteer, yaw rescue, ABS or ESP.

Detailed authority: `docs/recovery/RECOVERY_INPUT_CONTRACT_R1.md`.

## GitHub execution policy

Canonical workflow:

`.github/workflows/pinkcab-g1-github-control-plane.yml`

Normal development sequence:

1. exact-head preflight;
2. zero-debt code health;
3. `fast` for ordinary implementation iteration;
4. `human_gate` for a lightweight exact-head code-only owner-test build delivered through `PINKCAB Latest.lnk`; cook-sensitive Content/Config/Plugin/project changes fail closed and require `release_gate`;
5. `release_gate` only when a fresh full automation/cook/package evidence bundle is required;
6. human acceptance/rejection remains separate from technical PASS.

The code-only standalone lane that produced the accepted `8168d724...` build is immutable historical evidence. The current canonical workflow is manual-only with `fast`, `human_gate`, and `release_gate`, all targeting `main`.

Windows SAC/UMCI machine-level trust belongs only to `release_gate`. It must never block routine `fast` or `human_gate` development delivery.

Do not spend full package resources on documentation-only administration or ordinary handling iteration.

## Jira active surface

- **CD-519** — active product root, remains IN PROGRESS;
- **CD-848** — current broader mechanics/FIRST EURO owner, remains IN PROGRESS;
- **CD-868** — control-plane cleanup, DONE.

Backlog/history is not an active execution lane merely because it is not Done.

## Confluence precedence

1. page **6586369** — Product Family / Authority Index
2. page **16744449** — Control & Vehicle Mechanics Release Contract
3. page **13303842** — Native Chaos production authority
4. page **15663105** — canonical pre-model handoff

Recovery 48-series, former duplicate 46 and pre-Chaos FGear lineage are archive/history, not current status.

## Resource / cost policy

PINK-CAB remains **FREE-FIRST / RESOURCE-CONSTRAINED**.

- GitHub + self-hosted Windows UE runner is the technical execution engine.
- Jira is production tracking truth, not build infrastructure.
- Jira Free is the preferred account plan when eligible.
- Billing-plan state is an account setting and is **not** a blocker for the accepted runtime or Git authority.
- Production must not depend on paid Jira audit logs, Rovo/AI, advanced permission editing or high automation quotas.
- Any new paid SaaS/plugin/tool requires explicit owner approval and a concrete blocker.

## Integration and delivery controls

Every main change requires a PR, an up-to-date branch, both GitHub Actions checks `Repository verification` and `Gameplay acceptance gate`, and resolved conversations. Both required checks are pinned to the GitHub Actions app; strict up-to-date enforcement is enabled. Rules include administrators; force-push and deletion are prohibited. Required reviewer count is zero for the current single-owner team; this does not claim independent review.

The automatic coordinator is `pinkcab-repository-verification.yml`. It classifies the complete PR merge-base range (or the complete push before/after range). Only known documentation/administrative paths may skip Unreal; code, content, configuration, CI, the executable writer inventory and unknown paths require runtime checks. Missing scope/history fails closed.

For runtime changes, the existing TDD workflow runs the full physics suite once, including all 240 D3 samples. It publishes a receipt bound to the candidate SHA, Actions run, run attempt, complete registered test-name set and log hash. The existing P02 workflow validates that receipt and independently repeats the slope fixture five times. Manual P02 dispatch still runs the full physics suite before its five repeats. No test thresholds or gameplay acceptance rules change.

Use **Re-run all jobs** for a failed coordinated run. Re-running only a downstream failed job creates a new run attempt without a matching TDD receipt and intentionally fails closed; previous-attempt evidence is not silently reused.

`Gameplay acceptance gate` always evaluates the static, TDD and P02 results. Runtime changes require successful results from every needed job; failed, cancelled, skipped or missing runtime evidence cannot pass. Administrative-only changes explicitly report N/A for gameplay. Both required contexts were activated and read back from GitHub on 2026-10-03 after PR #57 candidate `4f25b8e1ecbdd98808787035df07b8fc566a90df` passed run `37094358590`: 71 script tests, 47 physics tests, the exact 240-case D3 grid and five independent slope repeats. That coordinated run took 12 minutes 50 seconds from its first job start through the aggregate gate. It proves the automated candidate checks; it does not certify a new packaged delivery or P03 human acceptance.

Self-hosted checkouts retain Unreal caches. Before building, exact SHA, tracked/untracked cleanliness, ignored authored input roots and the pinned MetaRoad package are checked. The normal UBT build still runs; cached output is never itself accepted as evidence. Unexpected authored leftovers fail and must be investigated instead of silently removed. Clean-checkout reproducibility remains a separate CD-559 obligation.

Self-hosted PR jobs accept only same-repository OWNER/MEMBER/COLLABORATOR branches. Fork checks run on GitHub-hosted machines. The dedicated runner is `DESKTOP-C7VAU4V-PINKCAB`; KUKURUZA has a separate registration.

Ordinary PR verification does not deliver or launch a human build. P02 delivery requires an explicit workflow dispatch with `deliver_human=true` after verification succeeds. Delivery is HUMAN_PENDING until the owner accepts that exact candidate.

Never mark CD-559 Done from script checks alone: clean-checkout build/assets, full exact-source regression, fresh package/runtime smoke and retained provenance remain its acceptance requirements. Known gameplay regressions and the P03 human gate stay visible under their existing owners.
