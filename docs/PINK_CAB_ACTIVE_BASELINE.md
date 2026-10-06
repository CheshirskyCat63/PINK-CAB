# PINK CAB · Active Product Baseline

**Status:** CURRENT ACTIVE PRODUCT / DEVELOPMENT-READY  
**Product root:** `CD-519`  
**Current broader mechanics owner:** `CD-848`  
**Canonical Git:** `CheshirskyCat63/PINK-CAB` → `main`

## Current execution checkpoint

Accepted P04 measured gearing runtime: **52239b61bc80e5a63716b9b09b87c520fa09fd05**, delivery **37149462470**, attempt 1; explicitly owner accepted on 2026-10-03. PR #62 integrated it as **4a313d38f0674a3e5048f832a07428defa31ab62**. Read GitHub for later main commits; integration/admin commits never rename accepted executable bytes.

Previous accepted fallbacks: P03/V2 `edf75e1b` / delivery `37117735294`, and P02 `8d68e456` / delivery `36868646970`. Road R1-R5 and no-assist input grammar remain frozen. Sole root desktop game entry: `PINCKCAB`; studio entries are Editor and explicitly named Rollback_V2.

Coordinator `37148042881`, gated delivery `37149462470` and installed audit `37151172013` passed: 79 script tests, complete ControlRuntime, 63 physics tests, 240 D3 cases, five slope repeats and 53 installed payload files. Additive owner decision is `OWNER_ACCEPTANCE.json`, retained by run `37152180059`; original HUMAN_PENDING handoff receipts remain unchanged historical evidence. Administrative closeout: CD-952.

CD-648 remains the single vehicle umbrella. CD-641 owns remaining P04 performance calibration and the non-monotonic intermediate-input observation; P05-P11 and full FIRST EURO remain unfinished. CD-559 retains clean-source/full-project/packaged-input engineering, not an absent P03 acceptance. PR #47/CD-650 remains diagnostic only. CD-855 placeholder polish/replacement is deferred until vehicle calibration and does not block internal physics/admin; public asset rights stay separate.

## Owner-accepted working runtime

Accepted P04 measured gearing runtime: **52239b61bc80e5a63716b9b09b87c520fa09fd05**, delivery **37149462470**, attempt 1; explicitly owner accepted on 2026-10-03. PR #62 integrated it as **4a313d38f0674a3e5048f832a07428defa31ab62**. Read GitHub for later main commits; integration/admin commits never rename accepted executable bytes.

Previous accepted fallbacks: P03/V2 `edf75e1b` / delivery `37117735294`, and P02 `8d68e456` / delivery `36868646970`. Road R1-R5 and no-assist input grammar remain frozen. Sole root desktop game entry: `PINCKCAB`; studio entries are Editor and explicitly named Rollback_V2.

Coordinator `37148042881`, gated delivery `37149462470` and installed audit `37151172013` passed: 79 script tests, complete ControlRuntime, 63 physics tests, 240 D3 cases, five slope repeats and 53 installed payload files. Additive owner decision is `OWNER_ACCEPTANCE.json`, retained by run `37152180059`; original HUMAN_PENDING handoff receipts remain unchanged historical evidence. Administrative closeout: CD-952.

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

Vehicle priorities remain CD-648: remaining P04 calibration, then P05-P11. The broader FIRST EURO queue below follows those priorities; no new migration is required:

`CD-869 → CD-870 → CD-871 → CD-872 → CD-873 → CD-874 → CD-875 → CD-876 → CD-877 → CD-878 → CD-879`

This closes representative L1↔L2 streaming, pickup/dropoff, automotive services, moving fuel, wet driving, traffic/incidents, repeat clients, daughter role, minimal Neural, payment/fines and finally the terminal Mechanics Freeze evidence audit.

## Delivery policy

Normal verification is `.github/workflows/verify.yml`: hosted static contracts first, then an exact-head `PinkCabEditor` build and the focused playable runtime suite on the PINKCAB self-hosted runner.

Human delivery is `.github/workflows/deliver.yml`. Its single entry point is `scripts/deliver.ps1`, which requires a clean exact HEAD, builds the editor, runs the focused runtime suite, packages with BuildCookRun, smoke-tests the package, and only then atomically publishes `PINCKCAB_BUILD` plus the `PINCKCAB.lnk` shortcut.

The old CD-648/CD-869/P00–P04 workflow graph, causality evidence programs, calibration fixtures and road-authoring gates are historical evidence only. They are not active build, verification or delivery authority.

Documentation-only administration does not require replacing the runtime package. PINK CAB and KUKURUZA registrations share one physical host; coordinate heavy Unreal work without stopping another project's processes.

## Product / world boundary

PINK CAB is a first-person taxi-work / arcade-sim / vehicle-parkour game in an effectively endless retrofuturist longitudinal city. FIRST EURO is PC single-player with Level 1 + approved Level 2 gameplay, hero Tatra, taxi/fare/passenger/payment loop, repeat clients/basic Neural, CityCode/persistence/streaming/road graph/traffic/rules/fines, automotive ServiceNodes, moving refuel and build/save/QA foundations.

Multiplayer, Level 3 gameplay, lifestyle/social ServiceNodes, full Taxi Regulator and daily insurance implementation are post-FIRST-EURO.

## Asset provenance

The current donor Tatra is allowed only as internal development/working-baseline material. Commercial modification/redistribution permission is not proven. `docs/ASSET_LICENSE_LEDGER.csv` records the donor as `BLOCKED_NO_COMMERCIAL_PERMISSION`.

`CD-855` owns permission evidence or legal replacement before public/commercial release. This does **not** block internal gameplay development.

## Truth rule

`CANON → SPECIFIED → IMPLEMENTED → VERIFIED`

Confluence establishes durable design authority. Jira owns live work/dependencies/evidence. Git owns code/tests/build history. Runtime evidence decides implemented/verified state.
