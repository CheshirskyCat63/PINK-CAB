# PINK CAB · Active Product Baseline

**Status:** CURRENT ACTIVE PRODUCT / DEVELOPMENT-READY  
**Product root:** `CD-519`  
**Current broader mechanics owner:** `CD-848`  
**Canonical Git:** `CheshirskyCat63/PINK-CAB` → `main`

## Owner-accepted working runtime

- source SHA: `8168d72406af6934ab20eace583c2b895f0620b7`
- GitHub Actions run: `35809749568`
- successful job: `PINK-CAB code-only standalone human build`
- integration merge: `865e8f77dde4af1f5c5bee8d49754628b6494db4`
- CD-868 control-plane cleanup: DONE
- current development branch: `main`

This is the immutable owner-accepted working executable baseline. Later sanitation/documentation/CI commits may advance `main`, but they do not retroactively rename that accepted binary. A future gameplay baseline supersedes it only after new exact-candidate evidence and owner acceptance.

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
