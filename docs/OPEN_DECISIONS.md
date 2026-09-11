# PINK CAB · Open Decisions Register

Confluence durable register: `5832744`
Jira live registry: `CD-588`
Product root: `CD-519`
BASE-100 code program: `CD-746`
FIRST EURO scope: `CD-753`
Code architecture: `docs/PINK_CAB_BASE100_CODE_ARCHITECTURE.md` / Confluence `11239425`
Technical owner pack: `docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md`
Vehicle technology: `10977288` / `CD-730`
Daily insurance: `10321936` / `CD-741..745` — POST-FIRST-EURO

## State rule

- `OPEN` — unresolved code/game-logic choice; implementation may not silently choose it.
- `PARTIAL` — direction exists, implementation-facing contract incomplete.
- `LOCKED DIRECTION` — structure/observable behavior fixed; plugin/profile tuning may remain.
- `LOCKED SPEC` — value/identity/contract recorded in live authority.
- `CALIBRATION` — product choice closed; implementation tunes named profile/plugin inside locked envelope.
- `POST-FIRST-EURO` — not implemented in first 12 months; preserve only explicitly required extension boundary.
- `IMPLEMENTED` / `VERIFIED` require executable evidence beyond documentation.

Visual/art/presentation questions are excluded from BASE-100 unless they alter runtime state/input/output/performance contracts.

## `OD-BASE100-CODE-001` — LOCKED SPEC

BASE-100 = **100% implementation-facing code/logic/numeric/technical specification for the complete 12-month FIRST EURO PC single-player product**.

Every runtime domain must define:

`PURPOSE -> BUY/ENGINE/WRITE -> AUTHORITATIVE OWNER -> STATE MACHINE -> INPUTS -> OUTPUT EVENTS -> DATA MODEL -> NUMERIC CONFIG -> PERSISTENCE -> DEPENDENCIES -> FAILURE MODES -> PERFORMANCE BOUNDS -> TEST FIXTURES -> EXTENSION POINTS`

FIRST EURO includes full L1+L2; hero Tatra; taxi/fare/passenger; persistent repeat clients + basic Neural; CityCode/streaming/road graph/traffic/rules/fines; `EnforcementEvent`; automotive ServiceNodes; moving refueling; build/config/save/migration/QA.

Post-FIRST-EURO implementation: multiplayer/coop/shared rooms/network stack; L3 gameplay; lifestyle ServiceNodes; full Taxi Regulator; daily insurance.

Previous ≈59% six-domain readiness snapshot is historical. Current normalized code-only START-90 readiness is **47.4%** from 196 Technical Owner Pack rows.

## `OD-ARCH-BUY-BEFORE-BUILD-001` — LOCKED DIRECTION

- audit Unreal/proven purchased plugins/assets before large custom subsystem work;
- vendor systems sit behind PINK CAB adapters/interfaces;
- one production owner per responsibility; no duplicate competing solvers after lock;
- C++ owns interfaces/state/persistence/invariants/critical orchestration;
- Blueprint remains thin composition/orchestration;
- DataAssets/DataTables/config own gameplay tuning;
- logical/persistent identity separate from materialized Actor;
- bounded Tick/queues/pools/caches; no world-wide scans or hot-path sync loads/spawns;
- explicit save/config/generator versioning and migrations;
- exactly-once persistent monetary/fare/fuel/service transitions;
- one evidence runner family; build PASS != gameplay VERIFIED.

Exact UE version/module split/feature flags and any explicit exceptions remain in Technical Owner Pack 01.

## Vehicle technology / Tatra

### `OD-TATRA-STACK-001` — LOCKED SPEC

FGear Vehicle Physics = sole hero-Tatra road-dynamics owner. Debugging Delight Vehicle Damage System = sole permanent runtime deformation owner. PINK CAB code owns adapters/profile/Expression/Vehicle Health/persistence/normalized telemetry only.

No parallel Chaos/custom hero-Tatra physics or second deformation solver without reopening `CD-730`.

### `OD-TATRA-MASS-001` — LOCKED SPEC

Base **1450 kg**; full-fuel vehicle **1550 kg**; full tank **100 kg**; heroine **58 kg**; daughter **49 kg**; passenger masses actual; full-fuel crew reference **1657 kg**; declared max fixture **2107 kg**.

### `OD-TATRA-E34-FEEL-001` — LOCKED DIRECTION / FGear CALIBRATION

E34 530i V8 5MT is behavioral-control reference only. PINK CAB retains rear-engine air-cooled V8, no ABS/ESP, 180 hp / 240 Nm and 195 km/h targets. Wet rapid lane change above ~160 km/h with maintained/excess throttle can progressively saturate rear grip; throttle reduction normally restores lateral reserve for steering recovery. No scripted drift threshold or automatic rescue.

The Tatra 50-question pack is retired. Low-level plugin constants are CALIBRATION.

### `OD-L1-MAG-LOAD-001` — LOCKED SPEC

`1657 kg -> 5.0 s`; `2107 kg -> 4.0 s`; lighter legal states cap 5.0 s; interpolation linear by authoritative total mass. Reacquisition/reset semantic remains an owner question in Pack `M05`; detailed force/contact tuning is calibration under `CD-592/CD-701`.

## Damage / Vehicle Health

### `OD-DAMAGE-001` — RESOLVED / LOCKED SPEC

`CD-600` DONE. FIRST EURO includes permanent VDS deformation, glass, lamps, door jam/latch, tire/wheel path, authored alignment/suspension consequences, brake heat/fade/hydraulic health, air-cooled V8 oil/head/fan/oil-cooler/airflow thermal health, repair and persistence.

No generic coolant/radiator system. No universal vehicle HP model as primary functional damage owner. Terminal failure is capability-based.

## Daily insurance

### `OD-INSURANCE-DAY-001` — POST-FIRST-EURO

`CD-741..745` / page `10321936` preserve future design authority. The insurance feature is **not implemented in FIRST EURO and does not lower first-year BASE-100/START-90**.

FIRST EURO keeps only a generic `RecoveryPolicy/RecoveryHook` boundary. The exact year-one uninsured terminal-crash behavior is independently OPEN in `CD-750` / Pack `I06`.

Future premium/deductible/claim settlement/repair-depth/debt questions remain future only.

## Passenger / taxi / Neural

### `OD-PAX-STOP-001` — LOCKED DIRECTION

Pickup and normal drop-off require deliberate full stop. Group 1–5 boards after eligible stop + physical door opening; rear3→front2; exact passenger masses apply exactly once. Exact stop epsilon/dwell remains OPEN.

### `OD-METER-BASIS-001` — BASIS LOCKED / SEQUENCE OPEN

Taximeter uses distance + elapsed fare time. METERED/OFF_METER are one-system modes. Exact START/STOP/reset/receipt timing and several fare-edge rules remain OPEN in `CD-672/CD-749`.

### `OD-NEURAL-FIRST-EURO-001` — LOCKED SCOPE / DETAILS PARTIAL

Persistent repeat clients + basic Neural are FIRST EURO; broad social/life simulation is not.

Procedural PassengerTemplate and persistent PassengerIdentity are distinct. Persistent identity can retain stable ID, trip history, compact relationship/preferences, contact permission, repeat-order eligibility and simple data-driven messages. Repeat orders reuse normal Order/Fare pipeline; identity can exist without Actor.

Still OPEN: promotion rate/scoring and exact compact relationship axes.

## Road / City / traffic

No traffic lights/red-light branch. No designed systemic standing-jam state. One road/lane graph is the proposed shared owner for generation/routing/traffic/rules.

L1 + L2 are full FIRST EURO gameplay/runtime and must share one CityCode/chunk/persistence architecture. L3 gameplay is POST-FIRST-EURO; only a compatible extension boundary remains in year one.

Still OPEN: chunk/lookahead/cache/migration/safe-spawn numerics, road/interchange implementation dimensions, physical traffic materialization rules, density/headway/gap/incident persistence, speed/fine/reputation/toll values.

## Taxi Regulator

### `OD-TAXI-REG-001` — POST-FIRST-EURO IMPLEMENTATION

Full professional inspection/regulator gameplay is post-year. FIRST EURO implements stable `EnforcementEvent` producers/consumers and ordinary rule/fine logic. Future regulator subscribes without producer rewrites.

## ServiceNode

### `OD-SERVICENODE-FIRST-EURO-001` — LOCKED SCOPE / DETAILS PARTIAL

FIRST EURO ServiceNode types: `Parking / Garage-Tuning / Parts / Repair-Service`, plus Practice Hangar only where Level1 training/acceptance requires it. They use one common transition/persistence/economy/VehicleBuild/VehicleHealth architecture.

Mall/food/bar/club/social lifestyle nodes are POST-FIRST-EURO.

Still OPEN: transition implementation form, Parking checkpoint/day-end responsibilities, exact parts/inventory schema details.

## Moving refueling

### `OD-MOVING-FUEL-FIRST-EURO-001` — LOCKED SCOPE / DETAILS PARTIAL

Moving refueling is FIRST EURO live-road gameplay. It reuses `FuelTank`, `EconomyService`, normalized `VehicleTelemetry` and `RoadGraph`; it owns only its refuel-session state machine.

Fuel credit and money settlement must be save/crash/retry safe and exactly-once. NPC/hose/animation presentation is not authoritative gameplay truth.

Still OPEN: queue/slot policy, distance/speed envelope, target gap, warning/disconnect behavior, settlement timing and insufficient-funds interaction.

## Newly resolved owner lock ? 2026-09-11

- `A01 LOCKED`: Unreal Engine 5.8 is the production engine line. Bootstrap machine currently exposes UE 5.8.2 (`++UE5+Release-5.8`, CL 56702186). Any production engine-line change requires an explicit compatibility/migration decision.

## Current highest-impact OPEN code questions

Technical Owner Pack 01 priority:

`A03 A07 A10 A13 A14 A15 C08 F04 F05 F06 F08 F11 F12 F13 F14 F15 F16 F17 F18 F19 G02 G04 H04 H05 I01 I02 I03 I04 I06 I08 I09 I11 I12 I13 J05 J07 J09 J11 J12 K04 K06 K08 K11 K12 K13 L05 L06 L07 M05 N05 N06 N07 N09 O02 O03 P03 P05 P09 P11`.

These are the **59 genuine OPEN owner rows**. The **88 PROPOSED DEFAULT** rows remain engineering proposals awaiting owner acceptance/rejection and are not hidden canon.

Batch acceptance/rejection of the remaining `PROPOSED DEFAULT` rows will close many additional code contracts without re-asking already-resolved behavior.

## Proof rule

Documentation establishes CANON/SPECIFIED only. Exact runtime/plugin/profile/schema versions and executable evidence are required for IMPLEMENTED/VERIFIED claims.
