# PINK CAB QA · FARE ZERO Physical Taxi Loop

Jira gate: `CD-543`
Executable QA owner: `CD-724`
Authority: Confluence `5832724`; Git `docs/PINK_CAB_TAXI_WORK_PHYSICALITY.md`.

## Evidence header

Every run records:

- exact commit/build id;
- engine/platform/configuration;
- schema/content versions;
- CityCode;
- passenger template/group ids;
- pickup context id (`OFFICIAL_STOP` / `CURB_PICKUP`);
- route/job id;
- vehicle speed trace around pickup/dropoff;
- door command/state trace;
- passenger state trace;
- meter mode/state and accumulator trace;
- payment/receipt/tip/non-payment event ids;
- expected/observed + pass/fail;
- video/log/telemetry artifact path.

OPEN numerics may be tested only as explicit `EXPERIMENTAL / NON-AUTHORITY` profiles until owner lock.

## Passenger presence

| ID | Required proof |
|---|---|
| FARE-PAX-001 | official-stop passenger/group is physically visible before admission |
| FARE-PAX-002 | curb passenger/group is physically visible before admission |
| FARE-PAX-003 | icon-only/invisible spawn path cannot commit boarding |
| FARE-PAX-004 | decline leaves passenger outside and taxi may continue without mission-fail pause |

## Full-stop exchange

| ID | Required proof |
|---|---|
| FARE-STOP-001 | pickup above approved full-stop threshold is rejected |
| FARE-STOP-002 | pickup at approved full stop can proceed |
| FARE-STOP-003 | drop-off above approved full-stop threshold is rejected |
| FARE-STOP-004 | drop-off at approved full stop can proceed |
| FARE-STOP-005 | no rolling-pickup compatibility path exists in active build |

## Passenger door / body

| ID | Required proof |
|---|---|
| FARE-DOOR-001 | physical in-cabin lever produces one authoritative OPEN command |
| FARE-DOOR-002 | physical lever produces one authoritative CLOSE command |
| FARE-DOOR-003 | closed doors prevent boarding/exiting |
| FARE-DOOR-004 | front+rear passenger doors expose one continuous pillarless right-side aperture |
| FARE-DOOR-005 | no fixed B-pillar collision blocks approved passenger path |
| FARE-DOOR-006 | save/restart cannot create phantom/split door state |

## Group boarding / seating

| ID | Required proof |
|---|---|
| FARE-SEAT-001 | group size 1..3 allocates rear slots only |
| FARE-SEAT-002 | group size 4..5 fills rear first then front slots |
| FARE-SEAT-003 | sixth passenger is rejected/capped according to authored group rule |
| FARE-SEAT-004 | group can board as one compressed simultaneous event after stop/opening |
| FARE-SEAT-005 | passenger state transitions outdoor → boarded/seated exactly once |

## Taximeter

| ID | Required proof |
|---|---|
| FARE-MTR-001 | metered fare changes when only distance changes in controlled harness |
| FARE-MTR-002 | metered fare changes when only elapsed fare time changes |
| FARE-MTR-003 | metered fare changes when both distance and elapsed time change |
| FARE-MTR-004 | repeated START cannot create duplicate meter transaction |
| FARE-MTR-005 | repeated STOP cannot duplicate fare completion/receipt |
| FARE-MTR-006 | save/load during active meter preserves one distance/time accumulator |

## Metered / off-meter separation

| ID | Required proof |
|---|---|
| FARE-OFF-001 | explicit `METERED` fare never silently becomes `OFF_METER` |
| FARE-OFF-002 | explicit `OFF_METER` fare never accumulates hidden official meter charge |
| FARE-OFF-003 | save/restart preserves fare mode exactly once |
| FARE-OFF-004 | curb context can feed distinct tip/risk hooks without an abstract morality-menu transition |

## Payment order / fare evasion

| ID | Required proof |
|---|---|
| FARE-PAY-001 | unpaid passenger with doors closed cannot exit |
| FARE-PAY-002 | doors opened before unresolved payment can trigger one eligible fare-evasion result |
| FARE-PAY-003 | payment committed before door opening prevents ordinary unpaid escape for that fare |
| FARE-PAY-004 | transaction cannot become both paid and unpaid |
| FARE-PAY-005 | reload/retry cannot duplicate payment, receipt, tip, complaint or non-payment event |

## Official stop / curb

| ID | Required proof |
|---|---|
| FARE-CTX-001 | official-stop and curb pickup are distinguishable from world position/context |
| FARE-CTX-002 | both require full stop + physical door opening |
| FARE-CTX-003 | curb path can produce different versioned economic/reputation hooks |
| FARE-CTX-004 | future regulator hook receives explicit context/meter/payment facts rather than inferred random violation |

## Rhythm / route

| ID | Required proof |
|---|---|
| FARE-RHY-001 | job supplies route to generated destination without street-address entry dependency |
| FARE-RHY-002 | route may reroute after deviation without changing destination identity |
| FARE-RHY-003 | proof has no traffic-light dependency |
| FARE-RHY-004 | proof has no systemic standing-jam dependency |
| FARE-RHY-005 | deliberate passenger stop resumes cleanly into moving road flow |

## Mechanical-health integration boundary

The core FARE ZERO gate does not require every future vehicle fault, but if a `CD-722` fault is active during a fare:

- its warning is physical/diegetic;
- handling/audio/VFX symptoms reference the same state;
- save/restart preserves the state according to its owner;
- fare/passenger/payment state does not duplicate because of the fault.

## Pass rule

FARE ZERO is not VERIFIED until an exact build passes the applicable tests with evidence. Documentation alone is not proof.
