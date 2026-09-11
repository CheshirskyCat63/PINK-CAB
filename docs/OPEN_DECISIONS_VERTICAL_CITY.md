# PINK CAB · Vertical City Open Decisions Register

**Status:** CURRENT FIRST-EURO L1/L2 SUBREGISTER
**Master decision program:** Jira `CD-588`
**BASE-100 world owner:** `CD-751`
**FIRST EURO scope:** `CD-753`
**Confluence master register:** `5832744`
**Vertical canon:** `docs/PINK_CAB_VERTICAL_TRANSIT_PARKOUR.md` / Confluence `8388609`

This file is an implementation-facing subregister. It does not replace `docs/OPEN_DECISIONS.md`.

## Locked structure safe to implement

- FIRST EURO includes full Level 1 + Level 2 gameplay/runtime.
- Level 3 gameplay is POST-FIRST-EURO; only a compatible route/schema extension boundary is required now.
- Level 1 owns exactly five upper freight-ceiling lanes.
- Freight-ceiling traffic always moves opposite the lower Level 1 road direction.
- Level 1 wallride uses seven gameplay bands; the Tatra footprint is approximately three bands.
- The fast direct cross-carriageway side-swap is available only from the Level 1 ceiling through approved poplar-gap modules.
- Level 2 owns suspended magnetic bus flow; its exact bus-lane count is not assumed to be two.
- Metro is below the Level 2 bus layer, longitudinally parallel, with two tracks/lines per side; trains are visible from Level 1.
- Taxi-on-metro driving is not canonical by default.
- Shared magnetic body-language direction remains clockwise ascent + clockwise descent = one longitudinal 360-degree cycle where that transition grammar applies.
- Level 1 residual magnet timeout is load-sensitive and linear: 5.0 s @1657 kg → 4.0 s @2107 kg; legal lighter states cap at 5.0 s.

## Current decision rows

| ID | Area | Owners | State | Locked now | Still required for FIRST EURO |
| --- | --- | --- | --- | --- | --- |
| `OD-T1-CEILING-001` | L1 freight ceiling / cross-road jump | `CD-676`, `CD-589`, `CD-592`, `CD-701`, traffic `CD-567` | PARTIAL | 5 lanes; opposite flow; player auto-parkour; ceiling-only fast side-swap; 7 wallride bands; load-sensitive residual function | implementation geometry/clearances, entry/exit envelope, valid poplar aperture/landing geometry, force/contact/reacquisition calibration, freight speed/headway/lane-change profile, recovery boundaries |
| `OD-T2-BUS-001` | L2 suspended bus flow | `CD-572`, `CD-677`, `CD-705`, `CD-592` | PARTIAL | bus layer belongs to L2; player magnetic traversal exists; lane count is not fixed to two | final lane count, lane width/elevation, speed/headway, transfer/contact envelope, station-zone restrictions, failure/cleanup profile |
| `OD-T2-METRO-001` | L2 metro + stations | `CD-677`, `CD-706`, `CD-707`, `CD-589` | PARTIAL | metro below buses; parallel to road; 2 tracks per side; repeatable stations; visible from L1; no taxi-on-metro default | elevation/clearance, station module dimensions, train speed/headway/dwell/phase rule, sightline/performance bounds, exact station gameplay function where code-facing |
| `OD-T3-SUSPENDED-001` | L3 suspended gameplay | future owner | POST-FIRST-EURO | uniqueness/extension compatibility only | no FIRST EURO gameplay decision required; entire L3 suspended concept remains future |

## Production rule

Do not guess unresolved owner decisions into canonical production data. Low-level plugin/contact values already classified as CALIBRATION are tuned inside their named acceptance envelopes and are not repeatedly returned to owner intake.

Temporary prototype values are allowed only as `EXPERIMENTAL / NON-AUTHORITY` with an explicit replacement owner/decision ID.

## Closure rule

A FIRST EURO row is specification-closed only when Jira + durable Confluence + Git agree on observable behavior/ownership and remaining low-level tuning is classified CALIBRATION. IMPLEMENTED/VERIFIED still requires exact executable evidence.

`LOCKED/SPECIFIED != IMPLEMENTED != VERIFIED`.
