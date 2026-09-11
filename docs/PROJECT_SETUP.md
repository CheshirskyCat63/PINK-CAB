# PINK CAB · Project Setup

**Status:** CURRENT PREPRODUCTION / BOOTSTRAP CONTRACT
**Product:** `CD-519`
**BASE-100:** `CD-746..753`; Confluence `11239425`
**Readiness:** `CD-660/CD-661`
**Repository:** `CheshirskyCat63/PINK-CAB` ? sole active PINK CAB technical truth.

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

Current BASE-100 is code/logic/technical readiness for the complete 12-month FIRST EURO single-player product. `START-90` is not currently scored; fresh scoring waits for Technical Owner Pack 01 answers + contradiction reconciliation.

Administrative/specification work is allowed before START-90. Broad production should not rely on unresolved owner decisions.

## Runtime status

There is no proven canonical PINK CAB executable yet. `CD-596`/current foundation owners must bootstrap only after exact runtime/plugin compatibility is sufficiently locked. Runtime creation may not silently decide open gameplay rules.

## Engine / plugin direction

- engine family: Unreal Engine 5;
- exact UE version remains an owner/toolchain lock until Pack A01 / `CD-557/CD-596` settles it;
- **FGear Vehicle Physics** = sole hero-Tatra road-dynamics solver;
- **Debugging Delight Vehicle Damage System** = sole permanent body-deformation owner;
- purchased/vendor systems remain behind PINK CAB adapters;
- native Unreal or mature ready-made solutions are preferred before custom framework development;
- FIRST EURO platform = PC;
- FIRST EURO input baseline = keyboard + mouse.

Do not treat an old UE candidate or long-range mobile/console wish list as a current bootstrap dependency.

## Repository / branch model

Current repository remains `CheshirskyCat63/DEADRACE` until an explicitly authorized clean-repo migration occurs. There must not be two active product repositories.

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
- Space = gaze/free-look while held;
- `1–4 = START` quick reach;
- `LMB = ATTENTION` focus/retain valid physical interaction;
- `RMB = GO` contextual manipulation/commit;
- quick targets: `1 signals / 2 horn / 3 gearbox / 4 handbrake`;
- Q clutch, W brake, E throttle;
- clutch wheel semantics adjust release speed, not instantaneous clutch pressure;
- hidden steering continuity cannot choose route/lane/gap, brake, throttle, overtake or avoid traffic.

Old `RMB grab / LMB manipulate` and old open 1/2/4 assignments are SUPERSEDED.

## FIRST EURO session direction

- direct start into playable Tatra/world;
- one personal character and deterministic personal CityCode;
- permanent night/night-shift visual baseline with changing weather;
- `2 real-world hours` is a QA/endurance soak target only; gameplay Workday duration/time semantics remain owner-open Pack `I08`;
- sleep = legal diegetic end-day/save+exit direction;
- quit-anywhere is a technical proposal only; exact permission/consequences/resume/checkpoint/anti-reload behavior remains owner-open `I03/I04/I13` under `CD-560/CD-602`;
- ordinary persistent state must survive through explicit schemas/transactions, not transient Actor snapshots.

## FIRST EURO world scope

Level1 + Level2 are fully playable in year one. Level3 gameplay is POST-FIRST-EURO and only a compatible schema/route extension boundary is required now.

World identity is `CityCode + GeneratorVersion + ContentSetVersion + persistent deltas`. Use Unreal World Partition/PCG/engine facilities where appropriate with a thin deterministic recipe layer; do not create a bespoke world streaming engine unless a proven gap requires it.

## Vehicle baseline

Hero Tatra: bespoke early/Gen-1 603-family; rear-mounted air-cooled V8; RWD; no ABS/ESP; base 1450 kg; full fuel 1550 kg; heroine 58 kg; daughter 49 kg; 1657 kg benchmark; 2107 kg max fixture; current 180 hp / 240 Nm / 195 km/h target.

Level1 residual magnetism is linear by authoritative mass: 5.0 s @1657 kg → 4.0 s @2107 kg; lighter legal states cap at 5.0 s.

## Taxi / Neural direction

FIRST EURO contains complete FareSession/taxi work plus persistent repeat clients/basic Neural. PassengerIdentity persists independently from a materialized passenger Actor. Repeat orders reuse the normal order/fare pipeline.

## Services

FIRST EURO ServiceNodes are automotive only: Parking, Garage/Tuning, Parts, Repair/Service, plus Practice Hangar where required. Moving refueling is a separate FIRST EURO live-road session over shared FuelTank/Economy/VehicleTelemetry/RoadGraph.

Mall/food/bar/club/social nodes are POST-FIRST-EURO.

## Enforcement / recovery extensions

FIRST EURO implements stable `EnforcementEvent` facts and ordinary rules/fines. Full Taxi Regulator is POST-FIRST-EURO.

Daily insurance is POST-FIRST-EURO. FIRST EURO only preserves generic `RecoveryPolicy/RecoveryHook`; exact current terminal-crash recovery remains Pack I06 / `CD-598/CD-750` owner confirmation.

## Hot-path setup rules

No unbounded Tick, global Actor scans, sync loads or repeated spawn/allocation storms in normal gameplay hot paths. Bound populations/queues/caches; pool/materialize only what runtime needs.

## Verification rule

Every executable result records exact commit/build, UE/plugin/config/schema/generator/profile versions, deterministic IDs/seeds, expected/observed behavior and artifacts. Build success alone does not establish gameplay verification.

PINK CAB remains **NOT IMPLEMENTED / NOT VERIFIED** until such evidence exists.
