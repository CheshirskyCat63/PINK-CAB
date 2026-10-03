# PINK CAB · Active Product Baseline

**Status:** CURRENT ACTIVE PRODUCT / DEVELOPMENT-READY  
**Product root:** `CD-519`  
**Current broader mechanics owner:** `CD-848`  
**Canonical Git:** `CheshirskyCat63/PINK-CAB` → `main`

## Current execution checkpoint

- Integration: protected `main`; exact current SHA is read from GitHub, not inferred from an old document.
- Accepted runtime: P02 `8d68e456d1944be295281535cf9fd103ecf05d52`, run `36868646970`, accepted 2026-10-01 and integrated by PR #49. P00-P02 and road R1-R5 remain frozen.
- Active gameplay correction: PR #52 / CD-649 + CD-659. P03 is HUMAN REJECTED; P04 is BLOCKED until corrective automation, packaged delivery and renewed owner acceptance.
- PR #47 / CD-650 is a separate draft tire diagnostic, not an accepted calibration or next road stage.
- Infrastructure: CD-559 remains IN PROGRESS. PR #53 integrated CI trust/scope repairs and explicit delivery control; full regression and release reproducibility are not thereby certified.
- Preserved preparation branches: `fix/CD-559-development-bootstrap-20261002` at `4554285` and `fix/CD-659-p03-readiness-20261002` at `95dafd4`. They are unmerged evidence/candidates, not competing integration branches. Reconcile them into PR #52 before a new gameplay acceptance.
- Preparation evidence: 436/446 latest selected test outcomes passed on the bootstrap branch; 10 failed. Separate P03 correction: 47/47 physics and 30/30 control tests passed. These are different source trees and must not be added together as full-suite proof.

## Owner-accepted working runtime

- source SHA: `8d68e456d1944be295281535cf9fd103ecf05d52`
- GitHub Actions run: `36868646970`
- runtime integration: PR #49, `104295ab6329e85b5998e8df298770255ad2dd05`
- accepted stage: P02, 2026-10-01
- local delivery: `E:\CHESHIRE_DIVISION\Builds\PINKCAB\CD869_ENDLESS_8d68e456_RUN36868646970`
- later local or remote candidates are not accepted replacements until explicitly approved

Historical baseline `8168d724` / run `35809749568` remains immutable prior evidence.

## Current vehicle/control authority

Production vehicle physics is **Unreal Engine 5.8 Native Chaos Vehicles** behind `IPinkCabVehicleDynamicsProvider`. FGear/VDS are archived research only.

Current cockpit/input behavior:

- mouse steers by default;
- Space owns gaze/free-look while held;
- `1–4` are ephemeral quick access for turn signals / horn / gearbox / handbrake;
- RMB is optional contextual capture/retain and never actuates by itself;
- LMB and mouse wheel may execute authored contextual actions without RMB first;
- Gearbox/Handbrake may be manipulated directly through the active contextual target;
- Q/W/E = clutch/brake/throttle with one wheel recipient priority E → W → Q;
- sustained same-direction wheel bursts accelerate progressively; pause/reversal resets the burst;
- steering is manual/no-assist: heavy at standstill, lighter rolling, calmer at speed;
- H-pattern = 1/3/5 top, 2/4/R bottom, neutral cross-gate;
- requested vs engaged gear remain separate;
- no auto-throttle, auto-rev-match, auto-countersteer, yaw rescue, ABS or ESP.

Authority: `docs/PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`, Recovery Input Contract R1, Confluence 16744449, Jira CD-848.

## Code / architecture state

- primitive FIRST EURO gameplay foundation: code-complete under `CD-793`;
- playable cockpit/build gate: `CD-823` DONE;
- canonical input compliance: `CD-825` DONE;
- vehicle technology reconciliation: `CD-843` DONE;
- pre-model handoff: `CD-856` DONE;
- strict code-health debt baseline: **0 items**;
- one production solver per responsibility;
- runtime domains remain separated across Core / Vehicle / Taxi / World / Traffic / Economy / Persistence / Interaction.

## Current FIRST EURO execution

The project is no longer blocked by recovery or administrative freeze. Current finite closure queue:

`CD-869 → CD-870 → CD-871 → CD-872 → CD-873 → CD-874 → CD-875 → CD-876 → CD-877 → CD-878 → CD-879`

This closes representative L1↔L2 streaming, pickup/dropoff, automotive services, moving fuel, wet driving, traffic/incidents, repeat clients, daughter role, minimal Neural, payment/fines and finally the terminal Mechanics Freeze evidence audit.

## Delivery policy

The canonical GitHub workflow has three distinct purposes:

- `fast` — ordinary engineering verification;
- `human_gate` — lightweight exact-head dev/test delivery to `PINKCAB Latest.lnk`, no full recook;
- `release_gate` — heavy full regression + fresh cook/package/runtime evidence.

Windows Smart App Control / UMCI signing requirements for Editor/full cook are a **release-host infrastructure boundary**, not an everyday gameplay-development blocker.

## Product / world boundary

PINK CAB is a first-person taxi-work / arcade-sim / vehicle-parkour game in an effectively endless retrofuturist longitudinal city. FIRST EURO is PC single-player with Level 1 + approved Level 2 gameplay, hero Tatra, taxi/fare/passenger/payment loop, repeat clients/basic Neural, CityCode/persistence/streaming/road graph/traffic/rules/fines, automotive ServiceNodes, moving refuel and build/save/QA foundations.

Multiplayer, Level 3 gameplay, lifestyle/social ServiceNodes, full Taxi Regulator and daily insurance implementation are post-FIRST-EURO.

## Asset provenance

The current donor Tatra is allowed only as internal development/working-baseline material. Commercial modification/redistribution permission is not proven. `docs/ASSET_LICENSE_LEDGER.csv` records the donor as `BLOCKED_NO_COMMERCIAL_PERMISSION`.

`CD-855` owns permission evidence or legal replacement before public/commercial release. This does **not** block internal gameplay development.

## Truth rule

`CANON → SPECIFIED → IMPLEMENTED → VERIFIED`

Confluence establishes durable design authority. Jira owns live work/dependencies/evidence. Git owns code/tests/build history. Runtime evidence decides implemented/verified state.
