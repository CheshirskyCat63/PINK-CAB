# PINK CAB · Vehicle Mass / Load Acceptance Contract

**Status:** QA SPECIFIED — RUNTIME NOT VERIFIED
**Spec:** `docs/PINK_CAB_VEHICLE_MASS_LOAD_DYNAMICS.md`
**Vehicle stack:** `docs/PINK_CAB_VEHICLE_TECH_STACK_FGEAR_VDS.md`, Jira `CD-730/CD-732/CD-738`
**Vertical QA:** `docs/qa/VERTICAL_TRANSIT_TESTS.md`
**Jira QA owner:** `CD-701`
**Related:** `CD-562`, `CD-592`, `CD-667`, `CD-570`, `CD-722`

Every VERIFIED result identifies exact build/commit, FGear profile/version, content/schema version and deterministic fixture.

## Locked fixture data

- base Tatra: **1450 kg**;
- fully fueled Tatra: **1550 kg**;
- heroine: **58 kg**;
- daughter: **49 kg**;
- female passenger: **55–70 kg**;
- male passenger: **65–90 kg**;
- reference service load: **1657 kg**;
- maximum declared passenger load: **2107 kg**;
- ABS: **absent**;
- ESP: **absent**.

## PC-T-MASS-001 · Exact total mass

Build deterministic fixtures with known fuel and passenger identities.

Pass:

`runtime_total = 1450 + current_fuel + 58 + 49 + sum(actual boarded passenger masses)`

within final numeric precision.

Reject hidden universal passenger averages, duplicate passenger mass, missing boarding mass, or fuel quantity that does not affect physical mass.

## PC-T-MASS-002 · Passenger mass range and identity stability

Pass:

- female fixture remains 55–70 kg;
- male fixture remains 65–90 kg;
- same passenger identity reconstructs with same mass after stream/save/reload;
- boarding/exiting changes total mass exactly once.

## PC-T-MASS-003 · Reference and max fixtures

Verify:

- **1657 kg** = 1550 + 58 + 49;
- **2107 kg** = 1550 + 58 + 49 + 5×90.

Any unowned luggage/cargo makes fixture invalid.

## PC-T-ELEC-001 · No ABS

Controlled hard-brake test.

Pass: no physical ABS system, no ABS-style pulsing and no ABS warning/control in authoritative cockpit data.

## PC-T-ELEC-002 · No ESP

Controlled yaw/spin test.

Pass: no ESP individual-wheel braking, no ESP equipment/control, deliberate spin remains possible, and any C+ yaw damping is separately logged as non-diegetic assistance rather than ESP.

## PC-T-LOAD-001 · Acceleration trend

Repeat identical FGear acceleration harness across increasing deterministic load fixtures.

Pass direction:

- increasing mass produces modest monotonic degradation;
- no seat-count discontinuity;
- one recognizable Tatra profile remains.

Exact response curve is **FGear CALIBRATION**, not an owner-open question.

## PC-T-LOAD-002 · Braking trend

Repeat identical braking harness across increasing load fixtures.

Pass direction:

- increasing mass modestly worsens braking/deceleration;
- no ABS behavior;
- one continuous FGear braking model.

Exact response curve is calibration inside `CD-732/CD-738` acceptance.

## PC-T-LOAD-003 · Roll / pitch / settling trend

Use matched cornering, braking and transition inputs.

Pass:

- heavier fixture produces more physical/expressed roll-pitch and/or longer settling according to calibrated model;
- no binary loaded/unloaded animation preset is sole cause;
- physical telemetry and visible expression are separately captured and coherent.

## PC-T-LOAD-004 · Suspension/chassis feedback coherence

Pass:

- shake/vibration/trim/audio response is tied to actual FGear suspension/load/road state;
- higher load produces intended heavier bodily feel;
- no random cosmetic shake contradicts physical state;
- comfort/accessibility envelope is respected.

## PC-T-WALL-LOAD-001 · Heavier wallride contact direction

At matched wallride geometry, speed, yaw and band state compare 1657 kg with heavier fixtures.

Pass direction:

- heavier fixture is not more prone to abrupt separation than reference within calibration tolerance;
- response shows locked modest/single-digit-percent direction of greater contact retention;
- roll/pitch/body-motion penalty remains;
- no hidden autopilot/infinite adhesion.

Exact force/contact constants are `CD-592` calibration.

## Locked Level1 residual-magnet function

For legal total mass `m`:

- if `m <= 1657 kg`, timeout = **5.0 s**;
- if `1657 < m < 2107 kg`, `timeout_s = 5.0 - (m - 1657) / 450`;
- if `m = 2107 kg`, timeout = **4.0 s**.

The interpolation is **LOCKED LINEAR**, not experimental/open. Values above the currently declared max fixture require a future explicit extension rule and may not silently extrapolate in production.

## PC-T-MAG-MASS-001 · 1657 kg residual endpoint

At **1657 kg**:

- 4.9 s gap/reacquisition succeeds;
- 5.0 s boundary follows final inclusive/exclusive timer convention;
- >5.0 s produces detach/failure.

Effective endpoint = **5.0 s**.

## PC-T-MAG-MASS-002 · 2107 kg residual endpoint

At **2107 kg**:

- 3.9 s gap/reacquisition succeeds;
- 4.0 s boundary follows final timer convention;
- >4.0 s produces detach/failure.

Effective endpoint = **4.0 s**.

## PC-T-MAG-MASS-003 · Linear intermediate interpolation

Use at least three deterministic masses strictly between 1657 and 2107 kg, including **1882 kg** midpoint.

Expected midpoint:

`timeout(1882) = 5.0 - (1882 - 1657) / 450 = 4.5 s`.

Pass:

- every intermediate timeout matches the versioned linear formula within timer precision;
- 1882 kg resolves to 4.5 s;
- same mass/version reproduces same timeout;
- increasing mass never increases timeout;
- there is no nonlinear hidden curve/preset by passenger count.

## PC-T-MAG-MASS-004 · Lighter-than-reference cap

Use legal low-fuel state below 1657 kg.

Pass: timeout does not exceed **5.0 s**.

## PC-T-MAG-MASS-005 · Above-declared-max guard

Attempt an artificial/debug fixture above 2107 kg.

Pass: production logic does not silently extrapolate an unowned timeout. Fixture is rejected/clamped according to the final guard implementation and logged as outside current declared passenger-load authority.

## PC-T-MASS-SAVE-001 · Save/reload exactness

Save/reload with boarded passengers and non-full fuel.

Pass:

- fuel mass reconstructs;
- every passenger mass reconstructs;
- seats reconstruct;
- total mass is identical;
- derived linear residual timeout is identical;
- no mass contribution duplicates.

## Required evidence fields

Record:

- exact build/commit/content/schema version;
- FGear version + Tatra profile version;
- CityCode and route/module id for vertical tests;
- fuel quantity/mass;
- heroine/daughter presence/mass;
- passenger id, class, seat and mass;
- total mass;
- expected timeout from locked formula and observed timeout;
- speed/acceleration/deceleration;
- steering/yaw/slip;
- physical roll/pitch/settling and expression channels separately;
- suspension/contact state;
- gameplay-assist term separately from physical vehicle systems;
- reacquisition/detach reason;
- evidence artifact path.

## Maturity rule

Passing documentation review means **SPECIFIED**, not VERIFIED. `CD-701` may mark runtime behavior VERIFIED only with executable evidence from exact tested build and versioned FGear/contact profiles.
