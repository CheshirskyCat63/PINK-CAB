# PINK CAB · FIRST EURO Verification Matrix

**Status:** CURRENT IMPLEMENTATION-FACING QA INDEX / NOT RUNTIME VERIFIED
**BASE-100:** `CD-746..753`; Confluence `11239425`
**Readiness:** `CD-660/CD-661`
**Open decisions:** `CD-588/CD-673`, Confluence `5832744`
**FIRST EURO scope:** `docs/FIRST_EURO_SCOPE.md`

## 1. Maturity / denominator

`CANON → SPECIFIED → IMPLEMENTED → VERIFIED`

This matrix follows the code-only BASE-100 denominator. Visual/art/presentation-only tests may remain useful production evidence but do **not** lower START-90/BASE-100 unless they create a code-facing state, interaction, dependency or performance contract.

FIRST EURO = first 12 months / PC / single-player / full Level1 + Level2. Post-year systems are listed separately and do not block first-year verification beyond their explicit extension boundaries.

## 2. Universal evidence record

Every executable result records where relevant:

- requirement/test ID;
- exact build/commit;
- UE version;
- exact FGear and VDS versions where vehicle code participates;
- content/config/schema/generator/vehicle-profile versions;
- CityCode and deterministic seeds;
- route/chunk/module/ServiceNode/workday/session IDs;
- vehicle, passenger, fare and transaction IDs;
- current fuel/crew/passenger/total Tatra mass;
- expected vs observed result;
- relevant vehicle/contact/slip/roll/pitch/Expression telemetry;
- frame-time/FPS/memory/Actor/Component/pool/queue counters where performance matters;
- warnings/errors/crashes;
- artifact path.

Build PASS is never gameplay VERIFIED.

---

## P0 · Foundation / reproducibility — `CD-747`

| Test ID | Required proof | State |
|---|---|---|
| `PC-T-P0-001` | clean checkout builds/packages with the exact locked UE + required plugin versions | BLOCKED exact stack/toolchain `CD-557/CD-596/CD-731` |
| `PC-T-P0-002` | executable reports build/commit/content/schema/FGear/VDS/profile identity | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-P0-003` | deterministic CityCode/Traffic/Passenger fixtures reproduce | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-P0-004` | logs/crashes identify exact build and version context | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-P0-005` | older/incompatible/newer persistent schema migrates or fails explicitly | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-P0-006` | vendor plugins are isolated behind declared PINK CAB adapters | LOCKED ARCHITECTURE / NOT IMPLEMENTED |
| `PC-T-P0-007` | hot-path audit finds no unbounded Tick/world scan/sync load or uncontrolled collection growth | SPECIFIED / NOT IMPLEMENTED |

## P1 · Hero Tatra / FGear / VDS / input — `CD-748`

Locked physical identity: 1450 kg base; 1550 kg full fuel; heroine 58 kg; daughter 49 kg; reference 1657 kg; declared max 2107 kg; no ABS/ESP; 180 hp / 240 Nm / 195 km/h target.

| Test ID | Required proof | State |
|---|---|---|
| `PC-T-STACK-001` | FGear is sole hero-Tatra road-dynamics solver | LOCKED / NOT IMPLEMENTED `CD-730/CD-738` |
| `PC-T-STACK-002` | Debugging Delight VDS is sole permanent body-deformation owner | LOCKED / NOT IMPLEMENTED `CD-730/CD-736/CD-738` |
| `PC-T-VEH-001` | Tatra remains controllable/readable across declared speed/load envelope | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-VEH-002` | versioned FGear profile reproduces owner handling targets and normalized telemetry | CALIBRATION / NOT IMPLEMENTED `CD-732/CD-738` |
| `PC-T-WET-001` | wet >~160 rapid lane change + excess throttle can progressively saturate rear; easing throttle restores reserve without ESP/autocountersteer | LOCKED DIRECTION / NOT IMPLEMENTED |
| `PC-T-MASS-001` | total mass = base + current fuel + crew + exact boarded passenger masses | LOCKED / NOT IMPLEMENTED |
| `PC-T-MASS-002` | passenger/fuel mass persists and applies exactly once | LOCKED / NOT IMPLEMENTED |
| `PC-T-MASS-003` | 1657 kg and 2107 kg fixtures reconstruct exactly | LOCKED / NOT IMPLEMENTED |
| `PC-T-ELEC-001` | no ABS system or authoritative intervention | LOCKED / NOT IMPLEMENTED |
| `PC-T-ELEC-002` | no ESP individual-wheel intervention; deliberate spin remains possible | LOCKED / NOT IMPLEMENTED |
| `PC-T-EXPR-001` | Tatra Expression may amplify presentation but cannot change FGear force/trajectory | LOCKED DIRECTION / NOT IMPLEMENTED |
| `PC-T-DMG-001` | VDS produces configured persistent deformation | LOCKED / NOT IMPLEMENTED |
| `PC-T-DMG-002` | only authored hit-zone mappings create Vehicle Health consequences | LOCKED / NOT IMPLEMENTED |
| `PC-T-DMG-003` | air-cooled oil/head/fan/oil-cooler/airflow thermal model; no generic coolant/radiator system | LOCKED / NOT IMPLEMENTED |
| `PC-T-INP-001` | Space transfers mouse steering→gaze→steering cleanly | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-INP-002` | START reach does not activate the target | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-INP-003` | LMB ATTENTION commits/retains hand on valid control | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-INP-004` | RMB GO owns contextual manipulation and mouse where required | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-INP-005` | 1 signals / 2 horn / 3 gearbox / 4 handbrake | LOCKED / NOT IMPLEMENTED |
| `PC-T-INP-006` | latest-held quick target wins; release restores older held target | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-INP-007` | hidden continuity never chooses route/lane/gap or brakes/throttles | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-INP-008` | repeated gaze/interaction/focus-loss/recovery cycles leave no stuck state | SPECIFIED / NOT IMPLEMENTED |

## P2 · Taxi / FareSession / passenger exchange — `CD-749`

| Test ID | Required proof | State |
|---|---|---|
| `PC-T-PAX-VIS-001` | passenger/group exists logically/physically before eligible boarding; icon-only completion invalid | LOCKED DIRECTION / NOT IMPLEMENTED |
| `PC-T-STOP-001` | pickup requires deliberate full stop using final epsilon+dwell | EPSILON OWNER OPEN |
| `PC-T-STOP-002` | ordinary drop-off requires deliberate full stop | EPSILON OWNER OPEN |
| `PC-T-DOOR-001` | opposed right-side doors expose continuous no-B-pillar aperture | IDENTITY LOCKED / NOT IMPLEMENTED |
| `PC-T-DOOR-002` | physical cabin lever owns door command state | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-PAX-SEAT-001` | group 1–5 boards once; rear3→front2; mass applies once | LOCKED DIRECTION / NOT IMPLEMENTED |
| `PC-T-MTR-001` | METERED fare accumulates distance + elapsed fare time | BASIS LOCKED / NUMERICS OPEN |
| `PC-T-MTR-002` | OFF_METER never accumulates hidden official-meter fare | LOCKED DIRECTION / NOT IMPLEMENTED |
| `PC-T-PAY-001` | unresolved payment + opened doors can produce one eligible evasion event | LOCKED DIRECTION / RULE NUMERICS OPEN |
| `PC-T-PAY-002` | committed payment prevents ordinary unpaid escape | LOCKED DIRECTION / NOT IMPLEMENTED |
| `PC-T-FARE-001` | fare/passenger/meter/receipt/payment/tip/evasion are save/retry idempotent | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-FARE-002` | one authoritative active FareSession rule follows final owner-pack answer | OWNER PACK ACTIVE |

## P3/P4 · CityCode / road graph / traffic / rules — `CD-751`

| Test ID | Required proof | State |
|---|---|---|
| `PC-T-CITY-SEED` | same CityCode + generator/content version reconstructs same static choices | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-CITY-REV` | reverse travel revisits deterministic prior static identity | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-CITY-030M` | long run keeps chunk/Actor/memory state bounded | BUDGET OPEN `CD-589` |
| `PC-T-ROADGRAPH-001` | one machine-readable road/lane graph can feed generation/routing/traffic/rules | PROPOSED DEFAULT / OWNER PACK ACTIVE |
| `PC-T-L1-NOSIG` | ordinary L1 has no traffic-light/red-light dependency | LOCKED / NOT IMPLEMENTED |
| `PC-T-L1-NOJAM` | no systemic standing-jam state; incidents retain moving bypass | LOCKED / NOT IMPLEMENTED |
| `PC-T-TRF-001` | heavy deterministic traffic preserves playable-gap budget | NUMERICS OPEN `CD-567` |
| `PC-T-TRF-002` | logical/kinematic traffic materializes expensive collision actors only where relevant | ARCHITECTURE PROPOSED / OWNER PACK ACTIVE |
| `PC-T-SIGN-001` | signs resolve to machine-readable RuleId | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-ENF-001` | current rules publish stable versioned EnforcementEvent facts | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-ENF-002` | first-year fine/reputation consumers cannot invent facts outside authoritative event/rule state | SPECIFIED / NOT IMPLEMENTED |

## P5 · Repeat clients / basic Neural — `CD-749`

| Test ID | Required proof | State |
|---|---|---|
| `PC-T-NEU-001` | persistent PassengerIdentity survives stream/save/reload without an Actor | SCOPE LOCKED / NOT IMPLEMENTED |
| `PC-T-NEU-002` | repeat client remembers trip outcomes/preferences/contact permission | SCOPE LOCKED / DETAILS OPEN |
| `PC-T-NEU-003` | repeat order reuses normal Order/Fare pipeline rather than second taxi loop | ARCHITECTURE LOCKED / NOT IMPLEMENTED |
| `PC-T-NEU-004` | simple data-driven messages/history survive save/version migration | SPECIFIED / OWNER NUMERICS OPEN |
| `PC-T-NEU-005` | promotion score/probability follows final deterministic/seeded owner contract | OWNER PACK ACTIVE |

## P6 · Level1 vertical / Level2 transit

Level1 residual magnetism is locked by authoritative total mass:

`m <= 1657 kg → 5.0 s`

`1657 < m < 2107 kg → timeout_s = 5.0 - (m - 1657) / 450`

`m = 2107 kg → 4.0 s`

| Test ID | Required proof | State |
|---|---|---|
| `L1-WALL` | seven-band wallride has stable/recoverable/failure states with no stuck attachment | FORCE/CONTACT CALIBRATION OPEN |
| `L1-WALL-LOAD` | heavier matched fixture modestly reduces abrupt-separation tendency while body penalty remains | DIRECTION LOCKED / CALIBRATION |
| `L1-MAG-REF` | 1657 kg effective residual endpoint = 5.0 s | LOCKED / NOT IMPLEMENTED |
| `L1-MAG-MAX` | 2107 kg effective residual endpoint = 4.0 s | LOCKED / NOT IMPLEMENTED |
| `L1-MAG-MID` | intermediate masses use exact linear formula; 1882 kg = 4.5 s | LOCKED / NOT IMPLEMENTED |
| `L1-MAG-CAP` | legal mass below 1657 kg does not exceed 5.0 s | LOCKED / NOT IMPLEMENTED |
| `L1-FRT-5` | exactly five upper freight lanes | LOCKED / NOT IMPLEMENTED |
| `L1-FRT-DIR` | freight moves opposite lower road | LOCKED / NOT IMPLEMENTED |
| `PC-T-T2-BUS` | Level2 uses final versioned bus-layer profile; lane count not silently assumed | OWNER/CALIBRATION ACTIVE `CD-705` |
| `PC-T-T2-METRO` | two metro tracks per side and versioned timing/identity | PARTIAL / NOT IMPLEMENTED |
| `PC-T-T2-STATION` | station gameplay function follows final owner-pack contract | OWNER PACK ACTIVE |
| `PC-T-MAG-FAIL` | vertical abort/loss/restart returns clean vehicle/world state | SPECIFIED / NOT IMPLEMENTED |

Old fixed-five-second and `interpolation OPEN` rules are SUPERSEDED.

## P7 · Moving refueling — `CD-752/CD-540/CD-593`

| Test ID | Required proof | State |
|---|---|---|
| `PC-T-FUEL-001` | join/advance/leave/abort bounded moving queue | NUMERICS OWNER OPEN |
| `PC-T-FUEL-002` | connection/session starts and cleans deterministically | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-FUEL-003` | distance/speed/lateral eligibility follows final versioned service profile | OWNER OPEN `CD-593` |
| `PC-T-FUEL-004` | approved road-grade/relative-height envelope remains valid | OWNER OPEN |
| `PC-T-FUEL-005` | physical payment endpoint causes exactly one EconomyService settlement | ARCHITECTURE LOCKED / DETAILS OPEN |
| `PC-T-FUEL-006` | same quantity follows locked first-year L1/L2 price relation and profile values | L1<L2 DIRECTION / EXACT PRICES OPEN |
| `PC-T-FUEL-007` | abort/collision/restart/chunk recycle leaves no session/worker/hose/payment/input leak | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-FUEL-MASS` | transferred fuel changes physical Tatra mass exactly once | LOCKED / NOT IMPLEMENTED |
| `PC-T-FUEL-IDEMP` | crash/save/retry cannot gain fuel without required settlement or charge twice | LOCKED ARCHITECTURE / NOT IMPLEMENTED |

Level3 fuel service/pricing is POST-FIRST-EURO and does not block these rows.

## P8 · Economy / workday / save / recovery — `CD-750`

| Test ID | Required proof | State |
|---|---|---|
| `PC-T-ECO-001` | one EconomyService owns balance; features use typed transactions | ARCHITECTURE LOCKED / NOT IMPLEMENTED |
| `PC-T-ECO-002` | fare/tip/fine/fuel/parking/parts/repair transactions are exactly-once | ARCHITECTURE LOCKED / NOT IMPLEMENTED |
| `PC-T-ECO-003` | insufficient-funds/debt/anti-softlock behavior follows owner-locked state machine | OPEN `CD-601` |
| `PC-T-SAVE-001` | save persists logical owners/schema versions rather than uncontrolled transient Actors | SPECIFIED / NOT IMPLEMENTED |
| `PC-T-SAVE-002` | old/unknown/newer schema behavior is explicit | SPECIFIED / DETAILS OPEN |
| `PC-T-SAVE-003` | quit-during-fare and unsafe quit follow final owner contract without duplication | OWNER PACK ACTIVE |
| `PC-T-REC-001` | terminal Vehicle Health classifier produces one common recovery event and clears transient input | ARCHITECTURE LOCKED / NOT IMPLEMENTED |
| `PC-T-REC-002` | FIRST EURO terminal rollback policy follows final owner confirmation under `CD-598/CD-750` | OWNER PACK ACTIVE |
| `PC-T-RC-120M` | full two-real-hour workday/shift proof uses final timer semantics and remains bounded | TIMER SEMANTICS OWNER PACK / NOT IMPLEMENTED |

**There are no FIRST EURO insurance test rows.** `CD-741..745` and `docs/qa/PINK_CAB_INSURANCE_ACCEPTANCE.md` are POST-FIRST-EURO only. Year one verifies only the generic `RecoveryPolicy/RecoveryHook` extension boundary.

## P11 · Automotive ServiceNodes — `CD-752/CD-576/CD-711/CD-713`

| Test ID | Required proof | State |
|---|---|---|
| `SN-PARK-001` | Parking enter/secure/exit returns same Tatra + CityCode | SPECIFIED / DETAILS OPEN |
| `SN-HANG-001` | Practice Hangar round trip where L1 includes it | SPECIFIED / NOT IMPLEMENTED |
| `SN-PART-001` | parts purchase charges/awards exactly once | SPECIFIED / NOT IMPLEMENTED |
| `SN-PART-002` | retry/refund cannot duplicate money/items | SPECIFIED / NOT IMPLEMENTED |
| `SN-PART-003` | incompatible part rejected deterministically | SPECIFIED / DETAILS OPEN |
| `SN-GAR-001` | install/remove/replace updates versioned VehicleBuild through adapter | SPECIFIED / NOT IMPLEMENTED |
| `SN-GAR-002` | VehicleBuild/node migration reconstructs or fails explicitly | SPECIFIED / DETAILS OPEN |
| `SN-REP-001` | Repair consumes Vehicle Health and EconomyService without parallel health/money state | SPECIFIED / NOT IMPLEMENTED |
| `SN-OWN-001` | no enter/exit/retry path creates a second owned Tatra | LOCKED / NOT IMPLEMENTED |
| `SN-OWN-002` | no passenger/fare/money/inventory/VehicleBuild/VehicleHealth duplication or loss | LOCKED / NOT IMPLEMENTED |

Mall/food/bar/club/social/common-lobby QA is POST-FIRST-EURO and does not block this section.

## Performance / bounded-runtime acceptance — `CD-747`

Every integrated family must additionally prove:

- no unbounded Actor/Component/entity/queue/pool growth;
- no global actor scan or synchronous asset load in normal hot path;
- traffic/passenger/incidents use bounded logical populations/materialization;
- chunk recycle leaves no stale ownership/contact/service state;
- exact worst-case fixtures and budgets are versioned once locked;
- presentation-only cost may be measured independently without entering code-readiness scoring unless it violates runtime budget.

## POST-FIRST-EURO QA registry — non-blocking for FIRST EURO

Preserved future test families:

- daily insurance `CD-741..745` / `docs/qa/PINK_CAB_INSURANCE_ACCEPTANCE.md`;
- Taxi Regulator `CD-723` consuming the same `EnforcementEvent` contract;
- Level3 gameplay, including future suspended identity and service/pricing;
- social/lifestyle ServiceNodes;
- common lobby `CD-712`;
- coop/multiplayer/network vehicle/race/moderation gates.

These may have specs/legacy QA files, but they are not year-one pass criteria.

## Current primary FIRST EURO owner locks

- exact runtime/toolchain/plugin/package matrix — `CD-557/CD-559/CD-596/CD-731`;
- save/autosave/quit/recovery details — `CD-560/CD-598/CD-602/CD-750`;
- debt/negative balance/anti-softlock — `CD-601`;
- remaining fare/meter/stop/door/evasion edges — `CD-672/CD-749`;
- Passenger/Neural promotion and compact relationship rules — `CD-570/CD-571/CD-749`;
- CityCode/chunk/traffic/rule numerics — `CD-565/CD-567/CD-589/CD-591/CD-751`;
- Level1 wallride force/contact/reacquisition and other runtime numerics — `CD-592`;
- moving-refuel queue/tolerance/transfer/settlement values — `CD-593/CD-752`;
- Level2 bus/station/timing values — `CD-705..710/CD-748`;
- ServiceNode transition/parking/inventory specifics — `CD-711/CD-752`;
- settings/accessibility/storefront details — `CD-594/CD-747`.

`CD-600` damage breadth is resolved. The Tatra 50-question handling pack is retired to calibration. Insurance, full regulator, Level3 gameplay and online are not FIRST EURO blockers.

## Truth rule

Jira/Confluence/Git can establish CANON/SPECIFIED. Until an exact runnable build executes an applicable row with stored evidence, the row remains **NOT VERIFIED**.
