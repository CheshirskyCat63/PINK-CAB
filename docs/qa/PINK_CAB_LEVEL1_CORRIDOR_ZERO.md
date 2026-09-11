# PINK CAB QA · Level 1 Corridor Zero

**Status:** CURRENT FIRST-EURO QA SPEC / NOT RUNTIME VERIFIED
Jira proof owner: `CD-700`; gate `CD-679`; detailed QA `CD-701/CD-702/CD-703`.
Authority: Confluence `8388630`; world mirror `docs/PINK_CAB_LEVEL1_WORLD_BIBLE.md`; mass QA `docs/qa/PINK_CAB_VEHICLE_MASS_LOAD_ACCEPTANCE.md`.

## Evidence header

Every relevant run records exact commit/build, UE/FGear/VDS/profile versions, generator/content/schema versions, CityCode and seeds, route/chunk/module IDs, fuel/crew/passenger masses, total mass, expected/observed residual timeout, contact/body telemetry, outcome, artifacts and bounded-runtime counters.

Temporary implementation values may not be promoted to production authority. Owner-open values remain explicit; plugin tuning inside a locked observable contract is CALIBRATION.

## Locked Level1 fixtures

- base Tatra **1450 kg**;
- full-fuel Tatra **1550 kg**;
- heroine **58 kg**;
- daughter **49 kg**;
- passenger masses use authored values;
- reference full-fuel crew-only **1657 kg**;
- declared max passenger fixture **2107 kg**;
- no ABS / no ESP;
- FGear remains sole road-dynamics owner.

Level1 residual magnetism is **LOCKED LINEAR** by authoritative total mass:

- `m <= 1657 kg` → **5.0 s** cap;
- `1657 < m < 2107 kg` → `timeout_s = 5.0 - (m - 1657) / 450`;
- `m = 2107 kg` → **4.0 s**;
- `m = 1882 kg` → **4.5 s**.

The old global fixed-five-second rule and `interpolation OPEN` wording are SUPERSEDED. Magnetic force/contact/reacquisition behavior remains calibration/owner work under `CD-592/CD-701`; reacquisition reset semantics remain owner-pack item M05.

## Continuous-flow assertions

Level1 ordinary road generation has no traffic-light/red-light dependency and no systemic standing-jam state. Heavy traffic may compress gaps/slow locally but must retain a moving bypass around normal incidents. Disabled/stopped lane obstacles are explicit incidents/failure states, not the default traffic model.

## Acceptance families

| ID | Area | Required proof | Current authority |
|---|---|---|---|
| `L1-XSEC-001` | road cross-section | five express lanes + required right accel/decel/local/frontage continuity | geometry owner active |
| `L1-XSEC-002` | right entry/exit | ordinary exits/entries originate right in both longitudinal directions | LOCKED DIRECTION |
| `L1-FLOW-001` | no signals | no ordinary traffic-light actor/phase/red-light rule dependency | LOCKED SPEC |
| `L1-FLOW-002` | moving flow | heavy normal traffic remains moving; no systemic jam state | LOCKED; density/gap numerics open |
| `L1-TRAF-001` | weaving | deterministic traffic provides playable gaps without forced collision | numerics open `CD-567` |
| `L1-INC-001` | incidents | approved incident family replays deterministically and cleans up | frequency/cleanup open |
| `L1-INC-002` | bypass | traffic responds with bounded moving bypass | LOCKED DIRECTION |
| `L1-SURF-001` | road surface | rut/patch/crack/water behavior consumes versioned surface/FGear profiles | calibration active |
| `L1-WALL-001` | wallride entry | road→debris→wallride valid across low/nominal/high entry fixtures | numerics open `CD-592` |
| `L1-WALL-002` | seven bands | stable/recoverable seven-band state; Tatra footprint ~3 bands | identity locked / tuning open |
| `L1-WALL-003` | failure | rear-first/front-first/contact-loss cases resolve deterministically | thresholds open |
| `L1-WALL-LOAD-001` | load direction | heavier matched fixture modestly reduces abrupt separation while body penalty rises | DIRECTION LOCKED / CALIBRATION |
| `L1-MASS-001` | total mass | exact base+fuel+crew+passenger composition/persistence | LOCKED SPEC |
| `L1-ELEC-001` | ABS/ESP | no ABS/ESP physical intervention/equipment | LOCKED SPEC |
| `L1-MAG-001` | ceiling acquire | valid receiving strip reaches stable contact | force/envelope open |
| `L1-MAG-REF-001` | residual 1657 | 1657 kg effective timeout = 5.0 s | LOCKED SPEC |
| `L1-MAG-MAX-001` | residual 2107 | 2107 kg effective timeout = 4.0 s | LOCKED SPEC |
| `L1-MAG-MID-001` | intermediate | at least 3 intermediate masses match exact linear function; 1882=4.5 s | **LOCKED LINEAR** |
| `L1-MAG-CAP-001` | lighter cap | legal mass below 1657 never exceeds 5.0 s | LOCKED SPEC |
| `L1-MAG-006` | bad contact | invalid speed/angle receiving contact rejects deterministically | thresholds open |
| `L1-FRT-001` | freight direction | exactly five upper freight lanes, always opposite lower road | LOCKED SPEC |
| `L1-FRT-002` | moving obstacles | single/convoy/lane-change fixtures remain readable and bounded | weights/numerics open |
| `L1-JUMP-001` | side swap | approved ceiling/poplar window completes with current load/contact state | geometry open |
| `L1-JUMP-002` | failed side swap | failure uses common VDS/Vehicle Health/recovery path with no stuck attachment | recovery details active |
| `L1-PARK-001` | ServiceNode ingress | live road→Practice Hangar where required | state/geometry partial |
| `L1-PARK-003` | return | exit/reset returns same Tatra + CityCode without duplication | LOCKED DIRECTION |
| `L1-RULE-001` | signs | sign resolves exact machine-readable RuleId | partial |
| `L1-RULE-002` | cameras | only declared non-signal rules generate enforcement facts | thresholds open |
| `L1-REV-001` | reverse reconstruction | same CityCode/version preserves static module identity | LOCKED DIRECTION |
| `L1-PERF-001` | 30-min soak | Actor/chunk/memory/pool state remains bounded | budgets open |
| `L1-PERF-002` | restart/reload | no transient incident/contact/mass duplication | LOCKED DIRECTION |

## CD-701 mass / vertical boundary suite

Required deterministic cases include exact mass composition/save reconstruction; no ABS/ESP; smooth load trends; wallride entry/failure/contact; five-lane opposite freight; **1657=5.0 s**, **2107=4.0 s**, at least three exact linear intermediate fixtures including **1882=4.5 s**, lighter 5.0 s cap; side-swap; return-to-road; collision/abort/restart with zero stuck attachment.

## CD-702 road / traffic / incident suite

Cover nominal/heavy/motorcycle traffic, no-signal/no-standing-jam invariants, right-exit friction, parking legality, surface response, bounded incidents and machine-readable sign/camera rule ownership.

## CD-703 forward / reverse soak

Run 15+ minutes forward, exercise right exit/return, incident, wallride/ceiling cycle and ServiceNode ingress; reverse/revisit; restart/reload; verify static IDs, mass, transaction/contact cleanup and bounded counters. Fail on unbounded growth, route reshuffle, seam collision, duplicated/rerolled passenger mass, standing deadlock or signal-phase dependency.

## Recovery boundary

FIRST EURO has no insurance-specific vertical branch. Terminal/immobilized outcomes emit the common Vehicle Health/recovery event consumed by `CD-598/CD-750`. Future insurance may attach to the generic `RecoveryPolicy/RecoveryHook` after the first year without changing wallride physics.

Documentation establishes SPECIFIED only. Runtime remains **NOT VERIFIED** until exact-build evidence passes applicable rows.
