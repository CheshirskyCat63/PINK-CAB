# PINK CAB · BASE-100 Technical Owner Pack 01

**Status:** CURRENT EXHAUSTIVE CODE-ONLY OWNER PACK
**Snapshot:** 2026-09-11
**Program:** `CD-746`
**Domains:** `CD-747..753`
**Readiness:** `CD-660/CD-661` / Confluence `6553617` / `docs/PINK_CAB_START90_READINESS.md`
**Authority:** `docs/PINK_CAB_BASE100_CODE_ARCHITECTURE.md` / Confluence `11239425`
**Visual/art questions:** EXCLUDED unless they create runtime/API/performance contracts

## How to use

This pack contains implementation-facing decisions that can change code, state, data, numbers, plugin/engine ownership, persistence, performance or test contracts.

Statuses:

- `LOCKED` — already decided by owner/current authority;
- `OPEN` — requires owner answer;
- `PROPOSED DEFAULT` — recommended engineering default, not owner-locked yet;
- `CALIBRATION` — implementation tuning inside an already locked observable contract;
- `POST-FIRST-EURO` — not implemented in first 12 months; preserve only stated extension boundary.

Readiness scoring uses `LOCKED/CALIBRATION = 1.0`, `PROPOSED DEFAULT = 0.5`, `OPEN = 0.0`. A proposed default never masquerades as owner canon.

Owner may answer compactly, e.g. `F05 B; H04 no; all other proposed defaults accepted`.

## Scope locks already accepted

- FIRST EURO = first 12 months of development;
- PC single-player only;
- L1 + L2 fully implemented during FIRST EURO;
- L3 gameplay POST-FIRST-EURO, schema/route extension boundary only;
- automotive ServiceNodes in FIRST EURO: Parking + Garage/Tuning + Parts + Repair, plus Practice Hangar only where Level1 training/acceptance requires it;
- mall/food/bar/club/lifestyle ServiceNodes POST-FIRST-EURO;
- persistent repeat clients + basic Neural in FIRST EURO; no broad social simulation;
- Taxi Regulator implementation POST-FIRST-EURO; stable `EnforcementEvent` interface in FIRST EURO;
- daily insurance implementation POST-FIRST-EURO; only generic recovery extension boundary remains;
- moving refueling gameplay is FIRST EURO;
- occupant injury/death simulation is not a FIRST EURO requirement under current `CD-600`; terminal vehicle recovery remains `I06`;
- visual presentation is outside this technical pack unless it creates runtime/API/performance requirements.

---

## A · Runtime / project structure / PC release foundation

- `A01 LOCKED` ? production engine line is Unreal Engine 5.8. Bootstrap machine currently has UE 5.8.2 (`++UE5+Release-5.8`, CL 56702186). Stay on the pinned 5.8 production line; changing the production engine line requires an explicit compatibility/migration decision.
- `A02 PROPOSED DEFAULT` — C++ core + thin Blueprint composition/orchestration.
- `A03 LOCKED` - runtime modules: `Core / Vehicle / Taxi / World / Traffic / Economy / Persistence / Interaction`; optional `Editor / Tests` are non-shipping; do not overfragment.
- `A04 PROPOSED DEFAULT` — modules consume public contracts only; no private cross-module internals.
- `A05 PROPOSED DEFAULT` — UE Subsystems over custom global singleton managers.
- `A06 PROPOSED DEFAULT` — Actors are not universal persistent state owners.
- `A07 LOCKED` - initial flags are `L2 / Damage / Neural / MovingFuel / ServiceNodes`; `Core FeatureConfig` owns them and feature modules only consume the values.
- `A08 PROPOSED DEFAULT` — debug/test behavior is compile/config gated; Shipping gameplay cannot depend on debug framework.
- `A09 LOCKED` - canonical clean-checkout build entrypoint is `scripts/build.ps1`; `\.\scripts\build.ps1` is the human/CI build command. The wrapper calls the pinned UE 5.8 native Build.bat/RunUAT.bat toolchain; Visual Studio is an IDE, not build authority. Packaging uses the same wrapper with `-Package`.
- `A10 LOCKED` - CI is GitHub Actions on a self-hosted Windows Unreal runner; local and CI use the same canonical build/smoke scripts; production merge is fail-closed if canonical CI is unavailable; explicit local evidence is bootstrap fallback only.
- `A11 PROPOSED DEFAULT` — build identity/artifact/log/crash contract records commit SHA, BuildId, content/config/schema versions and structured crash/log paths.
- `A12 PROPOSED DEFAULT` — one deterministic smoke invocation launches canonical greybox, reports identity/fixtures and exits with machine-readable pass/fail.
- `A13 LOCKED` - FIRST EURO support matrix is Windows 10/11 x64 + DX12 + Steam only; Linux/macOS/additional storefronts are post-FIRST-EURO.
- `A14 LOCKED` - acceptance seed: minimum 1080p60 Low on Ryzen 3600/i5-10400 6-core class, 16 GB RAM, SSD, GTX 1660/RX 5600 class; recommended 1440p60 High on Ryzen 5600X/i5-12400 class, 32 GB RAM, SSD, RTX 3060 Ti/RX 6700 XT class; 16.67 ms frame target with ~14 ms GPU and ~8 ms Game Thread budget seeds; hardware equivalence is empirically validated.
- `A15 LOCKED` - required settings: resolution/window, VSync/frame cap/scalability, FOV, camera shake, mouse sensitivity, Master/Music/SFX/Voice, subtitles + size/background, UI scale, hold/toggle interaction, with persistence.
- `A16 LOCKED` — primary optimization direction is 1440p and 60+ FPS; exact frame-time/hardware tiers remain A14.

## B · Data-driven architecture

- `B01 PROPOSED DEFAULT` — gameplay tunables in versioned DataAssets/DataTables/config, not scattered C++ literals.
- `B02 PROPOSED DEFAULT` — separate profiles per system rather than one giant GameConfig.
- `B03 PROPOSED DEFAULT` — every persistent entity has stable ID.
- `B04 PROPOSED DEFAULT` — generated world IDs deterministic; personal persistent identities stable generated IDs/GUIDs.
- `B05 PROPOSED DEFAULT` — runtime systems never mutate immutable source DataAssets.
- `B06 PROPOSED DEFAULT` — balance/tuning should normally change without recompilation.
- `B07 PROPOSED DEFAULT` — config/profile versions explicit.
- `B08 PROPOSED DEFAULT` — incompatible save/schema uses explicit migration/failure, never silent guess/fallback.

## C · Input / physical interaction

- `C01 PROPOSED DEFAULT` — one Enhanced Input router/adapter distributes semantic commands.
- `C02 LOCKED` — FGear does not consume raw Enhanced Input directly; PINK CAB vehicle adapter sits between.
- `C03 LOCKED` — steering/throttle/brake/clutch/handbrake are continuous normalized states/commands.
- `C04 PROPOSED DEFAULT` — one reusable physical-control interaction interface for cabin switches/levers/controls.
- `C05 PROPOSED DEFAULT` — physical controls emit semantic commands; they do not reach into unrelated subsystem internals.
- `C06 LOCKED` — common interaction grammar is `1–4 START -> LMB ATTENTION -> RMB GO`; Space gaze; default mouse steering.
- `C07 PROPOSED DEFAULT` — bounded interaction trace/current target; no world scan.
- `C08 LOCKED` - FIRST EURO includes semantic KBM rebinding with conflict detection and Restore Defaults; rebinding cannot change the canonical physical-control model.

## D · Vehicle / FGear

- `D01 LOCKED` — FGear is sole hero-Tatra road-dynamics solver.
- `D02 PROPOSED DEFAULT` — ordinary traffic uses cheaper logical/kinematic backend; FGear reserved for player/selected physically relevant vehicles.
- `D03 PROPOSED DEFAULT` — one versioned Tatra vehicle profile with nested structs/profiles.
- `D04 PROPOSED DEFAULT` — road surfaces resolve through stable `SurfaceId`/surface profile.
- `D05 PROPOSED DEFAULT` — dry/wet/storm are surface/environment modifiers, not duplicate vehicle definitions.
- `D06 LOCKED` — passenger/fuel load contributes actual authoritative total mass.
- `D07 LOCKED` — load implementation must affect mass and CG, not mass alone.
- `D08 LOCKED` — Tatra Expression cannot modify authoritative forces/trajectory.
- `D09 LOCKED` — wallride/magnet is an external force/contact/state layer over FGear, never replacement physics.
- `D10 PROPOSED DEFAULT` — gameplay consumes normalized vehicle telemetry interface rather than vendor-specific FGear types.

## E · Vehicle Damage / Health

- `E01 LOCKED` — VDS owns deformation; functional damage is authored Vehicle Health mapping.
- `E02 LOCKED` — no universal vehicle HP bar/state as primary damage model.
- `E03 LOCKED` — component-oriented health: wheels, steering/alignment, suspension, brakes, door, lamps, tire path, air-cooled engine thermal/oil/fan/oil-cooler/airflow and authored extensions.
- `E04 LOCKED` — functional consequence derives from authored hit zone + collision severity/threshold mapping.
- `E05 LOCKED` — cosmetic deformation may have no functional handling effect.
- `E06 LOCKED` — relevant damage/health persists.
- `E07 PROPOSED DEFAULT` — repair supports component repair plus optional full-service transaction.
- `E08 LOCKED` — terminal vehicle failure is capability-based, not `HP == 0`.

## F · Taxi work / FareSession / cabin-fare edges

- `F01 PROPOSED DEFAULT` — one authoritative active FareSession for hero taxi.
- `F02 PROPOSED DEFAULT` — states: Idle / RouteAssigned / ApproachingPickup / StoppedForPickup / Boarding / Occupied / StoppedForDropoff / AwaitingPayment / Completed|Failed.
- `F03 LOCKED` — pickup/dropoff eligibility requires deliberate full stop in valid context.
- `F04 LOCKED` - FullStop = vehicle speed <0.5 km/h continuously for 0.4 s; this technical epsilon/dwell is used for passenger exchange and manual quit eligibility.
- `F05 LOCKED` - after all passengers are seated and the passenger door is closed/latched, the driver physically presses START; fare distance/time does not accrue before manual START.
- `F06 LOCKED` - reaching the destination zone does not auto-stop the meter; after destination eligibility + FullStop the driver physically presses STOP, then FareSession enters AwaitingPayment.
- `F07 PROPOSED DEFAULT` — internal fare accumulator continuous; presentation may round/display discretely.
- `F08 LOCKED` - after START, fare time accrues for the whole active trip including ordinary stops/waiting; ESC/system hard-pause time is excluded. Passenger-requested active-meter stopovers may open the passenger door and let the passenger leave temporarily for a 30-35 second authored errand/thematic beat without ending the FareSession or stopping the meter; return is normal, fare evasion remains governed by F19.
- `F09 PROPOSED DEFAULT` — distance basis = actual odometer/path traveled, not route-plan distance.
- `F10 LOCKED` — METERED/OFF_METER are modes of the same fare system, not duplicate fare frameworks.
- `F11 LOCKED` - a passenger-requested trip cancellation after boarding is not a normal convenience path: it represents a serious crash/terminal service failure, the active fare fails, the workday ends, and repair/recovery follows. Temporary passenger errands during an active fare are not cancellation.
- `F12 LOCKED` - while the current FareSession is occupied, a second active fare is forbidden; offers may be surfaced/buffered but cannot become another active fare.
- `F13 LOCKED` - next cycle is STOP -> payment/receipt -> passenger exit -> RESET/Idle -> new START; the next fare cannot begin until the previous fare is closed exactly once.
- `F14 OPEN` — receipt blocking/removal policy: auto-eject/non-blocking vs physical removal gate before next fare.
- `F15 LOCKED` - passenger-door opening/closing is a deliberate cabin interaction through the physical handle/lever interface while using gaze/head-look; no magical automatic close is authoritative. The door may be opened while the meter is active for passenger errands without ending the FareSession.
- `F16 LOCKED` - right-hand-first cabin manipulation is authoritative where physically plausible so the left hand can preserve steering continuity; authored exceptions must be explicit, not automatic nearest-hand ambiguity.
- `F17 LOCKED` - no hidden keyboard shortcut path for physical cabin controls such as ignition/lights/wipers/meter/doors/radio. Interaction stays visible and immersive through the cabin interface; convenience may reduce friction through explicit accessibility/assist settings, while a more demanding interaction profile may be opt-in.
- `F18 OPEN` — versioned curb/off-meter economy profile: tip uplift, pricing/negotiation if retained, complaint/risk eligibility and thresholds.
- `F19 OPEN` - fare evasion is canon and may occur when a passenger leaves during an active-meter errand/stopover and does not return or pay. Exact eligible PassengerTemplates, probability/trigger model, consequences and exactly-once outcome mapping remain owner-open.
- `F20 CALIBRATION` — passenger-door mechanical envelope after physical/open-close semantics are locked: lever/hinge travel, latch timing, animation duration, reach tolerance and authored damage thresholds through Vehicle Health.

## G · Passenger / repeat clients / Neural

- `G01 LOCKED` - procedural PassengerTemplate and persistent PassengerIdentity are separate. A materialized passenger exposes a stable public PassengerIdentity profile before service when the driver looks at them near the cab.
- `G02 OPEN` — promotion frequency/eligibility for persistent repeat clients.
- `G03 PROPOSED DEFAULT` — promotion uses deterministic score + seeded randomness, not uncontrolled random roll.
- `G04 OPEN` — relationship dimensions. Proposed compact axes: Trust / Satisfaction / RiskTolerance.
- `G05 PROPOSED DEFAULT` — preferences may influence tip/repeat/acceptance outcomes.
- `G06 LOCKED` - basic Neural is data-driven contact/history/simple-message layer, not broad dialogue/life simulation. The local player may leave a quick star rating plus short text review on a PassengerIdentity; the schema must preserve review provenance for later shared/community propagation without requiring an online runtime in FIRST EURO.
- `G07 LOCKED` — repeat order reuses normal Order/FareSession pipeline.
- `G08 LOCKED` — persistent passenger can exist logically without Actor.
- `G09 PROPOSED DEFAULT` — only locally required passenger groups materialize.
- `G10 LOCKED` — materialized Actor may recycle/destroy while PassengerIdentity persists.

## H · Economy / transaction ledger

- `H01 PROPOSED DEFAULT` — one EconomyService owns money balance/settlement.
- `H02 PROPOSED DEFAULT` — FareIncome / Tip / FuelPurchase / PartPurchase / Repair / Parking / Fine are typed transactions.
- `H03 PROPOSED DEFAULT` — persistent/retriable transactions use stable TransactionId + exactly-once semantics.
- `H04 LOCKED` - bounded debt is allowed only for essential recovery, minimum roadworthy repair and mandatory day obligations; tuning and discretionary purchases may not create debt.
- `H05 LOCKED` - fuel purchase is capped to affordable volume; ordinary parts are denied when unaffordable; Repair may offer minimum roadworthy repair through allowed bounded debt; insufficient funds must not permanently block sleep/next Workday.
- `H06 PROPOSED DEFAULT` — prices live in data/config profiles.
- `H07 PROPOSED DEFAULT` — economy owns no UI/animation presentation.
- `H08 PROPOSED DEFAULT` — tips settle as separate transaction from fare principal.

## I · Session / save / workday

- `I01 LOCKED` - 3 campaign slots; each owns a bounded rolling autosave/checkpoint ring; no unbounded manual-save exploit set.
- `I02 LOCKED` - atomic save after fare settlement, service purchase/repair, refuel settlement, repeat-client promotion, Workday end and meaningful persistent world change; safe periodic checkpoint every 5 minutes; 3 rolling checkpoints per slot.
- `I03 LOCKED` - manual quit/save+exit is allowed only with no active FareSession/passenger and after a full vehicle stop; force-quit/crash anti-exploit remains I13.
- `I04 LOCKED` - manual quit/reload is unavailable while a FareSession/passenger is active; the order must be completed first; I13 owns force-quit/crash reconstruction anti-exploit.
- `I05 PROPOSED DEFAULT` — reconstruct transient passenger/traffic Actors from logical session state instead of serializing raw actor transforms broadly.
- `I06 LOCKED` - terminal/immobilizing Tatra fails any active fare, ends the Workday, commits an atomic save and recovers the still-damaged car to Repair for the next Workday; no free reset; fee applies under separate H04/H05 insufficient-funds policy.
- `I07 PROPOSED DEFAULT` — Workday has stable deterministic/logical ID.
- `I08 LOCKED` - one Workday is 12 in-game hours over 120 real minutes (time scale x6); early in-car sleep/end-day is permitted when eligible.
- `I09 LOCKED` - ESC/system menu is a hard single-player pause for world, fare and economy time; in-world dashboard/radio UI does not pause; transactions cannot be left mid-commit.
- `I10 LOCKED` — sleep is the intended diegetic end-of-day/save+exit affordance; detailed eligibility remains separate.
- `I11 LOCKED` - sleep/end-day occurs in the car at full stop with no active fare; location is unrestricted, but parking legality/signage and ordinary fines still apply; vehicle state persists as left.
- `I12 LOCKED` - Workday-end summary owns gross fare income, tips, fines, fuel, repair/service, net result and available money, plus exactly one mandatory household/family transaction for the day before next-cycle transition.
- `I13 LOCKED` - force quit/crash cannot roll back committed purchases, refuel, fines, fare/payment or damage; resume uses the last committed logical state and safely materializes vehicle/world without free rollback.

## J · CityCode / generation / streaming

- `J01 PROPOSED DEFAULT` — world identity = CityCode + GeneratorVersion + ContentSetVersion.
- `J02 PROPOSED DEFAULT` — CityCode is immutable for an existing campaign save.
- `J03 PROPOSED DEFAULT` — generated chunks/modules use deterministic coordinate/ID.
- `J04 PROPOSED DEFAULT` — persist seed/version + meaningful deltas, not complete generated world serialization.
- `J05 OPEN` — exact mandatory persistent-delta classes. Proposed core: vehicle/owned/service/economy/passenger identities + selected persistent incidents/world changes.
- `J06 PROPOSED DEFAULT` — ordinary temporary traffic is not persisted when unloaded.
- `J07 OPEN` — which incidents persist across streaming/save. Proposed: bounded important/active incidents only.
- `J08 PROPOSED DEFAULT` — same generator version + inputs reproduce deterministic logical world across supported machines.
- `J09 OPEN` — old-generator save policy: retain reconstruction version vs migrate world. Proposed: preserve versioned reconstruction; migrate only explicitly.
- `J10 PROPOSED DEFAULT` — UE World Partition/PCG/engine streaming + thin deterministic PINK CAB recipe layer; no bespoke world-streaming engine by default.
- `J11 OPEN` — exact chunk-length/lookahead/live-recent-reconstructable/cache/recycle numeric envelope required by CityCode streaming tests.
- `J12 OPEN` — exact world-streaming actor/component/memory/materialization ceilings and reverse-cache budget used by performance acceptance.

## K · Road graph / routing / traffic / gameplay geometry

- `K01 PROPOSED DEFAULT` — one road/lane graph feeds generation, routing, traffic and rule lookup.
- `K02 PROPOSED DEFAULT` — lane is graph/data identity, not Actor-per-lane.
- `K03 PROPOSED DEFAULT` — no full hero-level physics for every traffic car.
- `K04 OPEN` — exact physical-materialization radius/rules for traffic collision relevance.
- `K05 PROPOSED DEFAULT` — non-materialized traffic uses bounded graph/kinematic simulation.
- `K06 OPEN` — density control representation. Proposed: density profile + headway distribution.
- `K07 LOCKED` — ordinary flow must preserve playable traffic gaps by design.
- `K08 OPEN` — incident bypass behavior. Proposed: bounded moving bypass rather than systemic jam.
- `K09 LOCKED` — no designed systemic standing-jam state; temporary local zero speed can occur physically.
- `K10 PROPOSED DEFAULT` — speed enforcement uses authoritative vehicle speed + rule zone/context.
- `K11 OPEN` — Level1 base road cross-section and implementation geometry profile: express/shoulder/accel-decel/local/parking/micromobility/pedestrian/median/frontage/ramp/bridge/route-warning dimensions.
- `K12 OPEN` — Level1 gameplay-geometry envelope for wallride/debris/freight-ceiling/receiving strips/poplar side-swap apertures and valid approach/landing/no-spawn clearances. FGear/contact force values remain calibration under M/CD-592.
- `K13 OPEN` — right-entry pipe/interchange numeric geometry profile: entry/decel/merge throat/tube diameter-radius-banking-pitch-grade/lane-barrier/near-miss/recovery/streaming clearances.

## L · Enforcement / fine / reputation interface

- `L01 LOCKED` — one stable `EnforcementEvent` schema for rule producers.
- `L02 PROPOSED DEFAULT` — fields: EventId / Type / Timestamp / CityLocationId / VehicleId / optional PassengerId / Severity / EvidenceFlags / Context.
- `L03 PROPOSED DEFAULT` — FIRST EURO ordinary cameras/rules may convert events into fines/ordinary consequences.
- `L04 LOCKED` — future Taxi Regulator subscribes to same producer contract without producer rewrites.
- `L05 OPEN` — event retention/history lifetime. Proposed: bounded recent ledger + aggregated persistent reputation/consequence state.
- `L06 OPEN` — FIRST EURO speed/camera/lane/stopping/parking/bus-or-magnetic/toll/fine numeric profile and escalation/tolerance rules.
- `L07 OPEN` — city reputation/social-credit event weights, thresholds, visibility/downstream effects and any decay/recovery profile retained in FIRST EURO.

## M · Level 1 traversal

- `M01 LOCKED` — wallride is state/external constraint over FGear, not alternate vehicle solver.
- `M02 LOCKED` — seven wallride bands are gameplay/data bands; Tatra occupies approximately three.
- `M03 PROPOSED DEFAULT` — magnet state identity: Detached / Contact / Residual / Expired.
- `M04 LOCKED` — residual timeout is linear by authoritative mass: 5.0 s @1657 kg -> 4.0 s @2107 kg, lighter legal states cap 5.0 s.
- `M05 OPEN` — reacquisition reset semantics.
- `M06 PROPOSED DEFAULT` — gap without valid contact consumes residual countdown.
- `M07 LOCKED` — wallride failure outcome follows physical result/damage/recovery; no special scripted universal fail.
- `M08 PROPOSED DEFAULT` — freight ceiling traffic uses the common cheaper traffic backend/profile/graph, not hero physics per vehicle.

## N · Level 2

- `N01 PROPOSED DEFAULT` — suspended buses are traffic entities on Level2 graph, not a separate transport simulator.
- `N02 PROPOSED DEFAULT` — metro uses schedule/state owner + bounded materialized train actors.
- `N03 PROPOSED DEFAULT` — FIRST EURO passengers do not require full pedestrian metro-usage simulation unless separately promoted.
- `N04 LOCKED` — Tatra-on-metro is not normal canon/default gameplay.
- `N05 OPEN` — exact station gameplay function beyond world/route landmark.
- `N06 OPEN` — final suspended-bus lane count/cross-section identity. It is explicitly not fixed to two; owner must lock the profile before Transit Zero can treat it as calibration.
- `N07 OPEN` — metro operational timing/direction contract: headway/dwell/direction/counter-phase owner behavior. Low-level timing values become calibration after the state contract is locked.
- `N08 LOCKED` — L1 and L2 share CityCode/chunk/persistence architecture.
- `N09 OPEN` — Level2 structural geometry profile: bus elevation/clearances, metro elevation and two-track-per-side spacing, train envelope, station approach/platform/access and L1 sightline-safe collision/streaming clearances.

## O · Automotive ServiceNodes

- `O01 PROPOSED DEFAULT` — common state: Approach -> Eligible -> Enter -> NodeActive -> Commit -> Exit -> ResumeWorld.
- `O02 OPEN` — implementation form: streamed sublevel/instance/seamless same-world approach; keep caller abstraction independent.
- `O03 OPEN` — Parking responsibilities: storage only vs save/day-end/checkpoint capabilities.
- `O04 PROPOSED DEFAULT` — parts inventory uses stable item IDs and bounded quantities/slots as defined by profile.
- `O05 PROPOSED DEFAULT` — Garage operates on versioned `VehicleBuild` component/slot data.
- `O06 LOCKED` — Repair consumes/restores the same component Vehicle Health state owned by damage system.
- `O07 LOCKED` — installed parts modify FGear only through approved vehicle profile/adapter.
- `O08 LOCKED` — ServiceNodes request EconomyService transactions; they never own balance.

## P · Moving refueling

- `P01 LOCKED` — separate gameplay session using common FuelTank/Economy/VehicleTelemetry/RoadGraph; no parallel fuel/money/vehicle state.
- `P02 PROPOSED DEFAULT` — states: SearchingQueue / Queued / ApproachingSlot / Connected / Fueling / PaymentPending / Disconnecting / Completed|Aborted.
- `P03 OPEN` — queue/slot policy. Proposed first version: one deterministic active service slot per encounter.
- `P04 PROPOSED DEFAULT` — connection eligibility uses explicit distance/speed envelope.
- `P05 OPEN` — target longitudinal gap/tolerance; previous concept around ±25 cm is a calibration seed, not locked.
- `P06 PROPOSED DEFAULT` — tolerance breach: warning/grace -> disconnect rather than instant arbitrary failure.
- `P07 PROPOSED DEFAULT` — fuel transfer integrates liters/second from FuelProfile.
- `P08 LOCKED` — fuel credit occurs only while valid connected/fueling state exists.
- `P09 OPEN` — settlement model. Proposed: accrued liability while fueling + atomic final settlement with crash-safe transaction checkpoints.
- `P10 PROPOSED DEFAULT` — player can end fueling before full tank.
- `P11 OPEN` — insufficient-funds behavior tied to H04/H05 debt policy.
- `P12 LOCKED` — save/crash/retry cannot duplicate fuel or avoid corresponding settlement; exactly-once required.
- `P13 LOCKED` — NPC/hose/animation presentation follows state machine and cannot be authoritative through animation notify.
- `P14 CALIBRATION` — L1/L2 fuel price belongs to FuelPriceProfile; exact values tune later.
- `P15 CALIBRATION` — after P02/P03/P05/P09 semantics are locked, service-vehicle speed, warning/grace bands, grade/relative-height, hose break threshold if gameplay-relevant, transfer rate and telemetry tolerances tune in versioned profiles rather than becoming separate owner questions.

## Q · Plugin / asset policy

- `Q01 PROPOSED DEFAULT` — mandatory engine/Fab/plugin/library audit before large custom subsystem.
- `Q02 PROPOSED DEFAULT` — purchased plugin is always wrapped by adapter/interface.
- `Q03 LOCKED for FGear/VDS` — vendor APIs do not spread throughout gameplay; same doctrine proposed globally.
- `Q04 PROPOSED DEFAULT` — vendor replacement should primarily require adapter/config changes, not whole-game rewrites.
- `Q05 PROPOSED DEFAULT` — mature 80% solution + thin product-specific extension preferred to custom 100% framework.
- `Q06 PROPOSED DEFAULT` — no two competing production plugins for one responsibility after selection.
- `Q07 PROPOSED DEFAULT` — bake-off/probe branches allowed before final lock.
- `Q08 PROPOSED DEFAULT` — dependency manifest records version/source/license/owner and evidence compatibility.

## R · Performance architecture

- `R01 PROPOSED DEFAULT` — no Actor-per-object for data-only/logical/static repeated entities.
- `R02 PROPOSED DEFAULT` — all enabled Tick has bounded explicit purpose; no unbounded Tick work.
- `R03 PROPOSED DEFAULT` — no global actor scans in normal gameplay hot path.
- `R04 PROPOSED DEFAULT` — pool/recycle repeated traffic/passenger/incident/presentation populations where appropriate.
- `R05 LOCKED as doctrine` — logical identity and materialized Actor can be separate.
- `R06 PROPOSED DEFAULT` — growing runtime collections require explicit caps/eviction/lifetime.
- `R07 PROPOSED DEFAULT` — chunks/materialized actors recycle while persistent logical state remains separate.

## S · Testing / evidence

- `S01 PROPOSED DEFAULT` — each gameplay state machine/system has deterministic test fixture where practical.
- `S02 PROPOSED DEFAULT` — one shared evidence/runner stack, no per-feature QA framework.
- `S03 PROPOSED DEFAULT` — test record includes exact engine/plugin/config version + seed/CityCode where relevant.
- `S04 LOCKED doctrine` — compile/build PASS is not gameplay VERIFIED.
- `S05 PROPOSED DEFAULT` — logic automation/headless tests plus separate player-observable runtime acceptance where needed.
- `S06 PROPOSED DEFAULT` — every persistent subsystem gets save/load/reload/retry coverage.
- `S07 PROPOSED DEFAULT` — exactly-once fare/economy/fuel/service paths get duplication/crash/interruption tests.
- `S08 PROPOSED DEFAULT` — fixed worst-case corridor/traffic/load fixtures for performance evidence.

## Current census / readiness input

This normalized pack contains **196 code-facing rows**:

- **75 LOCKED**
- **3 CALIBRATION**
- **88 PROPOSED DEFAULT**
- **30 OPEN**

Readiness points: `75 + 3 + 88×0.5 = 122`.

Current item-weighted START-90 specification readiness: `122 / 196 = 62.2%`.

Domain snapshots under the same rubric:

- CORE `A/B/C/Q/R/S`: **64.5%**
- VEHICLE `D/E/M/N`: **68.6%**
- TAXI `F/G`: **71.7%**
- STATE `H/I`: **81.0%**
- WORLD `J/K/L`: **34.4%**
- SERVICE `O/P`: **56.5%**
- SCOPE `CD-753`: **100% LOCKED**, reported separately and not allowed to hide weak technical domains.

If every PROPOSED DEFAULT is owner-accepted, score becomes `166/196 = 84.7%`. At least **11 of the 30 OPEN rows** must then close to reach `177/196 = 90.31%` and cross START-90.

## Current owner-answer priority

All genuine OPEN rows are:

`F14 F18 F19 G02 G04 J05 J07 J09 J11 J12 K04 K06 K08 K11 K12 K13 L05 L06 L07 M05 N05 N06 N07 N09 O02 O03 P03 P05 P09 P11`.

Highest structural priority for broad-start readiness: `J05/J07/J09/J11/J12`, `K04/K06/K08/K11-K13`, `L05-L07`, then the remaining taxi/service/transit owner rows.

Everything not OPEN is either already locked, an engineering default awaiting batch acceptance, or calibration inside a locked observable contract. No admin process may silently promote an OPEN or PROPOSED DEFAULT row to LOCKED.
