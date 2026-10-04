# PINK-CAB Control Plane

Status: **PROTECTED INTEGRATION / P04 CANDIDATE HUMAN ACCEPTED / ADMIN CD-952**

## Current execution checkpoint

Accepted P04 measured gearing runtime: **52239b61bc80e5a63716b9b09b87c520fa09fd05**, delivery **37149462470**, attempt 1; explicitly owner accepted on 2026-10-03. PR #62 integrated it as **4a313d38f0674a3e5048f832a07428defa31ab62**. Read GitHub for later main commits; integration/admin commits never rename accepted executable bytes.

Previous accepted fallbacks: P03/V2 `edf75e1b` / delivery `37117735294`, and P02 `8d68e456` / delivery `36868646970`. Road R1-R5 and no-assist input grammar remain frozen. Sole root desktop game entry: `PINCKCAB`; studio entries are Editor and explicitly named Rollback_V2.

Coordinator `37148042881`, gated delivery `37149462470` and installed audit `37151172013` passed: 79 script tests, complete ControlRuntime, 63 physics tests, 240 D3 cases, five slope repeats and 53 installed payload files. Additive owner decision is `OWNER_ACCEPTANCE.json`, retained by run `37152180059`; original HUMAN_PENDING handoff receipts remain unchanged historical evidence. Administrative closeout: CD-952.

CD-648 remains the single vehicle umbrella. CD-641 owns remaining P04 performance calibration and the non-monotonic intermediate-input observation; P05-P11 and full FIRST EURO remain unfinished. CD-559 retains clean-source/full-project/packaged-input engineering, not an absent P03 acceptance. PR #47/CD-650 remains diagnostic only. CD-855 placeholder polish/replacement is deferred until vehicle calibration and does not block internal physics/admin; public asset rights stay separate.

## Accepted executable and rollback

`C:\Users\CheCat\Desktop\PINCKCAB.lnk` targets the real executable in `E:\CHESHIRE_DIVISION\Builds\PINKCAB\CD869_ENDLESS_52239b61_RUN37149462470_ATTEMPT1`. Accepted source is 52239b61, not whichever branch is checked out. `CHESHIRE_STUDIO` retains PINKCAB_Editor and PINKCAB_Rollback_V2 (edf75e1b / 37117735294). Old Play/Test aliases were preserved outside the desktop. P02 and historical 8168d724 remain retained evidence. Original handoff and additive owner decision retain separate identities.

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

Normal coordinator: `.github/workflows/pinkcab-repository-verification.yml` — complete scope, repository checks, exact-source physics, same-run evidence plus five independent slope repeats, then aggregate technical gate. Main requires both `Repository verification` and `Gameplay acceptance gate`.

Human delivery is explicit: `cd648-p02-phy009.yml` with `deliver_human=true` on the reviewed candidate ref, then its verified `cd869-deliver.yml` call. Record the full SHA/run/attempt. The route checks packaged map/four-wheel/material smoke, installs to a unique SHA/run/attempt directory, compares payload paths/sizes/SHA256, verifies an interactive window and atomically replaces only `PINCKCAB.lnk`, retaining prior link and accepted package. Delivery initially records HUMAN_PENDING; owner acceptance is a subsequent separate record.

Retained G1 fast/human_gate/release_gate and inline definitions are auxiliary/history, not competing routine entry points. Policy and implementation now permit only P02-mediated owner-test delivery: CD-869 is reusable-only (`workflow_call`), has no direct `workflow_dispatch`, and its delivery job requires the verified P02 caller identity plus the same GitHub run ID and attempt. The retired local fast-delivery script refuses mutation. Full clean-source/full-project/packaged-input qualification remains CD-559 engineering. Cached-package source-marker/executable-presence checks are not a complete cache-provenance certificate.

Documentation-only administration does not require replacing the runtime package. PINK CAB and KUKURUZA registrations share one physical host; coordinate heavy Unreal work without stopping another project's processes.

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

Never mark CD-559 Done from script checks alone. Full clean-source/full-product regression and comprehensive packaged input remain open; the current P04 fresh package, immutable install, 53-file manifest, smoke, visible handoff and owner acceptance are already verified sub-scopes.
