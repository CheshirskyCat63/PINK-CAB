# PINK CAB · FIRST EURO Verification Matrix

**Status:** CURRENT ROW-BY-ROW SOURCE / PROOF-OWNER INDEX; FULL PRODUCT RUNTIME ACCEPTANCE OPEN
**BASE-100:** `CD-746..753`; Confluence `11239425`
**Readiness:** `CD-660/CD-661`
**Open decisions:** `CD-588/CD-673`, Confluence `5832744`
**FIRST EURO scope:** `docs/FIRST_EURO_SCOPE.md`


## Reconciliation checkpoint — 2026-10-03 / CD-858

All 97 FIRST EURO rows were mapped against source tree `ec3344133ef7f17d9bb3bb12bb53a4d1e6f1c4e2`. The source registry below resolves concrete production headers from each fixture's includes and names the actual fixture files. This is a source/coverage audit, not a new full-suite run. IMPLEMENTED FOUNDATION means the cited logical primitive and fixture exist; it does not claim that the complete physical/presentation/performance acceptance row passes. SPECIFIED/PARTIAL rows explicitly retain their missing scope. No row is promoted to current-head VERIFIED from Jira status, test declarations or an aggregate count.

Historical evidence is retained separately: previous matrix rows cited CD-825 12/12 at `e3241dc92a4d89d7a9abd22c2e3920a1ff91a25b` and the package receipt on UE 5.8.2. Those receipts do not prove the current optional-RMB/key-held prompt/no-assist contract. Accepted P02 runtime is `8d68e456d1944be295281535cf9fd103ecf05d52`, exact-head run [36868646970](https://github.com/CheshirskyCat63/PINK-CAB/actions/runs/36868646970), integrated by PR #49. That historical P03 rejection/P04 block was superseded by later accepted P03/P04 and the accepted P4/RIG06 fallback 6edea774. Current Tasks 0-4 are integrated through PR70/main c8459ba; Task5 is RED and Tasks6-9 remain open. The current nine-test focused suite is not whole-product proof; no matrix row is promoted by this status reconciliation. P02 stage evidence remains in `docs/vehicle_physics/`; it is not whole-FIRST-EURO acceptance.

Canonical integration is live protected `main`, not an embedded old SHA. Current development setup: `docs/PROJECT_SETUP.md`; control contract: Confluence 16744449. CD-856 is DONE; CD-855 owns presentation and donor rights/exposure remediation. CD-848 remains the broader mechanics gate; CD-559 owns complete current-candidate regression/reproducibility. Each row below also names its domain proof owner. CD-663's hidden-steering proposal is archived/superseded and must not be implemented.

## 1. Maturity / denominator

`CANON → SPECIFIED → IMPLEMENTED → VERIFIED`

This matrix follows the code-only BASE-100 denominator. Visual/art/presentation-only tests may remain useful production evidence but do **not** lower START-90/BASE-100 unless they create a code-facing state, interaction, dependency or performance contract.

FIRST EURO = first 12 months / PC / single-player / full Level1 + Level2. Post-year systems are listed separately and do not block first-year verification beyond their explicit extension boundaries.

## 2. Universal evidence record

Every executable result records where relevant:

- requirement/test ID;
- exact build/commit;
- UE version;
- exact native Chaos provider/profile/build identity where vehicle code participates;
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

| Test ID | Required proof | State | Source / proof owner |
|---|---|---|---|
| `PC-T-P0-001` | clean checkout builds/packages with the exact locked UE and declared dependency set | HISTORICAL package evidence `e3241dc92a4d89d7a9abd22c2e3920a1ff91a25b` / UE 5.8.2; current clean package NOT VERIFIED | [foundation](#source-foundation); `CD-559 / CD-596 / CD-560` |
| `PC-T-P0-002` | executable reports build/commit/content/schema/Chaos-provider/profile identity | PARTIAL foundation/identity fixtures; complete executable evidence NOT VERIFIED | [foundation](#source-foundation); `CD-559 / CD-596 / CD-560` |
| `PC-T-P0-003` | deterministic CityCode/Traffic/Passenger fixtures reproduce | PARTIAL foundation/identity fixtures; complete executable evidence NOT VERIFIED | [foundation](#source-foundation); `CD-559 / CD-596 / CD-560` |
| `PC-T-P0-004` | logs/crashes identify exact build and version context | PARTIAL foundation/identity fixtures; complete executable evidence NOT VERIFIED | [foundation](#source-foundation); `CD-559 / CD-596 / CD-560` |
| `PC-T-P0-005` | older/incompatible/newer persistent schema migrates or fails explicitly | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [persistence](#source-persistence); `CD-560 / CD-602 / CD-559` |
| `PC-T-P0-006` | vendor plugins are isolated behind declared PINK CAB adapters | IMPLEMENTED adapter and pinned MetaRoad preparation; complete dependency-boundary proof NOT VERIFIED | [chaos](#source-chaos); `CD-648 / CD-921` |
| `PC-T-P0-007` | hot-path audit finds no unbounded Tick/world scan/sync load or uncontrolled collection growth | SPECIFIED whole-project performance gate; bounded component fixtures exist; NOT VERIFIED | [streaming](#source-streaming); `CD-869 / CD-703 / CD-589` |

## P1 · Hero Tatra / native Chaos / input — `CD-748`

Mass fixtures remain 1450 kg base, 1550 kg full fuel, crew 58 + 49 kg, reference 1657 kg and declared max 2107 kg. Native Chaos profile in current source owns engine numerics (approximately 250 hp / 260 Nm, 8500 RPM operating envelope, 925 RPM warm idle); the historical 180 hp / 240 Nm blanket statement is superseded. P4 is owner accepted; the current `Vehicle Feel 90` lane performs post-P4 measurable calibration toward the intended FIRST EURO driving envelope without reopening that acceptance. No ABS/ESP or hidden driving assistance.

| Test ID | Required proof | State | Source / proof owner |
|---|---|---|---|
| `PC-T-STACK-001` | native Unreal Chaos is sole production hero-Tatra road-dynamics solver | IMPLEMENTED Native Chaos foundation; accepted P02 history below; current full-row proof NOT VERIFIED | [chaos](#source-chaos); `CD-648 / CD-921` |
| `PC-T-STACK-002` | production path has no required FGear/VDS dependency; damage/destruction uses bounded native authored/Chaos architecture | IMPLEMENTED Native Chaos foundation; accepted P02 history below; current full-row proof NOT VERIFIED | [chaos](#source-chaos); `CD-648 / CD-921` |
| `PC-T-VEH-001` | Tatra remains controllable/readable across declared speed/load envelope | SPECIFIED handling acceptance; P03 rejected / later calibration open; NOT VERIFIED | [chaos](#source-chaos); `CD-648 / CD-921` |
| `PC-T-VEH-002` | versioned Chaos physical profile and provider expose reproducible handling targets and normalized telemetry | IMPLEMENTED Native Chaos foundation; accepted P02 history below; current full-row proof NOT VERIFIED | [chaos](#source-chaos); `CD-648 / CD-921` |
| `PC-T-WET-001` | wet >~160 rapid lane change + excess throttle can progressively saturate rear; easing throttle restores reserve without ESP/autocountersteer | SPECIFIED handling acceptance; P03 rejected / later calibration open; NOT VERIFIED | [chaos](#source-chaos); `CD-650 / CD-873 / CD-921` |
| `PC-T-MASS-001` | total mass = base + current fuel + crew + exact boarded passenger masses | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [mass](#source-mass); `CD-652 / CD-921` |
| `PC-T-MASS-002` | passenger/fuel mass persists and applies exactly once | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [mass](#source-mass); `CD-652 / CD-921` |
| `PC-T-MASS-003` | 1657 kg and 2107 kg fixtures reconstruct exactly | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [mass](#source-mass); `CD-652 / CD-921` |
| `PC-T-ELEC-001` | no ABS system or authoritative intervention | LOCKED no-assist contract; provider fixtures exist; whole-path intervention absence NOT VERIFIED | [chaos](#source-chaos); `CD-648 / CD-921` |
| `PC-T-ELEC-002` | no ESP individual-wheel intervention; deliberate spin remains possible | LOCKED no-assist contract; provider fixtures exist; whole-path intervention absence NOT VERIFIED | [chaos](#source-chaos); `CD-648 / CD-921` |
| `PC-T-EXPR-001` | Tatra Expression/presentation may amplify presentation but cannot change Chaos force/trajectory | IMPLEMENTED provider/presentation boundary; complete force-authority proof NOT VERIFIED | [chaos](#source-chaos); `CD-648 / CD-921` |
| `PC-T-DMG-001` | authored/native damage presentation maps severity to bounded persistent damage states | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [health](#source-health); `CD-740 / CD-722 / CD-921` |
| `PC-T-DMG-002` | only authored hit-zone mappings create Vehicle Health consequences | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [health](#source-health); `CD-740 / CD-722 / CD-921` |
| `PC-T-DMG-003` | air-cooled oil/head/fan/oil-cooler/airflow thermal model; no generic coolant/radiator system | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [health](#source-health); `CD-740 / CD-722 / CD-921` |
| `PC-T-INP-001` | hold Space transfers mouse steering→gaze/target-search→steering cleanly | IMPLEMENTED current input foundation; historical CD-825 evidence below; current full-row proof NOT VERIFIED | [input](#source-input); `CD-649 / CD-670 / CD-921` |
| `PC-T-INP-002` | gaze/target selection and quick recall never actuate the target | IMPLEMENTED current input foundation; historical CD-825 evidence below; current full-row proof NOT VERIFIED | [input](#source-input); `CD-649 / CD-670 / CD-921` |
| `PC-T-INP-003` | RMB optionally captures/retains a valid physical target; contextual LMB/wheel does not require RMB first | IMPLEMENTED current input foundation; historical CD-825 evidence below; current full-row proof NOT VERIFIED | [input](#source-input); `CD-649 / CD-670 / CD-921` |
| `PC-T-INP-004` | LMB tap/hold produces momentary press semantics only on controls that declare it; horn proves short/long hold distinction | IMPLEMENTED current input foundation; historical CD-825 evidence below; current full-row proof NOT VERIFIED | [input](#source-input); `CD-649 / CD-670 / CD-921` |
| `PC-T-INP-005` | mouse wheel produces signed detent/rotary/incremental input only on controls that declare wheel adjustment | IMPLEMENTED current input foundation; historical CD-825 evidence below; current full-row proof NOT VERIFIED | [input](#source-input); `CD-649 / CD-670 / CD-921` |
| `PC-T-INP-006` | 1/2/3/4 provides key-held quick-target access only; release hides the prompt and recall never actuates a control | IMPLEMENTED current input foundation; historical CD-825 evidence below; current full-row proof NOT VERIFIED | [input](#source-input); `CD-649 / CD-670 / CD-921` |
| `PC-T-INP-007` | one bounded interaction target; no global scan, hidden steering correction, lane keeping, yaw rescue, brake or throttle assist | IMPLEMENTED current input foundation; historical CD-825 evidence below; current full-row proof NOT VERIFIED | [input](#source-input); `CD-649 / CD-670 / CD-921` |
| `PC-T-INP-008` | repeated gaze/target/grip/press/wheel/focus-loss/recovery cycles leave no stuck hand or input ownership state | IMPLEMENTED current input foundation; historical CD-825 evidence below; current full-row proof NOT VERIFIED | [input](#source-input); `CD-649 / CD-670 / CD-921` |

## P2 · Taxi / FareSession / passenger exchange — `CD-749`

| Test ID | Required proof | State | Source / proof owner |
|---|---|---|---|
| `PC-T-PAX-VIS-001` | passenger/group exists logically/physically before eligible boarding; icon-only completion invalid | PARTIAL primitive presentation; final visible boarding/aperture proof NOT VERIFIED | [presentation](#source-presentation); `CD-870 / CD-720 / CD-724` |
| `PC-T-STOP-001` | pickup requires deliberate full stop using final epsilon+dwell | IMPLEMENTED logical fare foundation; final physical interaction/numerics and full-row proof NOT VERIFIED | [fare](#source-fare); `CD-870 / CD-724 / CD-672` |
| `PC-T-STOP-002` | ordinary drop-off requires deliberate full stop | IMPLEMENTED logical fare foundation; final physical interaction/numerics and full-row proof NOT VERIFIED | [fare](#source-fare); `CD-870 / CD-724 / CD-672` |
| `PC-T-DOOR-001` | opposed right-side doors expose continuous no-B-pillar aperture | PARTIAL primitive presentation; final visible boarding/aperture proof NOT VERIFIED | [presentation](#source-presentation); `CD-870 / CD-720 / CD-724` |
| `PC-T-DOOR-002` | physical cabin lever owns door command state | IMPLEMENTED logical fare foundation; final physical interaction/numerics and full-row proof NOT VERIFIED | [fare](#source-fare); `CD-870 / CD-724 / CD-672` |
| `PC-T-PAX-SEAT-001` | group 1–5 boards once; rear3→front2; mass applies once | IMPLEMENTED logical fare foundation; final physical interaction/numerics and full-row proof NOT VERIFIED | [fare](#source-fare); `CD-870 / CD-724 / CD-672` |
| `PC-T-MTR-001` | METERED fare accumulates distance + elapsed fare time | IMPLEMENTED logical fare foundation; final physical interaction/numerics and full-row proof NOT VERIFIED | [fare](#source-fare); `CD-870 / CD-724 / CD-672` |
| `PC-T-MTR-002` | OFF_METER never accumulates hidden official-meter fare | IMPLEMENTED logical fare foundation; final physical interaction/numerics and full-row proof NOT VERIFIED | [fare](#source-fare); `CD-870 / CD-724 / CD-672` |
| `PC-T-PAY-001` | unresolved payment + open exit can produce fare evasion either by stopover no-return or final exit before grab-rail-reader payment commit | IMPLEMENTED logical fare foundation; final physical interaction/numerics and full-row proof NOT VERIFIED | [fare](#source-fare); `CD-870 / CD-724 / CD-672` |
| `PC-T-PAY-002` | passenger hand swipe on grab-rail reader commits payment exactly once; committed payment prevents ordinary unpaid escape | IMPLEMENTED logical fare foundation; final physical interaction/numerics and full-row proof NOT VERIFIED | [fare](#source-fare); `CD-870 / CD-724 / CD-672` |
| `PC-T-FARE-001` | fare/passenger/meter/optional receipt/payment/tip/off-register outcome/evasion are save/retry idempotent | IMPLEMENTED logical fare foundation; final physical interaction/numerics and full-row proof NOT VERIFIED | [fare](#source-fare); `CD-870 / CD-724 / CD-672` |
| `PC-T-FARE-002` | one authoritative active FareSession rule follows final owner-pack answer | IMPLEMENTED logical fare foundation; final physical interaction/numerics and full-row proof NOT VERIFIED | [fare](#source-fare); `CD-870 / CD-724 / CD-672` |

## P3/P4 · CityCode / road graph / traffic / rules — `CD-751`

| Test ID | Required proof | State | Source / proof owner |
|---|---|---|---|
| `PC-T-CITY-SEED` | same CityCode + generator/content version reconstructs same static choices | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [city](#source-city); `CD-869 / CD-565` |
| `PC-T-CITY-REV` | reverse travel revisits deterministic prior static identity | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [city](#source-city); `CD-869 / CD-565` |
| `PC-T-CITY-030M` | long run keeps chunk/Actor/memory state bounded | IMPLEMENTED bounded fixtures; timed integrated 30-minute route/performance proof NOT VERIFIED | [streaming](#source-streaming); `CD-869 / CD-703 / CD-589` |
| `PC-T-ROADGRAPH-001` | one machine-readable road/lane graph can feed generation/routing/traffic/rules | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [city](#source-city); `CD-869 / CD-565` |
| `PC-T-L1-NOSIG` | ordinary L1 has no traffic-light/red-light dependency | IMPLEMENTED traffic foundation; final population/physical world acceptance NOT VERIFIED | [traffic](#source-traffic); `CD-874 / CD-567 / CD-702` |
| `PC-T-L1-NOJAM` | no systemic standing-jam state; incidents retain moving bypass | IMPLEMENTED traffic foundation; final population/physical world acceptance NOT VERIFIED | [traffic](#source-traffic); `CD-874 / CD-567 / CD-702` |
| `PC-T-TRF-001` | heavy deterministic traffic preserves playable-gap budget | IMPLEMENTED traffic foundation; final population/physical world acceptance NOT VERIFIED | [traffic](#source-traffic); `CD-874 / CD-567 / CD-702` |
| `PC-T-TRF-002` | logical/kinematic traffic materializes expensive collision actors only where relevant | IMPLEMENTED traffic foundation; final population/physical world acceptance NOT VERIFIED | [traffic](#source-traffic); `CD-874 / CD-567 / CD-702` |
| `PC-T-SIGN-001` | signs resolve to machine-readable RuleId | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [enforcement](#source-enforcement); `CD-878 / CD-591 / CD-569` |
| `PC-T-ENF-001` | current rules publish stable versioned EnforcementEvent facts | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [enforcement](#source-enforcement); `CD-878 / CD-591 / CD-569` |
| `PC-T-ENF-002` | first-year fine/reputation consumers cannot invent facts outside authoritative event/rule state | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [enforcement](#source-enforcement); `CD-878 / CD-591 / CD-569` |

## P5 · Repeat clients / basic Neural — `CD-749`

| Test ID | Required proof | State | Source / proof owner |
|---|---|---|---|
| `PC-T-NEU-001` | persistent PassengerIdentity survives stream/save/reload without an Actor | IMPLEMENTED logical registry/history foundation; current integrated row and final policy NOT VERIFIED | [neural](#source-neural); `CD-875 / CD-877 / CD-571` |
| `PC-T-NEU-002` | PassengerIdentity persists from first encounter; repeat-pool promotion after 2 paid fares or one authored relationship event; Trust/Satisfaction/RiskTolerance persist | IMPLEMENTED logical registry/history foundation; current integrated row and final policy NOT VERIFIED | [neural](#source-neural); `CD-875 / CD-877 / CD-571` |
| `PC-T-NEU-003` | repeat order reuses normal Order/Fare pipeline rather than second taxi loop | IMPLEMENTED logical registry/history foundation; current integrated row and final policy NOT VERIFIED | [neural](#source-neural); `CD-875 / CD-877 / CD-571` |
| `PC-T-NEU-004` | simple data-driven messages/history survive save/version migration | IMPLEMENTED logical registry/history foundation; current integrated row and final policy NOT VERIFIED | [neural](#source-neural); `CD-875 / CD-877 / CD-571` |
| `PC-T-NEU-005` | promotion score/probability follows final deterministic/seeded owner contract | IMPLEMENTED logical registry/history foundation; current integrated row and final policy NOT VERIFIED | [neural](#source-neural); `CD-875 / CD-877 / CD-571` |

## P6 · Level1 vertical / Level2 transit

Level1 residual magnetism is locked by authoritative total mass:

`m <= 1657 kg → 5.0 s`

`1657 < m < 2107 kg → timeout_s = 5.0 - (m - 1657) / 450`

`m = 2107 kg → 4.0 s`

| Test ID | Required proof | State | Source / proof owner |
|---|---|---|---|
| `L1-WALL` | seven-band wallride has stable/recoverable/failure states with no stuck attachment | PARTIAL state/contact foundation; calibrated physical seven-band behavior NOT VERIFIED | [wallride](#source-wallride); `CD-592 / CD-701 / CD-869` |
| `L1-WALL-LOAD` | heavier matched fixture modestly reduces abrupt-separation tendency while body penalty remains | PARTIAL state/contact foundation; calibrated physical seven-band behavior NOT VERIFIED | [wallride](#source-wallride); `CD-592 / CD-701 / CD-869` |
| `L1-MAG-REF` | 1657 kg effective residual endpoint = 5.0 s | IMPLEMENTED exact mass-time formula and fixtures; current runtime NOT VERIFIED | [wallride](#source-wallride); `CD-592 / CD-701 / CD-869` |
| `L1-MAG-MAX` | 2107 kg effective residual endpoint = 4.0 s | IMPLEMENTED exact mass-time formula and fixtures; current runtime NOT VERIFIED | [wallride](#source-wallride); `CD-592 / CD-701 / CD-869` |
| `L1-MAG-MID` | intermediate masses use exact linear formula; 1882 kg = 4.5 s | IMPLEMENTED exact mass-time formula and fixtures; current runtime NOT VERIFIED | [wallride](#source-wallride); `CD-592 / CD-701 / CD-869` |
| `L1-MAG-CAP` | legal mass below 1657 kg does not exceed 5.0 s | IMPLEMENTED exact mass-time formula and fixtures; current runtime NOT VERIFIED | [wallride](#source-wallride); `CD-592 / CD-701 / CD-869` |
| `L1-FRT-5` | exactly five upper freight lanes | SPECIFIED complete freight lane/direction acceptance; traversal hooks only; NOT VERIFIED | [freight](#source-freight); `CD-688 / CD-869` |
| `L1-FRT-DIR` | freight moves opposite lower road | SPECIFIED complete freight lane/direction acceptance; traversal hooks only; NOT VERIFIED | [freight](#source-freight); `CD-688 / CD-869` |
| `PC-T-T2-BUS` | Level2 uses final versioned bus-layer profile; lane count not silently assumed | PARTIAL logical transit/abort foundation; representative physical route NOT VERIFIED | [transit](#source-transit); `CD-705 / CD-710 / CD-869` |
| `PC-T-T2-METRO` | two metro tracks per side and versioned timing/identity | PARTIAL logical transit/abort foundation; representative physical route NOT VERIFIED | [transit](#source-transit); `CD-705 / CD-710 / CD-869` |
| `PC-T-T2-STATION` | station gameplay function follows final owner-pack contract | PARTIAL logical transit/abort foundation; representative physical route NOT VERIFIED | [transit](#source-transit); `CD-705 / CD-710 / CD-869` |
| `PC-T-MAG-FAIL` | vertical abort/loss/restart returns clean vehicle/world state | PARTIAL logical transit/abort foundation; representative physical route NOT VERIFIED | [transit](#source-transit); `CD-705 / CD-710 / CD-869` |

Old fixed-five-second and `interpolation OPEN` rules are SUPERSEDED.

## P7 · Moving refueling — `CD-752/CD-540/CD-593`

| Test ID | Required proof | State | Source / proof owner |
|---|---|---|---|
| `PC-T-FUEL-001` | join/advance/leave/abort bounded moving queue | IMPLEMENTED fuel/session/settlement foundation; final physical envelope/pricing and full row NOT VERIFIED | [fuel](#source-fuel); `CD-872 / CD-593` |
| `PC-T-FUEL-002` | connection/session starts and cleans deterministically | IMPLEMENTED fuel/session/settlement foundation; final physical envelope/pricing and full row NOT VERIFIED | [fuel](#source-fuel); `CD-872 / CD-593` |
| `PC-T-FUEL-003` | distance/speed/lateral eligibility follows final versioned service profile | IMPLEMENTED fuel/session/settlement foundation; final physical envelope/pricing and full row NOT VERIFIED | [fuel](#source-fuel); `CD-872 / CD-593` |
| `PC-T-FUEL-004` | approved road-grade/relative-height envelope remains valid | IMPLEMENTED fuel/session/settlement foundation; final physical envelope/pricing and full row NOT VERIFIED | [fuel](#source-fuel); `CD-872 / CD-593` |
| `PC-T-FUEL-005` | physical payment endpoint causes exactly one EconomyService settlement | IMPLEMENTED fuel/session/settlement foundation; final physical envelope/pricing and full row NOT VERIFIED | [fuel](#source-fuel); `CD-872 / CD-593` |
| `PC-T-FUEL-006` | same quantity follows locked first-year L1/L2 price relation and profile values | IMPLEMENTED fuel/session/settlement foundation; final physical envelope/pricing and full row NOT VERIFIED | [fuel](#source-fuel); `CD-872 / CD-593` |
| `PC-T-FUEL-007` | abort/collision/restart/chunk recycle leaves no session/worker/hose/payment/input leak | IMPLEMENTED fuel/session/settlement foundation; final physical envelope/pricing and full row NOT VERIFIED | [fuel](#source-fuel); `CD-872 / CD-593` |
| `PC-T-FUEL-MASS` | transferred fuel changes physical Tatra mass exactly once | IMPLEMENTED fuel/session/settlement foundation; final physical envelope/pricing and full row NOT VERIFIED | [fuel](#source-fuel); `CD-872 / CD-593` |
| `PC-T-FUEL-IDEMP` | crash/save/retry cannot gain fuel without required settlement or charge twice | IMPLEMENTED fuel/session/settlement foundation; final physical envelope/pricing and full row NOT VERIFIED | [fuel](#source-fuel); `CD-872 / CD-593` |

Level3 fuel service/pricing is POST-FIRST-EURO and does not block these rows.

## P8 · Economy / workday / save / recovery — `CD-750`

| Test ID | Required proof | State | Source / proof owner |
|---|---|---|---|
| `PC-T-ECO-001` | one EconomyService owns balance; features use typed transactions | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [economy](#source-economy); `CD-601 / CD-569 / CD-878` |
| `PC-T-ECO-002` | fare/tip/fine/fuel/parking/parts/repair transactions are exactly-once | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [economy](#source-economy); `CD-601 / CD-569 / CD-878` |
| `PC-T-ECO-003` | insufficient-funds/debt/anti-softlock behavior follows owner-locked state machine | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [economy](#source-economy); `CD-601 / CD-569 / CD-878` |
| `PC-T-SAVE-001` | save persists logical owners/schema versions rather than uncontrolled transient Actors | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [persistence](#source-persistence); `CD-560 / CD-602 / CD-559` |
| `PC-T-SAVE-002` | old/unknown/newer schema behavior is explicit | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [persistence](#source-persistence); `CD-560 / CD-602 / CD-559` |
| `PC-T-SAVE-003` | quit-during-fare and unsafe quit follow final owner contract without duplication | IMPLEMENTED FOUNDATION; full row NOT VERIFIED | [persistence](#source-persistence); `CD-560 / CD-602 / CD-559` |
| `PC-T-REC-001` | terminal Vehicle Health classifier produces one common recovery event and clears transient input | IMPLEMENTED logical recovery foundation; complete packaged policy proof NOT VERIFIED | [recovery](#source-recovery); `CD-598 / CD-602 / CD-574` |
| `PC-T-REC-002` | FIRST EURO terminal rollback policy follows final owner confirmation under `CD-598/CD-750` | IMPLEMENTED logical recovery foundation; complete packaged policy proof NOT VERIFIED | [recovery](#source-recovery); `CD-598 / CD-602 / CD-574` |
| `PC-T-RC-120M` | full two-real-hour workday/shift proof uses final timer semantics and remains bounded | SPECIFIED real two-hour integrated workday/soak; component fixtures are not duration proof; NOT VERIFIED | [recovery](#source-recovery); `CD-598 / CD-602 / CD-574` |

**There are no FIRST EURO insurance test rows.** `CD-741..745` and `docs/qa/PINK_CAB_INSURANCE_ACCEPTANCE.md` are POST-FIRST-EURO only. Year one verifies only the generic `RecoveryPolicy/RecoveryHook` extension boundary.

## P11 · Automotive ServiceNodes — `CD-752/CD-576/CD-711/CD-713`

| Test ID | Required proof | State | Source / proof owner |
|---|---|---|---|
| `SN-PARK-001` | Parking enter/secure/exit returns same Tatra + CityCode | IMPLEMENTED service/inventory/replay foundation; complete physical ServiceNode row NOT VERIFIED | [service](#source-service); `CD-871 / CD-711 / CD-713` |
| `SN-HANG-001` | Practice Hangar round trip where L1 includes it | IMPLEMENTED service/inventory/replay foundation; complete physical ServiceNode row NOT VERIFIED | [service](#source-service); `CD-871 / CD-711 / CD-713` |
| `SN-PART-001` | parts purchase charges/awards exactly once | IMPLEMENTED service/inventory/replay foundation; complete physical ServiceNode row NOT VERIFIED | [service](#source-service); `CD-871 / CD-711 / CD-713` |
| `SN-PART-002` | retry/refund cannot duplicate money/items | IMPLEMENTED service/inventory/replay foundation; complete physical ServiceNode row NOT VERIFIED | [service](#source-service); `CD-871 / CD-711 / CD-713` |
| `SN-PART-003` | incompatible part rejected deterministically | IMPLEMENTED service/inventory/replay foundation; complete physical ServiceNode row NOT VERIFIED | [service](#source-service); `CD-871 / CD-711 / CD-713` |
| `SN-GAR-001` | install/remove/replace updates versioned VehicleBuild through adapter | IMPLEMENTED service/inventory/replay foundation; complete physical ServiceNode row NOT VERIFIED | [service](#source-service); `CD-871 / CD-711 / CD-713` |
| `SN-GAR-002` | VehicleBuild/node migration reconstructs or fails explicitly | IMPLEMENTED service/inventory/replay foundation; complete physical ServiceNode row NOT VERIFIED | [service](#source-service); `CD-871 / CD-711 / CD-713` |
| `SN-REP-001` | Repair consumes Vehicle Health and EconomyService without parallel health/money state | IMPLEMENTED service/inventory/replay foundation; complete physical ServiceNode row NOT VERIFIED | [service](#source-service); `CD-871 / CD-711 / CD-713` |
| `SN-OWN-001` | no enter/exit/retry path creates a second owned Tatra | IMPLEMENTED service/inventory/replay foundation; complete physical ServiceNode row NOT VERIFIED | [service](#source-service); `CD-871 / CD-711 / CD-713` |
| `SN-OWN-002` | no passenger/fare/money/inventory/VehicleBuild/VehicleHealth duplication or loss | IMPLEMENTED service/inventory/replay foundation; complete physical ServiceNode row NOT VERIFIED | [service](#source-service); `CD-871 / CD-711 / CD-713` |

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

- exact runtime/toolchain/dependency/package matrix — `CD-785..CD-792`, `CD-802`, `CD-823..829`;
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

## Source and fixture registry

Paths below are relative to the repository and were checked at the source checkpoint above. Production headers name the owning API; related implementations live in the corresponding module. Fixture presence is not a PASS receipt.

### Source foundation

Proof owner: `CD-559 / CD-596 / CD-560`.

Production interfaces: `Source/PinkCab/Public/Core/PinkCabBuildIdentity.h`, `Source/PinkCabCore/Public/Core/PinkCabDeterministicSeed.h`, `Source/PinkCabCore/Public/Core/PinkCabEventEnvelope.h`, `Source/PinkCabCore/Public/Core/PinkCabExactlyOnceStore.h`, `Source/PinkCabCore/Public/Core/PinkCabStateService.h`, `Source/PinkCabCore/Public/Core/PinkCabStateKernel.h`, `Source/PinkCabEconomy/Public/Economy/PinkCabTransaction.h`, `Source/PinkCabCore/Public/Core/PinkCabFeatureConfig.h`, `Source/PinkCabCore/Public/Core/PinkCabResult.h`, `Source/PinkCabCore/Public/Core/PinkCabSchemaVersion.h`, `Source/PinkCabCore/Public/Core/PinkCabStableId.h`

Fixtures: `Source/PinkCabTests/Private/PinkCabBootstrapTests.cpp`, `Source/PinkCabTests/Private/Core/PinkCabFoundationStateTests.cpp`, `Source/PinkCabTests/Private/Core/PinkCabCoreContractTests.cpp`

### Source persistence

Proof owner: `CD-560 / CD-602 / CD-559`.

Production interfaces: `Source/PinkCabPersistence/Public/Persistence/PinkCabSaveHeader.h`, `Source/PinkCab/Public/Persistence/PinkCabPersistenceService.h`, `Source/PinkCabPersistence/Public/Persistence/PinkCabMigrationRegistry.h`, `Source/PinkCab/Public/Persistence/PinkCabRecoveryOrchestrator.h`, `Source/PinkCab/Public/Persistence/PinkCabGamePersistenceCoordinator.h`

Fixtures: `Source/PinkCabTests/Private/Persistence/PinkCabPersistenceTests.cpp`, `Source/PinkCabTests/Private/Persistence/PinkCabPersistenceIntegratedAcceptanceTests.cpp`, `Source/PinkCabTests/Private/Persistence/PinkCabGameSnapshotTests.cpp`

### Source chaos

Proof owner: `CD-648 / CD-921`.

Production interfaces: `Source/PinkCab/Public/Runtime/PinkCabChaosTatraPawn.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabChaosCockpitBridge.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabCockpitState.h`, `Source/PinkCab/Public/Vehicle/PinkCabChaosWheelFront.h`, `Source/PinkCab/Public/Vehicle/PinkCabChaosWheelRear.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabTatraProfile.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabSteeringController.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabChaosPhysicalProfile.h`, `Source/PinkCab/Public/Runtime/PinkCabVehicleVisualProfile.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabChaosVehicleDynamicsProvider.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabThrottleResponse.h`

Fixtures: `Source/PinkCabTests/Private/Vehicle/PinkCabChaosPawnTests.cpp`, `Source/PinkCabTests/Private/Vehicle/PinkCabChaosVehicleProviderTests.cpp`, `Source/PinkCabTests/Private/Vehicle/PinkCabChaosPhysicalProfileTests.cpp`

### Source mass

Proof owner: `CD-652 / CD-921`.

Production interfaces: `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleControlState.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleTelemetry.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleDynamicsProvider.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabTatraProfile.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleLoadState.h`, `Source/PinkCabPersistence/Public/Persistence/PinkCabVehicleSnapshot.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabChaosLoadBridge.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleHealthService.h`

Fixtures: `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleContractTests.cpp`, `Source/PinkCabTests/Private/Persistence/PinkCabVehiclePersistenceTests.cpp`, `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleLiveStateTests.cpp`

### Source health

Proof owner: `CD-740 / CD-722 / CD-921`.

Production interfaces: `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleHealthState.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleHitEvent.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleHealthService.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabDrivetrainCondition.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleHealthBinding.h`, `Source/PinkCabPersistence/Public/Persistence/PinkCabVehicleSnapshot.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleControlState.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleDamageProfile.h`

Fixtures: `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleHealthTests.cpp`, `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleDamageLockedSpecTests.cpp`, `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleDamageProfileTests.cpp`

### Source input

Proof owner: `CD-649 / CD-670 / CD-921`.

Production interfaces: `Source/PinkCabInteraction/Public/Interaction/PinkCabInteractionModel.h`, `Source/PinkCabInteraction/Public/Interaction/PinkCabSemanticInputRouter.h`, `Source/PinkCabInteraction/Public/Interaction/PinkCabSemanticCommand.h`, `Source/PinkCab/Public/Interaction/PinkCabContractCabinPrimitive.h`, `Source/PinkCabInteraction/Public/Interaction/PinkCabPlayerInputAdapter.h`, `Source/PinkCabInteraction/Public/Interaction/PinkCabWheelInputResponse.h`, `Source/PinkCab/Public/Cockpit/PinkCabCockpitInteractionComponent.h`, `Source/PinkCab/Public/Cockpit/PinkCabCockpitAssemblyComponent.h`, `Source/PinkCab/Public/Runtime/PinkCabChaosTatraPawn.h`, `Source/PinkCab/Public/Vehicle/PinkCabCockpitInteractionRouter.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabCockpitState.h`, `Source/PinkCab/Public/Vehicle/PinkCabVehicleInputFrame.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabSteeringController.h`, `Source/PinkCab/Public/Vehicle/PinkCabVehicleControlRuntime.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabHGateGeometry.h`

Fixtures: `Source/PinkCabTests/Private/Interaction/PinkCabInteractionContractTests.cpp`, `Source/PinkCabTests/Private/Interaction/PinkCabPlayerInputAdapterTests.cpp`, `Source/PinkCabTests/Private/Cockpit/PinkCabCockpitInputTests.cpp`, `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleControlRuntimeTests.cpp`

### Source presentation

Proof owner: `CD-870 / CD-720 / CD-724`.

Production interfaces: `Source/PinkCabTaxi/Public/Taxi/PinkCabFarePassengerManifest.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabFarePassengerPresentation.h`, `Source/PinkCab/Public/Cockpit/PinkCabCockpitSlot.h`, `Source/PinkCab/Public/Cockpit/PinkCabCockpitPresentationState.h`, `Source/PinkCabInteraction/Public/Interaction/PinkCabInteractionModel.h`, `Source/PinkCab/Public/Cockpit/PinkCabCockpitAssemblyComponent.h`

Fixtures: `Source/PinkCabTests/Private/Taxi/PinkCabPassengerPresentationTests.cpp`, `Source/PinkCabTests/Private/Cockpit/PinkCabCockpitContractTests.cpp`

### Source fare

Proof owner: `CD-870 / CD-724 / CD-672`.

Production interfaces: `Source/PinkCabTaxi/Public/Taxi/PinkCabTaximeter.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabFarePassengerManifest.h`, `Source/PinkCabEconomy/Public/Economy/PinkCabEconomyLedger.h`, `Source/PinkCabEconomy/Public/Economy/PinkCabFareSettlementService.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleLoadState.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabTatraProfile.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabFareLoopCoordinator.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabOrder.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabFareSession.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabPassengerTemplate.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabPassengerIdentity.h`, `Source/PinkCabPersistence/Public/Persistence/PinkCabFareRuntimeSnapshot.h`

Fixtures: `Source/PinkCabTests/Private/Taxi/PinkCabFareLoopTests.cpp`, `Source/PinkCabTests/Private/Taxi/PinkCabFareLoopCoordinatorTests.cpp`, `Source/PinkCabTests/Private/Taxi/PinkCabTaxiContractTests.cpp`, `Source/PinkCabTests/Private/Persistence/PinkCabFareRuntimePersistenceTests.cpp`

### Source city

Proof owner: `CD-869 / CD-565`.

Production interfaces: `Source/PinkCabWorld/Public/World/PinkCabCityIdentity.h`, `Source/PinkCabWorld/Public/World/PinkCabChunkId.h`, `Source/PinkCabWorld/Public/World/PinkCabRoadGraph.h`, `Source/PinkCabWorld/Public/World/PinkCabRouteService.h`, `Source/PinkCabWorld/Public/World/PinkCabCityDeltaState.h`, `Source/PinkCabWorld/Public/World/PinkCabCityLocationRegistry.h`, `Source/PinkCabEconomy/Public/Economy/PinkCabEconomyLedger.h`, `Source/PinkCab/Public/Enforcement/PinkCabEnforcementService.h`, `Source/PinkCabTraffic/Public/Traffic/PinkCabTrafficFlow.h`, `Source/PinkCabTraffic/Public/Traffic/PinkCabTrafficIncident.h`, `Source/PinkCabTraffic/Public/Traffic/PinkCabTrafficInteractionBubble.h`, `Source/PinkCabWorld/Public/World/PinkCabWorldMaterializationPolicy.h`

Fixtures: `Source/PinkCabTests/Private/World/PinkCabCityCodeTests.cpp`, `Source/PinkCabTests/Private/World/PinkCabRouteTests.cpp`, `Source/PinkCabTests/Private/World/PinkCabCityRuntimeTests.cpp`, `Source/PinkCabTests/Private/World/PinkCabCityIntegratedAcceptanceTests.cpp`

### Source streaming

Proof owner: `CD-869 / CD-703 / CD-589`.

Production interfaces: `Source/PinkCabWorld/Public/World/PinkCabWorldMaterializationPolicy.h`, `Source/PinkCabWorld/Public/World/PinkCabCityLocationRegistry.h`, `Source/PinkCab/Public/World/PinkCabL1RoadChunkActor.h`, `Source/PinkCabWorld/Public/World/PinkCabL1EndlessRoadModel.h`, `Source/PinkCab/Public/World/PinkCabL1EndlessRoadStreamer.h`

Fixtures: `Source/PinkCabTests/Private/World/PinkCabWorldMaterializationTests.cpp`, `Source/PinkCabTests/Private/World/PinkCabL1EndlessRoadRuntimeTests.cpp`

### Source traffic

Proof owner: `CD-874 / CD-567 / CD-702`.

Production interfaces: `Source/PinkCabTraffic/Public/Traffic/PinkCabTrafficEntity.h`, `Source/PinkCabTraffic/Public/Traffic/PinkCabTrafficFlow.h`, `Source/PinkCabTraffic/Public/Traffic/PinkCabTrafficMaterializationPolicy.h`, `Source/PinkCabWorld/Public/World/PinkCabCityIdentity.h`, `Source/PinkCabWorld/Public/World/PinkCabChunkId.h`, `Source/PinkCabWorld/Public/World/PinkCabRoadGraph.h`, `Source/PinkCabTraffic/Public/Traffic/PinkCabTrafficIncident.h`, `Source/PinkCabTraffic/Public/Traffic/PinkCabTrafficInteractionBubble.h`, `Source/PinkCabWorld/Public/World/PinkCabRouteService.h`

Fixtures: `Source/PinkCabTests/Private/Traffic/PinkCabTrafficTests.cpp`, `Source/PinkCabTests/Private/Traffic/PinkCabTrafficRuntimeTests.cpp`

### Source enforcement

Proof owner: `CD-878 / CD-591 / CD-569`.

Production interfaces: `Source/PinkCab/Public/Enforcement/PinkCabEnforcementEvent.h`, `Source/PinkCab/Public/Enforcement/PinkCabEnforcementLedger.h`, `Source/PinkCab/Public/Enforcement/PinkCabRoadRuleProfile.h`, `Source/PinkCab/Public/Enforcement/PinkCabEnforcementService.h`, `Source/PinkCabEconomy/Public/Economy/PinkCabEconomyLedger.h`

Fixtures: `Source/PinkCabTests/Private/Enforcement/PinkCabEnforcementTests.cpp`, `Source/PinkCabTests/Private/Enforcement/PinkCabEnforcementRuntimeTests.cpp`

### Source neural

Proof owner: `CD-875 / CD-877 / CD-571`.

Production interfaces: `Source/PinkCabTaxi/Public/Taxi/PinkCabPassengerRecord.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabPassengerTemplate.h`, `Source/PinkCabPersistence/Public/Persistence/PinkCabPassengerSnapshotCodec.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabPassengerSnapshot.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabConductorServiceHooks.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabPassengerHistory.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabNeuralContactState.h`, `Source/PinkCabTaxi/Public/Taxi/PinkCabFareSession.h`

Fixtures: `Source/PinkCabTests/Private/Taxi/PinkCabPassengerRegistryTests.cpp`, `Source/PinkCabTests/Private/Taxi/PinkCabPassengerSnapshotTests.cpp`, `Source/PinkCabTests/Private/Taxi/PinkCabPassengerSocialTests.cpp`, `Source/PinkCabTests/Private/Taxi/PinkCabPassengerHistoryTests.cpp`, `Source/PinkCabTests/Private/Taxi/PinkCabRepeatClientTests.cpp`, `Source/PinkCabTests/Private/Taxi/PinkCabPassengerNeuralTests.cpp`

### Source wallride

Proof owner: `CD-592 / CD-701 / CD-869`.

Production interfaces: `Source/PinkCabVehicle/Public/Vehicle/PinkCabWallrideController.h`, `Source/PinkCabWorld/Public/World/PinkCabVerticalContactRegistry.h`, `Source/PinkCab/Public/Vehicle/PinkCabL1TraversalState.h`, `Source/PinkCab/Public/Vehicle/PinkCabL1TraversalConfig.h`, `Source/PinkCab/Public/Vehicle/PinkCabL1TraversalTransitionPolicy.h`

Fixtures: `Source/PinkCabTests/Private/Vehicle/PinkCabWallrideTests.cpp`, `Source/PinkCabTests/Private/Vehicle/PinkCabVerticalContactTests.cpp`, `Source/PinkCabTests/Private/Vehicle/PinkCabL1TraversalTests.cpp`

### Source freight

Proof owner: `CD-688 / CD-869`.

Production interfaces: `Source/PinkCab/Public/Vehicle/PinkCabL1TraversalState.h`, `Source/PinkCabWorld/Public/World/PinkCabL1RouteHookRuntime.h`, `Source/PinkCabWorld/Public/World/PinkCabRouteService.h`, `Source/PinkCab/Public/Vehicle/PinkCabL1TraversalConfig.h`, `Source/PinkCab/Public/Vehicle/PinkCabL1TraversalTransitionPolicy.h`

Fixtures: `Source/PinkCabTests/Private/World/PinkCabL1RouteHookTests.cpp`, `Source/PinkCabTests/Private/Vehicle/PinkCabL1TraversalTests.cpp`

### Source transit

Proof owner: `CD-705 / CD-710 / CD-869`.

Production interfaces: `Source/PinkCabWorld/Public/World/PinkCabSuspendedBusRuntime.h`, `Source/PinkCabWorld/Public/World/PinkCabMetroTransitRuntime.h`, `Source/PinkCab/Public/Vehicle/PinkCabL1TraversalState.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleLoadState.h`, `Source/PinkCabWorld/Public/World/PinkCabL1RouteHookRuntime.h`, `Source/PinkCab/Public/World/PinkCabVerticalAcceptanceCourse.h`

Fixtures: `Source/PinkCabTests/Private/World/PinkCabSuspendedBusRuntimeTests.cpp`, `Source/PinkCabTests/Private/World/PinkCabMetroTransitRuntimeTests.cpp`, `Source/PinkCabTests/Private/World/PinkCabVerticalAcceptanceTests.cpp`

### Source fuel

Proof owner: `CD-872 / CD-593`.

Production interfaces: `Source/PinkCab/Public/Service/PinkCabFuelTank.h`, `Source/PinkCab/Public/Service/PinkCabMovingFuelSession.h`, `Source/PinkCabEconomy/Public/Economy/PinkCabEconomyLedger.h`, `Source/PinkCab/Public/Persistence/PinkCabServiceSnapshotCodec.h`, `Source/PinkCab/Public/Service/PinkCabServiceNode.h`, `Source/PinkCab/Public/Service/PinkCabServiceOperationRuntime.h`, `Source/PinkCab/Public/Service/PinkCabServiceSnapshot.h`

Fixtures: `Source/PinkCabTests/Private/Service/PinkCabMovingFuelTests.cpp`, `Source/PinkCabTests/Private/Service/PinkCabMovingFuelRuntimeTests.cpp`, `Source/PinkCabTests/Private/Service/PinkCabServiceIntegratedAcceptanceTests.cpp`

### Source economy

Proof owner: `CD-601 / CD-569 / CD-878`.

Production interfaces: `Source/PinkCabEconomy/Public/Economy/PinkCabTransaction.h`, `Source/PinkCabEconomy/Public/Economy/PinkCabEconomyLedger.h`, `Source/PinkCabPersistence/Public/Persistence/PinkCabEconomySnapshot.h`, `Source/PinkCabEconomy/Public/Economy/PinkCabFareSettlementService.h`

Fixtures: `Source/PinkCabTests/Private/Economy/PinkCabEconomyTests.cpp`, `Source/PinkCabTests/Private/Persistence/PinkCabEconomyPersistenceTests.cpp`

### Source recovery

Proof owner: `CD-598 / CD-602 / CD-574`.

Production interfaces: `Source/PinkCab/Public/Persistence/PinkCabPersistenceService.h`, `Source/PinkCab/Public/Persistence/PinkCabRecoveryOrchestrator.h`

Fixtures: `Source/PinkCabTests/Private/Persistence/PinkCabRecoveryTests.cpp`, `Source/PinkCabTests/Private/Persistence/PinkCabPersistenceIntegratedAcceptanceTests.cpp`

### Source service

Proof owner: `CD-871 / CD-711 / CD-713`.

Production interfaces: `Source/PinkCab/Public/Service/PinkCabServiceNode.h`, `Source/PinkCab/Public/Service/PinkCabVehicleBuild.h`, `Source/PinkCab/Public/Service/PinkCabRepairService.h`, `Source/PinkCabEconomy/Public/Economy/PinkCabEconomyLedger.h`, `Source/PinkCab/Public/Service/PinkCabServiceContext.h`, `Source/PinkCab/Public/Service/PinkCabPartCatalog.h`, `Source/PinkCab/Public/Service/PinkCabServiceInventory.h`, `Source/PinkCab/Public/Service/PinkCabServiceOperationRuntime.h`, `Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleHealthState.h`, `Source/PinkCab/Public/Persistence/PinkCabServiceSnapshotCodec.h`, `Source/PinkCab/Public/Service/PinkCabServiceSnapshot.h`

Fixtures: `Source/PinkCabTests/Private/Service/PinkCabServiceNodeTests.cpp`, `Source/PinkCabTests/Private/Service/PinkCabServiceContextTests.cpp`, `Source/PinkCabTests/Private/Service/PinkCabPartsInventoryTests.cpp`, `Source/PinkCabTests/Private/Service/PinkCabServiceOperationTests.cpp`, `Source/PinkCabTests/Private/Service/PinkCabServiceSnapshotTests.cpp`
