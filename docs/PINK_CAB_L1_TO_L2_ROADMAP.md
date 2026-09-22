# PINK CAB · Level 1 → Level 2 Delivery & Verification Roadmap

Implementation-facing mirror of Confluence `8716289`. This is dependency/proof order, not a calendar commitment.

FIRST EURO scope authority: `CD-746/CD-753`, `docs/PINK_CAB_BASE100_CODE_ARCHITECTURE.md`, Confluence `11239425`.

Core owners:

- BASE-100 code/runtime program: `CD-746`; domain packs `CD-747..753`;
- vehicle technology: Native Chaos `CD-785..792` / `docs/PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md`; current mechanics/recovery `CD-848`;
- Tatra handling owner-lock: `CD-729`; detailed calibration `CD-648..659`; physics/mass `CD-562`; power `CD-641`;
- Level1 gameplay: `CD-678` / proof `CD-679`;
- Level1 mass/contact: `CD-592` / QA `CD-701`;
- Level1 presentation: `CD-714..718` / proof `CD-719`;
- interchange: `CD-680` / proof `CD-681`;
- Level2: `CD-682` / proof `CD-683`;
- ServiceNode: `CD-576`, schema `CD-711`, QA `CD-713`;
- daily insurance `CD-741..745` is POST-FIRST-EURO and not on this first-year critical path.

## Maturity

`CANON → SPECIFIED → IMPLEMENTED → VERIFIED`

Temporary values are `EXPERIMENTAL / NON-AUTHORITY`. Plugin calibration is not an owner-open decision when the observable contract is already locked.

## FIRST EURO delivery boundary

First 12 months = **PC single-player**.

Included in year one: full L1+L2, hero Tatra, taxi/fare/passengers, repeat clients/basic Neural, automotive ServiceNodes, moving refuel, CityCode/streaming/traffic/rules/economy/save/build/QA.

Post-year implementation: multiplayer/coop/network stack, L3 gameplay, lifestyle ServiceNodes, full Taxi Regulator, daily insurance. Only extension interfaces required to avoid core rewrites remain in year-one architecture.

## Target integrated path

`LEVEL1 ROAD → RIGHT-ENTRY INTERCHANGE / WALLRIDE OPTIONS → LEVEL1 CEILING → RETURN → LEVEL2 BUS LAYER → STATION ZONE → RETURN TO ROAD`

with reverse CityCode reconstruction, exact Tatra/load identity, deterministic recovery and no ServiceNode ownership duplication.

## Phase 0 — BASE-100 code/runtime contracts

Owners `CD-746..753`, readiness `CD-660/CD-661`.

Before broad implementation, lock the architecture contracts that would otherwise force structural rewrites:

- exact runtime/toolchain/project-module policy;
- data/profile/versioning rules;
- Enhanced Input and physical-control adapters;
- Economy/Workday/save/transaction ownership;
- CityCode/chunk/road-graph/traffic ownership;
- FareSession/passenger/Neural ownership;
- ServiceNode/moving-fuel ownership;
- plugin buy-before-build and adapter doctrine;
- common performance/evidence constraints.

`START-90` is calculated against this code-only denominator. Visual completeness does not count.

## Phase A — geometry / authority truth

Owners `CD-563/CD-589/CD-588`.

Lock Level1 cross-section, right entry/exit, wallride/ceiling/ramp/frontage clearances, interchange tube envelope, Level2 bus/metro/station geometry and streaming budgets where code/tests require exact values.

## Phase B — production vehicle stack

Owners: Native Chaos program `CD-785..792`; current mechanics/recovery `CD-848`; damage persistence remains in Vehicle Health/save contracts.

Order:

1. pin Unreal Engine 5.8 Native Chaos production line and PINK-CAB provider/profile versions;
2. build one authoritative versioned Native Chaos/PINK-CAB Tatra profile;
3. connect physical analog controls through one adapter (`CD-733`);
4. tune dry/wet/storm combined-grip behavior and wet >160 km/h recovery (`CD-734`);
5. build presentation-only Tatra Expression Layer (`CD-735`);
6. integrate bounded authored/native damage states + detachable parts + Vehicle Health consequences;
7. map authored hit zones to bounded Vehicle Health consequences (`CD-737`);
8. persist damage state (`CD-740`);
9. prove stack in `CD-738`.

Architectural rule: Native Unreal Chaos Vehicles behind `IPinkCabVehicleDynamicsProvider` is the sole hero-Tatra road-dynamics solver. Damage uses bounded authored/native states + Vehicle Health. Required third-party vehicle/damage plugins are none. No parallel solver.

## Phase C — Tatra handling / road-feel acceptance

Owners `CD-729`, calibration `CD-648..659`, Native Chaos `CD-785..792`, current mechanics `CD-848`, QA/surface evidence under current recovery authority.

Locked contract:

- E34-like calm/progressive/readable steering/grip behavior;
- rear-engined old-Tatra mass/layout/expression;
- no BMW ABS/front-engine/power/hardware transplant;
- no ABS / no ESP;
- 180 hp / 240 Nm current authority, 195 km/h target;
- continuous steering/throttle/brake/clutch-release/handbrake commands;
- wet rapid lane change above roughly 160 km/h may saturate rear combined grip under excess throttle; easing throttle normally restores lateral reserve;
- physical Native Chaos chassis state separate from expression/presentation;
- road defects use one coherent physical→expression chain.

The previous Tatra 50-question pack is closed as owner intake. Low-level constants are CALIBRATION.

## Phase D — FareSession / passenger / Neural foundation

Owners `CD-749` plus existing fare/passenger/cabin owners.

Deliver one authoritative fare/order flow, full-stop pickup/dropoff eligibility, boarding/exchange transaction boundaries, taximeter state, payment/evasion transaction ordering, PassengerTemplate vs PassengerIdentity split, persistent repeat-client history/basic Neural and repeat orders through the normal Order/Fare pipeline.

Visual passenger presentation is not a Phase D readiness blocker unless it changes state/input/output contracts.

## Phase E — Economy / Workday / save foundation

Owner `CD-750` plus `CD-560/CD-601`.

Deliver one EconomyService/transaction ledger, Workday/Session ownership, versioned save schema/migrations/checkpoints, exact retry semantics, and generic `RecoveryPolicy/RecoveryHook` extension point.

Daily insurance is not implemented here. FIRST EURO terminal recovery is locked by `I06`: fail active fare, end Workday, atomic save, recover the still-damaged Tatra through Repair next Workday, no free reset; `H04/H05` govern insufficient funds.

## Phase F — Level1 moving road flow

Owners `CD-567/CD-568/CD-692`, world contract `CD-751`.

Deliver bounded passenger cars + motorcycles/sidecars, lane speeds/gaps, right-exit friction, signs/cameras/parking and reproducible seeds. No traffic lights or systemic standing jams. Ordinary buses belong to Level2; heavy freight belongs to Level1 ceiling.

Distant traffic should not receive hero-level full Chaos simulation by default; materialization/collision relevance is controlled by common traffic architecture.

## Phase G — Level1 identity / presentation kit

World owners `CD-684/CD-685/CD-693/CD-603`; presentation `CD-714..719`; budget `CD-590`.

Presentation work remains a production phase but **does not inflate BASE-100 code readiness** except where it sets measurable performance/runtime contracts.

## Phase H — Level1 vertical oval / load-sensitive contact

Owners `CD-687/CD-686/CD-689/CD-688/CD-676`; vehicle `CD-562/CD-592/CD-732`; QA `CD-701`.

Order:

1. debris transition;
2. seven-band wallride;
3. load-aware wallride contact;
4. stable ceiling contact;
5. 5.0 s @1657 kg residual endpoint;
6. 4.0 s @2107 kg residual endpoint;
7. verify locked linear mass interpolation + lighter 5.0 s cap;
8. five-lane opposite-flow freight;
9. moving strip/gap interaction;
10. poplar side-swap;
11. return to road.

For 1657–2107 kg: `timeout_s = 5.0 - (total_mass_kg - 1657) / 450`.

Only magnetic force/contact/reacquisition calibration remains. Reacquisition reset semantics must be owner-locked before implementation guesses.

## Phase I — incident theatre

Owner `CD-691`: deterministic bounded incident families, moving bypass and cleanup.

## Phase J — Automotive ServiceNodes + moving refuel

Owners `CD-752`, ServiceNode `CD-576/CD-711/CD-713`, moving-fuel current owner family.

FIRST EURO node types are only:

`Parking / Garage-Tuning / Parts / Repair-Service`.

All share one transition/persistence/economy/VehicleBuild/VehicleHealth architecture. Mall/food/bar/club/social nodes are post-year.

Moving refueling is FIRST EURO live-road gameplay and must reuse common FuelTank/Economy/VehicleTelemetry/RoadGraph state. Fuel/money settlement must be exactly-once across save/crash/retry.

## Phase K — Corridor Zero

Gate `CD-679`; QA `CD-700..703`; mass/contact `CD-701`; presentation `CD-719`.

Evidence covers cross-section, continuous moving traffic/no signals, Native Chaos road response, wallride, linear load-sensitive magnet rule, freight, poplar side-swap, incidents, automotive ServiceNode access, rules, reverse reconstruction and streaming.

## Phase L/M/N — right-entry pipe interchange

`CD-696/CD-564` approach graph → `CD-697/CD-698/CD-699` two-lane one-way pipe/tube, building penetration, traffic/vignettes → gate `CD-681`, QA `CD-704`.

Same Native Chaos/PINK-CAB Tatra handling applies. No interchange-only drift physics.

## Phase O — Level2 cross-section lock

Owners `CD-705/CD-706/CD-707`, geometry `CD-589`.

Level2 is a full FIRST EURO target. Final bus lane count may remain data/calibration until greybox lock; metro has two tracks per side. Lock code-facing elevation/spacing/direction/headway/dwell/counter-phase, station gameplay function/geometry/access and Level1 sightlines.

## Phase P/Q/R — live transit → bus traversal → Transit Zero

Deliver reproducible bus/metro/stations (`CD-706/CD-707/CD-709`), Tatra bus-layer traversal (`CD-708/CD-572/CD-592`), then proof `CD-683/CD-710`.

Current fuel/passenger load remains part of Tatra state. Tier2 may not silently copy Tier1 contact numerics.

## Cross-phase remaining code blockers / owner locks

- `CD-747`: exact UE/runtime module/feature-flag/input policy;
- `CD-749`: remaining FareSession/meter/passenger/Neural edge rules;
- `CD-750`: debt, save/checkpoint/quit/workday/default crash recovery;
- `CD-751`: CityCode/chunk/road graph/traffic/enforcement numerics;
- `CD-752`: ServiceNode + moving-fuel state/numeric rules;
- `CD-557/CD-559`: exact runtime/toolchain/build/CI platform locks;
- `CD-560/CD-602`: autosave/quit/resume/anti-reload details;
- `CD-589`: road/transit geometry + streaming;
- `CD-592`: wallride/magnetic/bus force/contact/reacquisition calibration, not residual interpolation;
- `CD-601`: negative balance/debt/bankruptcy;
- `CD-705`: final Level2 bus lane count if not left calibration;
- `CD-711`: ServiceNode schema/persistence.

Daily insurance `CD-741..745`, full Taxi Regulator `CD-723`, future common lobby `CD-712`, L3 gameplay and lifestyle nodes are POST-FIRST-EURO implementation and are not first-year blockers beyond their extension interfaces.

## Evidence

Every proof records exact build/commit, UE/plugin/profile/schema versions where relevant, generator/content versions, CityCode/seeds, route/module IDs, vehicle mass, state-machine inputs/outputs, transaction ids, expected/observed results, bounded runtime counters and artifact paths.

Documentation/admin can be internally consistent while runtime remains **NOT IMPLEMENTED / NOT VERIFIED**.
