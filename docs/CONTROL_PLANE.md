# PINK-CAB Control Plane

Status: **WORKING BASELINE FROZEN / ADMIN CLEAN**  
Date: **2026-09-23**

## Canonical production truth

- Product root: Jira **CD-519**
- Mechanics / continuing FIRST EURO owner: Jira **CD-848**
- Administrative cleanup gate: Jira **CD-868 — DONE**
- Canonical Git branch: **main**
- Active implementation PRs: **none**
- Accepted working runtime SHA: `8168d72406af6934ab20eace583c2b895f0620b7`
- Accepted GitHub Actions run: **35809749568**
- Accepted job: **PINK-CAB code-only standalone human build — SUCCESS**
- Accepted local delivery: `E:\CHESHIRE_DIVISION\Builds\PINKCAB\CODEONLY_8168d72_RUN35809749568`
- Desktop handoff: `PINKCAB Latest.lnk`
- Integration merge commit: `865e8f77dde4af1f5c5bee8d49754628b6494db4`
- Production vehicle technology: **Unreal Engine 5.8.2 native Chaos Vehicles**
- FGear / Vehicle Damage System: **archived research only**

The accepted SHA remains immutable executable evidence. Administrative descendants on `main` do not retroactively change that binary.

PR #7 is merged/closed. PRs #3–#6 are historical/closed.

The legacy recovery ref `fix/CD-848-recovery-r1-input-contract` is not an execution lane. It was fast-forwarded to the merged history after PR #7. New work follows:

`main → one Jira-keyed task branch → one PR → verify → merge`

## Accepted runtime evidence

Owner acceptance freezes `8168d724...` as the current **working baseline**, not as a claim that all FIRST EURO scope is finished.

Observed runtime evidence from the accepted build:

- PinkCab monolithic Win64 Development target built successfully;
- code-health zero-debt preflight passed in the standalone lane;
- game process remained running after smoke;
- UE 5.8.2 initialized on Windows;
- D3D12 selected the NVIDIA GeForce RTX 4090 Laptop GPU;
- Chaos physics initialized;
- `/Game/Dev/Maps/L_PinkCab_ChaosWeave` loaded and entered play;
- the owner-supplied runtime log fragment contains no fatal/assert/ensure/unhandled-exception evidence.

Non-blocking log noise for this baseline:

- PIX capture DLL absent when not launched through PIX;
- RenderDoc not attached;
- stable PSO cache absent;
- AudioCapture implementation absent;
- ru-RU localization falls back to English;
- PlayerStart fallback warning;
- render-thread safety warning for `r.MotionVectorSimulation`.

These items are not promoted to gameplay blockers by this baseline acceptance. Performance/content work may address them later where a specific gate requires it.

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
3. fast focused verification;
4. only for genuine human candidates: full regression/package/runtime delivery;
5. human acceptance/rejection.

The standalone code-only lane exists for the current Windows Code Integrity constraint and must preserve its eligibility guard: no cooked Content/Config overlay drift relative to its declared package base.

Do not spend full package resources on documentation-only administration.

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

## Freeze rule

The accepted working build remains:

`8168d72406af6934ab20eace583c2b895f0620b7 / run 35809749568`

Do not rewrite its history or relabel a later administrative commit as the accepted executable. Future gameplay changes start from canonical `main`, receive a new exact SHA and new evidence, and supersede the working baseline only after a new human acceptance.
