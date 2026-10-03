# PINK CAB · Project Setup

**Status:** CURRENT DEVELOPMENT SETUP / ACCEPTED P04 CANDIDATE
**Product:** `CD-519`
**BASE-100:** `CD-746..753`; Confluence `11239425`
**Readiness:** `CD-660/CD-661`
**Repository:** `CheshirskyCat63/PINK-CAB` — sole active PINK CAB technical truth.

## Development entry · 2026-10-03

Canonical working copy: `E:\CHESHIRE_DIVISION\Games\PINK-CAB`. Integration target: protected `main`. Other restored copies and historical worktrees are not current build evidence. Preserve unmerged work before changing branches.

Run `python scripts/ci/prepare_metaroad.py` before building. The repository pins MetaRoad 3.2.0 to 533 authored-file SHA256 values in `scripts/ci/metaroad.lock.json`. Set `PINKCAB_METAROAD_PACKAGE` to an authorized local package root when using another workstation. The payload is installed in ignored `Plugins/MetaRoad`; it is not published to Git and the shared engine installation is not modified. Missing, modified or mixed source files fail verification.

Build or generate the IDE project with `.\scripts\build.ps1 -GenerateProjectFiles`; package with `.\scripts\build.ps1 -Package`. Launcher and source-engine project generation are supported. Engine family is pinned to UE 5.8; the prepared workstation reports 5.8.3 / CL 58210709. Override the engine path with `-EngineRoot` or `PINKCAB_UE_ROOT`.

CI runner: `DESKTOP-C7VAU4V-PINKCAB`, scoped to this repository. `scripts/ci/start-pinkcab-runner.ps1` checks registration identity and avoids a second listener. Required integration checks: `Repository verification` and `Gameplay acceptance gate`, including administrators; merge requires an up-to-date PR and resolved conversations. Ordinary verification cannot deliver or launch a replacement HUMAN build.

Accepted P04 measured gearing runtime: **52239b61bc80e5a63716b9b09b87c520fa09fd05**, delivery **37149462470**, attempt 1; explicitly owner accepted on 2026-10-03. PR #62 integrated it as **4a313d38f0674a3e5048f832a07428defa31ab62**. Read GitHub for later main commits; integration/admin commits never rename accepted executable bytes.

Previous accepted fallbacks: P03/V2 `edf75e1b` / delivery `37117735294`, and P02 `8d68e456` / delivery `36868646970`. Road R1-R5 and no-assist input grammar remain frozen. Sole root desktop game entry: `PINCKCAB`; studio entries are Editor and explicitly named Rollback_V2.

Coordinator `37148042881`, gated delivery `37149462470` and installed audit `37151172013` passed: 79 script tests, complete ControlRuntime, 63 physics tests, 240 D3 cases, five slope repeats and 53 installed payload files. Additive owner decision is `OWNER_ACCEPTANCE.json`, retained by run `37152180059`; original HUMAN_PENDING handoff receipts remain unchanged historical evidence. Administrative closeout: CD-952.

CD-648 remains the single vehicle umbrella. CD-641 owns remaining P04 performance calibration and the non-monotonic intermediate-input observation; P05-P11 and full FIRST EURO remain unfinished. CD-559 retains clean-source/full-project/packaged-input engineering, not an absent P03 acceptance. PR #47/CD-650 remains diagnostic only. CD-855 placeholder polish/replacement is deferred until vehicle calibration and does not block internal physics/admin; public asset rights stay separate.

Authoring tests that save tracked assets must run in an isolated checkout. Never combine different source trees as one full-suite proof.

## Read first

- `docs/AUTHORITY.yaml`;
- `docs/FIRST_EURO_SCOPE.md`;
- `docs/PINK_CAB_BASE100_CODE_ARCHITECTURE.md`;
- `docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md`;
- `docs/OPEN_DECISIONS.md`;
- `docs/VERIFICATION_MATRIX.md`;
- Confluence `6586369`, `11239425`, `5832744`;
- Jira `CD-519`, `CD-588`, `CD-673`, `CD-660/CD-661`.

## Readiness

The old <=20% / ~59% estimates and previous 240-question/20-question-pack methodology are SUPERSEDED.

BASE-100 is code/logic/technical specification readiness for the complete 12-month FIRST EURO single-player product. The earlier 66.6% census is historical, not live administrative or runtime readiness. Current execution status comes from Jira CD-519/CD-648/CD-559 and `docs/AUTHORITY.yaml`; do not use a historical percentage to accept a build.

Administrative/specification work is allowed before START-90. Broad production should not rely on unresolved owner decisions.

## Runtime status

PF-00 bootstrap is VERIFIED on the canonical UE 5.8.2 build/package path and PF-01 Core Contracts is VERIFIED/merged. Subsequent runtime work must continue to attach exact commit/build/automation evidence and may not silently decide open gameplay rules.

## Engine / plugin direction

- engine family: Unreal Engine 5;
- `A01 LOCKED`: production engine line is Unreal Engine 5.8; the historical bootstrap used 5.8.2 / CL 56702186, while the current prepared workstation has 5.8.3 / CL 58210709; changing the production engine line requires an explicit migration/compatibility decision;
- **Chaos Vehicles / native Unreal physics** = sole production hero-Tatra road-dynamics solver;
- **Native bounded damage/destruction** = authored damage-state swaps, detachable parts, pooled debris and selective Chaos events;
- FGear/VDS remain archived research only; any future vendor system must remain behind a PINK CAB adapter and requires a new explicit migration decision;
- native Unreal or mature ready-made solutions are preferred before custom framework development;
- FIRST EURO platform = PC;
- FIRST EURO input baseline = keyboard + mouse.

Do not treat an old UE candidate or long-range mobile/console wish list as a current bootstrap dependency.

## Repository / branch model

Current repository is `CheshirskyCat63/PINK-CAB`, the sole active PINK CAB technical source. `CheshirskyCat63/DEADRACE` is frozen legacy/migration-source history only.

- `main` = integration target;
- implementation uses short-lived Jira-keyed branches;
- authority/spec work may use dedicated docs/spec branches;
- product-intent changes must reconcile Jira + Confluence + Git before integration;
- exact build/runtime evidence references the commit actually tested.

## Module/data doctrine

Exact module count/names remain Pack A03. General lock:

- C++ owns public interfaces, state ownership, persistence, schema/versioning, invariants, critical orchestration and vendor adapters;
- Blueprint remains thin composition/orchestration;
- DataAssets/DataTables/config own tuning/balance/profile values;
- immutable config is separate from runtime state;
- logical identities can exist without materialized Actors;
- UE Subsystems preferred over ad-hoc global managers.

## Current input contract

- mouse = steering by default;
- hold Space = gaze/free-look and bounded target search;
- `1–4` = quick recall of saved physical targets/hand poses for `1 signals / 2 horn / 3 gearbox / 4 handbrake`; recall alone never actuates;
- RMB = optional acquire/retain on an authored target; direct LMB/wheel actions do not require a universal RMB-first gesture;
- LMB = press/hold momentary controls; e.g. tap horn for a short signal or hold for a long signal;
- mouse wheel = contextual detent/rotary/incremental adjustment where the current control supports it;
- Q clutch, W brake, E throttle;
- clutch wheel semantics adjust release speed, not instantaneous clutch pressure;
- one bounded current interaction target; no world scan; hidden steering continuity cannot choose route/lane/gap, brake, throttle, overtake or avoid traffic;
- target feedback is textless: translucent reticle by default, brighter white-matte/less-transparent on a valid control;
- left/right hand is selected automatically by presentation context; hand animation visualizes the interaction but never owns gameplay truth.

The universal `LMB ATTENTION / RMB GO` wording is SUPERSEDED. Interaction is control-type-driven: target/quick-recall, optional grip, then the gesture declared by that physical control.

## FIRST EURO session direction

- direct start into playable Tatra/world;
- one personal character and deterministic personal CityCode;
- permanent night/night-shift visual baseline with changing weather;
- Workday is LOCKED: 12 in-game hours over 120 real minutes (x6), with eligible early sleep/end-day;
- sleep/end-day occurs in the car at full stop with no active FareSession; parking legality/signage and fines still apply;
- manual save+exit is prohibited while a FareSession/passenger is active and otherwise requires a full stop; `I13` still owns force-quit/crash anti-exploit reconstruction;
- ordinary persistent state must survive through explicit schemas/transactions, not transient Actor snapshots.

## FIRST EURO world scope

Level1 + Level2 are fully playable in year one. Level3 gameplay is POST-FIRST-EURO and only a compatible schema/route extension boundary is required now.

World identity is `CityCode + GeneratorVersion + ContentSetVersion + persistent deltas`. Use Unreal World Partition/PCG/engine facilities where appropriate with a thin deterministic recipe layer; do not create a bespoke world streaming engine unless a proven gap requires it.

## Vehicle baseline

Hero Tatra: bespoke early/Gen-1 603-family; rear-mounted air-cooled V8; RWD; no ABS/ESP. The executable profile and current vehicle release contract own mass/power/ratio calibration. Earlier 180 hp / 240 Nm values are historical, not current locks. Accepted P02 uses approximately 250 hp / 260 Nm / 8500 RPM with healthy warm idle centered at 925 RPM; P04 first/reverse4.0 and profile-derived RPM/speed defaults are owner accepted; remaining torque/rev-inertia/road-performance calibration stays in CD-641.

Level1 residual magnetism is linear by authoritative mass: 5.0 s @1657 kg → 4.0 s @2107 kg; lighter legal states cap at 5.0 s.

## Taxi / Neural direction

FIRST EURO contains complete FareSession/taxi work plus persistent repeat clients/basic Neural. PassengerIdentity persists independently from a materialized passenger Actor. Repeat orders reuse the normal order/fare pipeline.

## Services

FIRST EURO ServiceNodes are automotive only: Parking, Garage/Tuning, Parts, Repair/Service, plus Practice Hangar where required. Moving refueling is a separate FIRST EURO live-road session over shared FuelTank/Economy/VehicleTelemetry/RoadGraph.

Mall/food/bar/club/social nodes are POST-FIRST-EURO.

## Enforcement / recovery extensions

FIRST EURO implements stable `EnforcementEvent` facts and ordinary rules/fines. Full Taxi Regulator is POST-FIRST-EURO.

Daily insurance is POST-FIRST-EURO. FIRST EURO preserves generic `RecoveryPolicy/RecoveryHook`; Pack I06 / `CD-750` locks terminal-crash recovery to Workday end + atomic save + still-damaged Repair recovery next Workday, with no free reset.

## Hot-path setup rules

No unbounded Tick, global Actor scans, sync loads or repeated spawn/allocation storms in normal gameplay hot paths. Bound populations/queues/caches; pool/materialize only what runtime needs.

## Verification rule

Every executable result records exact commit/build, UE/plugin/config/schema/generator/profile versions, deterministic IDs/seeds, expected/observed behavior and artifacts. Build success alone does not establish gameplay verification.

Apply IMPLEMENTED/VERIFIED per scope: the recorded P04 candidate is owner accepted, while unproved full-product/release requirements remain open. Do not label the entire project unimplemented or infer release readiness from one accepted candidate.
