# PINK CAB QA · Right-Entry Pipe Interchange Zero

Jira proof owner: `CD-704`; gate `CD-681`; delivery `CD-680`.

Authority: Confluence page `8421388`; Git mirror `docs/PINK_CAB_PIPE_INTERCHANGE_BIBLE.md`.

## Evidence header

Every run records:

- exact commit/build id;
- generator/content version;
- CityCode;
- TrafficSeed;
- interchange template/version;
- graph dump/checksum;
- node/link/branch/tube ids;
- player speed/yaw/slip state;
- nearest traffic ids;
- clearance report;
- pass/fail + expected/observed;
- video/log/telemetry path.

OPEN geometry values may not be embedded as production expected values until `OD-INTERCHANGE-PIPE-001` is locked.

## Acceptance matrix

| ID | Area | Required proof | Authority state |
| --- | --- | --- | --- |
| INT-RIGHT-001 | Entry direction A | ordinary entrance begins from right side | LOCKED DIRECTION |
| INT-RIGHT-002 | Entry direction B | mirrored opposite travel also enters from right | LOCKED DIRECTION |
| INT-APP-001 | Deceleration | approach has valid right-side decel/entry envelope | geometry OPEN |
| INT-APP-002 | Merge return | return branch has valid accel/merge envelope | geometry OPEN |
| INT-APP-003 | Exclusions | parking/service/vignette collision absent from mandatory approach | LOCKED DIRECTION |
| INT-GRAPH-001 | Forward | through/forward route exists | LOCKED DIRECTION |
| INT-GRAPH-002 | Reversal | complete longitudinal reversal route exists where selected family supports it | LOCKED DIRECTION |
| INT-GRAPH-003 | Loop | closed loop/circle path works | LOCKED DIRECTION |
| INT-GRAPH-004 | Oval | sustained oval/circuit path works | LOCKED DIRECTION |
| INT-GRAPH-005 | Figure eight | selected figure-eight variant enumerates valid route | LOCKED DIRECTION |
| INT-GRAPH-006 | Tier transfer | selected tier-transfer path enumerates valid links | LOCKED DIRECTION |
| INT-GRAPH-007 | No dead ends | no accidental terminal link | LOCKED DIRECTION |
| INT-PIPE-001 | Two lanes | core resolves to two one-way lanes | LOCKED DIRECTION |
| INT-PIPE-002 | Geometry continuity | banking/grade/spiral has no collision seam | numerics OPEN |
| INT-PIPE-003 | Grip pass | one clean full route completes | handling direction LOCKED, geometry OPEN |
| INT-PIPE-004 | Sustained drift | one full sustained drift route completes without separate drift mode | handling direction LOCKED, geometry OPEN |
| INT-PIPE-005 | Recovery | over-rotation/contact can recover or fail deterministically | recovery numerics OPEN |
| INT-TRAF-001 | Sparse traffic | pipe remains populated by non-zero traffic | LOCKED DIRECTION; density OPEN |
| INT-TRAF-002 | Overtake | one readable traffic overtake gap exists in deterministic seed | density/headway OPEN |
| INT-TRAF-003 | Drift gap | one drift pass through live moving gaps completes without mandatory collision | density/headway OPEN |
| INT-TRAF-004 | Restart ownership | crash/restart restores player + AI to valid lane/link ownership | LOCKED DIRECTION |
| INT-BLD-001 | Building penetration | tube passes through approved panel-megablock volume | LOCKED DIRECTION |
| INT-BLD-002 | Balcony clearance | balcony/fire-stair/service props remain outside legal envelope | clearance OPEN |
| INT-BLD-003 | Vignette budget | building route shows bounded deterministic life-theatre slots | budget OPEN `OD-INTERCHANGE-VIGNETTE-001` |
| INT-BLD-004 | Streaming | no major near-miss pop-in/collision seam inside decision envelope | numeric LOD/streaming OPEN |
| INT-RULE-001 | Core camera boundary | no fixed enforcement camera in core drift tube | CURRENT LOCKED DIRECTION |
| INT-RULE-002 | Approach rules | ordinary approach signs/cameras remain legal/machine-readable | PARTIAL |
| INT-DMG-001 | Collision handoff | collision hands off to normal vehicle/property consequence owner | depth BLOCKED by `CD-600` |
| INT-REV-001 | Reverse CityCode | same static graph family reconstructs with correct mirrored legality | LOCKED DIRECTION |

## Required graph enumeration

For the selected Interchange Zero template, export and archive:

- every legal entry node;
- every exit node;
- direction legality;
- forward continuation path;
- reversal path;
- loop/circuit path;
- sustained oval/circuit path;
- figure-eight links if selected;
- tier-transfer links if selected;
- branch/tube ids;
- recovery markers;
- checksum/hash for static graph identity.

Fail if any generated ordinary route requires an unintended left-side entrance, creates an accidental dead end, or reverses direction legality incorrectly on CityCode reconstruction.

## Building-penetration clearance report

For each near-miss slot record minimum legal clearance to:

- balcony frame;
- fire stair;
- structural column/support;
- service gallery;
- utility prop;
- vignette actor/prop;
- sign/light fixture.

The expected values remain linked to `OD-INTERCHANGE-PIPE-001` until locked.

## Grip/drift traffic procedure

Deterministic seeds required:

- `INT_LIGHT`;
- `INT_NOMINAL`;
- `INT_HEAVY`;
- `INT_DRIFT_TEST`.

Each seed must support:

1. clean grip pass;
2. sustained drift pass;
3. traffic overtake;
4. one controlled collision/recovery/restart case.

No seed may require hidden AI teleporting or scripted traffic disappearance to pass.

## Verification rule

`CD-681` cannot be VERIFIED until the exact executable evidence record contains build, CityCode, graph checksum, traffic seed, clearance report, telemetry and video path.
