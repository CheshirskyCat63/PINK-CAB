# PINK CAB · Active Product Baseline

Status: ACTIVE PRODUCT AUTHORITY COMPANION
Jira root: `CD-519`
BASE-100 code architecture: `docs/PINK_CAB_BASE100_CODE_ARCHITECTURE.md` / Confluence `11239425` / `CD-746..753`
Readiness: `docs/PINK_CAB_START90_READINESS.md` / Confluence `6553617` / `CD-660/CD-661`
Confluence baseline: `5537802`
Vehicle technology: `docs/PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md` / page `13303842` / `CD-785..CD-792`
Control/mechanics release: `docs/PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md` / Confluence `16744449` / `CD-848`
Handling companion: `docs/PINK_CAB_TATRA_HANDLING_E34_REFERENCE.md` / page `6225936` / `CD-648`
Mass/load: `docs/PINK_CAB_VEHICLE_MASS_LOAD_DYNAMICS.md`
Future daily insurance authority: `docs/PINK_CAB_DAILY_INSURANCE_CRASH_RECOVERY.md` / page `10321936` / `CD-741..745` — POST-FIRST-EURO
Repository: `CheshirskyCat63/PINK-CAB` is the sole active PINK CAB technical truth; `DEADRACE` is legacy/migration-source only.

## Current control/mechanics precedence

The 2026-09-18 owner correction takes precedence for vehicle controls/mechanics. See [`PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`](PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md) and Confluence [page 16744449](https://cheshire-division.atlassian.net/wiki/spaces/TWC/pages/16744449/47+PINK+CAB+Control+Vehicle+Mechanics+Release+Contract). Older 530i, left-forward reverse, fixed 0.20–1.20 clutch-range, START→ATTENTION→GO and yaw-edge-guard wording is superseded. The corrected matrix is SPECIFIED / NOT RUNTIME VERIFIED.

## Product / FIRST EURO horizon

PINK CAB is a first-person arcade-sim taxi/work/vehicle-parkour game in a persistent, effectively endless retrofuturist longitudinal megacity. The car is the workplace rather than an abstract avatar.

**FIRST EURO = first 12 months of development = PC single-player product.** Full Level 1 + Level 2 gameplay/runtime belongs inside this horizon. Repeat clients/basic Neural, automotive ServiceNodes and moving refueling are also FIRST EURO systems.

POST-FIRST-EURO implementation: multiplayer/coop/common rooms/online clubs; Level 3 gameplay; lifestyle ServiceNodes such as mall/food/bar/club; full Taxi Regulator; daily insurance. FIRST EURO preserves only the extension interfaces needed to add these later without core rewrites.

The player is a woman around 45, **1.65 m / 58 kg**, driving one neon-pink bespoke early/Gen-1 Tatra 603-family taxi. Her 17-year-old daughter, **1.55 m / 49 kg**, occupies the rear/service area and progresses through `daughter → conductor/service role → later onboard operator`. No third permanent conductor NPC exists.

## BASE-100 readiness meaning

BASE-100 measures **code/logic/numeric/technical specification coverage** for FIRST EURO, not visual completion. Visual art, mesh/material/light polish, decorative animation/audio and narrative presentation are excluded unless they create an implementation-facing state/input/output/performance contract.

Every runtime domain must expose: `PURPOSE → BUY/ENGINE/WRITE → AUTHORITATIVE OWNER → STATE MACHINE → INPUTS → OUTPUT EVENTS → DATA MODEL → NUMERIC CONFIG → PERSISTENCE → DEPENDENCIES → FAILURE MODES → PERFORMANCE BOUNDS → TEST FIXTURES → EXTENSION POINTS`.

Prefer proven Unreal features/plugins/assets over custom frameworks. Selected third-party systems sit behind PINK CAB adapters. One authoritative owner per responsibility; no competing production solvers. C++ owns interfaces/state/persistence/invariants; Blueprint remains thin composition/orchestration; DataAssets/DataTables/config own tunable values.

## Current START-90 snapshot — 2026-09-12

Technical Owner Pack 01 contains **196 code-facing rows**: 102 LOCKED, 3 CALIBRATION, 69 PROPOSED DEFAULT, 22 OPEN.

Scoring: LOCKED/CALIBRATION = 1.0; PROPOSED DEFAULT = 0.5; OPEN = 0.0.

**Current item-weighted START-90 specification readiness = 71.2%. Gate FAIL. Gap = 18.8 percentage points.**

Domain snapshot: CORE 69.1%; VEHICLE 71.4%; TAXI 85.0%; STATE 92.9%; WORLD 57.8%; SERVICE 56.5%. SCOPE `CD-753` is separately 100% locked.

If all 69 proposed defaults are accepted, score becomes 174/196 = 88.8%. At least 3 of the remaining 22 OPEN rows must then close to cross START-90 at 177/196 = 90.31%.

Historical ≈59% and 38.2% scores are retired. Broad production remains HOLD; narrow bootstrap/specification/calibration/admin work is allowed.

## Hero Tatra / technology

Locked identity:

- early/Gen-1 bespoke Tatra 603-family;
- rear-mounted air-cooled V8, RWD;
- three front lamps; center lamp steering-linked/directional and somewhat farther-reading;
- rear/tail intentionally stretched without uniform body rescale;
- passenger side has no fixed B-pillar;
- opposed passenger doors create one wide continuous right-side aperture;
- physical in-cabin passenger-door lever.

Technology stack:

- **Chaos Vehicles / native Unreal physics** = sole production road-dynamics owner;
- **Vehicle Damage System — Debugging Delight Studios** = sole permanent runtime body-deformation owner;
- PINK CAB code = input/profile adapters, Tatra Expression, Vehicle Health bridge, persistence and normalized telemetry;
- no parallel competing hero-Tatra dynamics solver or hidden second destruction authority without an explicit migration decision.

## Handling — E34-like control / Tatra expression

BMW E34 **535i** is the current gameplay capability/control reference only: calm center, progressive steering, predictable combined grip/release, readable/catchable RWD oversteer, straight-line composure and communicative braking.

Not imported: BMW front-engine layout, ABS, donor power/top-speed, exact BMW tire/brake/suspension/chassis hardware or interior.

PINK CAB retains no ABS, no ESP, current **180 hp / 240 Nm** and **195 km/h** terminal target.

Wet high-speed behavior is locked: rain + rapid lane change above roughly 160 km/h + maintained/excess throttle may progressively saturate rear combined grip and lead to spin; easing excessive throttle normally restores rear lateral reserve and lets the player recover through steering. No scripted threshold, automatic countersteer or ESP rescue.

Speed character: 0–100 lively/comfortable; 100–140 confident; 140–170 visibly barge-like and suspension-busy; 170–195 still power-capable while chassis/road workload becomes the challenge.

Chaos owns physical trajectory/forces. Tatra Expression may slightly amplify roll/pitch/heave, body/cabin shake, trim rattle, steering tremor and bounded camera motion but cannot independently change grip or collision outcome.

## Closed Tatra calibration baseline

The latest 50-question handling pack is retired from owner intake. Low-level values are CALIBRATION inside the locked observable contract.

Current targets: 45/55 F/R @1657 kg; historical chassis baseline ~2750 mm wheelbase / ~1485 mm front track / ~1400 mm rear track; tall 185R15-like visual tires; 41°±2° inner-wheel lock; ~2.5 steering-wheel turns lock-to-lock; no mouse dead-zone; 18–40° useful sideslip; ~4.5–5.5° physical roll @~0.8g; ~1.35× nominal visual roll; 60/40 starting brake bias; mild LSD; ~1700 Nm/rear-wheel handbrake seed. **Clutch release timing remains adjustable but its final numeric range is RECONCILE/NOT OWNER-LOCKED after the 2026-09-18 correction.** 1657 kg remains the headline mass benchmark.

## Mass / load

- base Tatra **1450 kg**;
- fully fueled **1550 kg**;
- fuel contribution **100 kg**;
- heroine **58 kg**;
- daughter **49 kg**;
- female passenger **55–70 kg**;
- male passenger **65–90 kg**;
- maximum passengers 5;
- runtime total = base + current fuel + heroine + daughter + exact boarded passenger masses;
- reference full-fuel crew-only **1657 kg**;
- declared max passenger fixture **2107 kg**.

More load modestly worsens acceleration/braking, increases roll/pitch/settling/expression, and modestly improves abrupt wallride-separation resistance at matched conditions. One continuous handling model remains.

Level1 residual magnetism is **linear by total mass**: **5.0 s @1657 kg → 4.0 s @2107 kg**, lighter states capped at 5.0 s. Exact reacquisition reset semantics remain owner-open `M05`.

## Input / physical controls

Current grammar: mouse steers by default; hold Space for gaze/free-look and target search; `1–4` quick-recall the saved physical targets for turn signals / horn / gearbox / handbrake without actuating them. RMB brings/retains the right hand on the current target where a grip is required. LMB presses or holds momentary controls (for example a short/long horn press). Mouse wheel moves detents/rotaries/incremental controls where that control supports it. Looking at, quick-recalling or gripping a target never implicitly actuates it.

Steering, throttle, brake, clutch state/release speed and handbrake are continuous. Holding E commands throttle immediately: each fresh standstill launch starts from the playable 45% pedal target, while E+wheel is optional fine adjustment in 5% steps. Releasing E still commands zero throttle, so this is not auto-throttle. Q+wheel adjusts clutch release timing; W+wheel doses brake; W+E may coexist. If Q/W/E overlap, the wheel has one recipient with priority E → W → Q. The H-gate is 1/3/5 over 2/4/R with a neutral cross-gate. No Drift Mode, auto-throttle, auto-rev-match, auto-countersteer, yaw rescue, ABS or ESP.

Input reaches the Chaos-backed `IPinkCabVehicleDynamicsProvider` through the PINK CAB adapter; cockpit controls publish semantic commands/events rather than directly mutating unrelated subsystem state. FIRST EURO semantic KBM rebinding is locked under C08 and cannot change this physical-control model. One C++ semantic router owns raw KBM intake, one bounded target is active, and valid-target feedback is a textless translucent reticle that brightens to a white-matte state. Left/right hand choice is presentation only and may switch automatically; instruments/controls must work before full hand animation fidelity.

## Core taxi work

`ORDER / ROUTE → SEE PHYSICAL PASSENGER GROUP → APPROACH OFFICIAL STOP OR CURB → FULL STOP → ASSESS → ADMIT/DECLINE → PHYSICAL DOOR LEVER → PILLARLESS APERTURE → GROUP BOARDS → DRIVE → FULL STOP DESTINATION → PAYMENT/CONSEQUENCE → DOORS OPEN → EXIT → NEXT`

Route is provided. Ordinary roads have no traffic lights and no systemic standing traffic jam. Pickup/dropoff require full stop. Groups 1–5 may board as one compressed event after stop + door opening; seats rear3→front2; actual passenger mass enters/exits exactly once.

Physical taximeter = **distance + elapsed fare time**. `METERED` and `OFF_METER` are explicit. Curb pickup may offer higher tips/off-meter opportunity with higher non-payment/complaint/rule risk. Unpaid escape can occur only after doors open; committed payment blocks ordinary unpaid escape. Exact full-stop epsilon/dwell and meter/receipt timing remain owner-open in F04–F08/CD-672.

## Repeat clients / basic Neural — FIRST EURO

FIRST EURO includes persistent repeat clients and a compact Neural layer without broad social simulation.

Procedural PassengerTemplate and persistent PassengerIdentity are distinct. A persistent identity can retain stable ID, trip history, relationship/preferences, contact permission, repeat-order eligibility and simple data-driven messages. Passenger identities may exist logically without a materialized Actor. Repeat orders re-enter the normal Order/Fare flow; Neural never owns taxi, vehicle or economy truth.

## Damage / mechanical health

`CD-600` is RESOLVED/DONE. FIRST EURO includes persistent authored damage-state consequences, glass/lamp damage, door jam/latch, tire/wheel path, authored alignment/suspension consequences, brake heat/fade/hydraulic health, **clutch heat/wear/torque-transfer degradation**, **gearbox/transmission conflict wear/damage**, and **air-cooled V8 oil/head/fan/oil-cooler/airflow thermal health**, plus repair/persistence.

Generic coolant/radiator-overheat wording is SUPERSEDED. Full soft-body structural fracture, detailed engine internals, CFD/fluid volumes, detailed tire thermodynamics, catastrophic fuel explosion/fire chain, centimeter brake plumbing and occupant injury are off/post-sale by default.

## Daily insurance / terminal recovery

Daily insurance mechanics under `CD-741..745` are preserved as future backlog authority but are **not implemented inside the first 12-month FIRST EURO product** and do not reduce its BASE-100/START-90 readiness.

FIRST EURO keeps only a generic `RecoveryPolicy/RecoveryHook` extension boundary connecting terminal vehicle state, Workday/Session and Economy/Persistence. No insurance toggle, premium, claim, policy persistence or extra-life transaction is required in year one. FIRST EURO terminal recovery is **LOCKED under `I06` / CD-750**: active fare fails, Workday ends, atomic save commits, and the still-damaged Tatra returns through Repair next Workday with no free reset; fee/debt fallback remains under Economy policy.

## Persistent city / Level 1 / Level 2

The city is an effectively endless longitudinal network reconstructed from `CityCode + GeneratorVersion + ContentSetVersion + persistent deltas`.

Level1: five express lanes; right-side ordinary exits/interchanges; lower passenger cars + motorcycles/sidecars; no ordinary lower heavy freight/buses; no traffic lights/systemic standing jams; road defects; debris transition; seven-band wallride; five-lane freight ceiling always opposite lower road; ceiling/poplar fast side-swap.

Level2 is fully inside FIRST EURO and owns suspended bus + metro + stations. Bus lane count remains calibration/greybox after its architecture contract; metro has two tracks per side. Direct Tatra-on-metro driving is not canon unless separately locked. Exact station gameplay function remains owner-open `N05`.

Level3 gameplay is POST-FIRST-EURO; first-year CityCode/road schema must only retain a compatible future tier extension boundary.

Prefer Unreal World Partition/PCG/engine streaming plus a thin deterministic PINK CAB recipe layer rather than a custom world-streaming engine. One machine-readable road/lane graph should feed generation, routing, traffic and rule lookup.

## Traffic / enforcement interface

Ordinary distant traffic should use bounded logical/kinematic simulation rather than hero-level Chaos for every vehicle. Physically relevant traffic may materialize near the player/collision context. Traffic populations, incidents and queues must remain bounded.

Full Taxi Regulator/inspection gameplay under `CD-723` is POST-FIRST-EURO. FIRST EURO still defines a stable `EnforcementEvent` contract and ordinary rules/fines may consume those events. A future regulator subscribes without changing event producers.

## Automotive ServiceNodes — FIRST EURO

FIRST EURO ServiceNodes are:

- Parking;
- Garage/Tuning;
- Parts Shop;
- Repair/Service;
- **Practice Hangar only where Level1 training/acceptance requires it.**

They share one bounded destination/transition architecture with exact return to the same Tatra/CityCode and no duplicated vehicle/economy/persistence state. Parts and repairs use common inventory/VehicleBuild/Vehicle Health/Economy services rather than one-off systems.

Mall/food/bar/club/social lifestyle interiors are POST-FIRST-EURO. ServiceNode implementation form and Parking save/day-end role remain owner-open `O02/O03`.

## Moving refueling — FIRST EURO

Moving refueling is a first-year gameplay mechanic. It uses the common FuelTank, EconomyService, VehicleTelemetry and RoadGraph contracts and owns only its refueling session/state machine. Fuel transfer and money settlement must be save/crash/retry safe and exactly-once. NPC/hose/animation presentation follows the state machine and is never authoritative gameplay truth. Queue/slot, gap/tolerance, settlement and insufficient-funds behavior remain `P03/P05/P09/P11` owner-open.

## Production architecture priority

1. BASE-100 core/runtime/data/input/plugin/evidence contract (`CD-746/CD-747`);
2. native Chaos baseline/calibration and Tatra profile (`CD-785..CD-788`);
3. analog input, wet handling and Tatra Expression (`CD-733..735`);
4. native bounded destruction, Vehicle Health and damage persistence (`CD-790` plus current persistence owners);
5. integrated native vehicle proof (`CD-792`);
6. FareSession/passenger exchange + repeat-client/basic Neural contracts (`CD-749` plus existing fare/passenger owners);
7. Economy/Workday/save/exactly-once transaction layer (`CD-750`);
8. CityCode/world streaming/road graph/traffic/enforcement interface (`CD-751`);
9. automotive ServiceNodes + moving refuel (`CD-752`);
10. full Level1+Level2 composition and acceptance; post-year systems remain extension-only.

## Verification

Canonical runtime evidence now exists for the native Chaos vehicle/core/playable path through `CD-792`, `CD-802` and `CD-823..829`. Remaining unresolved work is design/authority reconciliation and later asset/content substitution; documentation alone still cannot promote unverified future mechanics.

Current code-only START-90 score is **66.6%** under `docs/PINK_CAB_START90_READINESS.md`; gate FAILS. PF-00 bootstrap and PF-01 Core Contracts have exact-build executable evidence; later systems remain NOT IMPLEMENTED / NOT VERIFIED until their own evidence exists.
