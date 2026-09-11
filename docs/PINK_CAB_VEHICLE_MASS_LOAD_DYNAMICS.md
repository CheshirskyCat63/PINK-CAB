# PINK CAB · Vehicle Mass, Occupant Load & Dynamic Response

**Status:** CURRENT IMPLEMENTATION-FACING SPEC
**Jira:** physics `CD-562`; vehicle stack `CD-730`; FGear profile `CD-732`; handling `CD-729`; magnetic/contact `CD-592`; passenger mass `CD-570/CD-667`; QA `CD-701/CD-738`
**Technology companion:** `docs/PINK_CAB_VEHICLE_TECH_STACK_FGEAR_VDS.md`

This file owns mass/load composition and observable consequences. **FGear Vehicle Physics** is the sole road-dynamics implementation owner for these consequences.

## Locked mass inputs

| Item | Locked value |
| --- | ---: |
| Base Tatra vehicle | **1450 kg** |
| Fully fueled Tatra vehicle | **1550 kg** |
| Full-tank fuel contribution | **100 kg** |
| Heroine | **58 kg / 1.65 m** |
| Daughter / conductor | **49 kg / 1.55 m** |
| Female passenger | **55–70 kg** |
| Male passenger | **65–90 kg** |
| Maximum passengers | **5** |

Runtime: `total_mass_kg = 1450 + current_fuel_mass_kg + 58 + 49 + Σ(boarded_passenger_mass_kg)`.

Passenger mass is stable identity/state and may not reroll. Luggage/cargo is not part of the current lock.

Reference fixtures: **1657 kg** full fuel + heroine + daughter + no passengers; **2107 kg** full fuel + heroine + daughter + five 90 kg passengers.

## Packaging / balance reference

Target static balance at 1657 kg = **45/55 F/R**, with only a small correction permitted when final 3D packaging is measured. Fuel and passenger sockets must physically shift CG once implemented. Historical chassis baseline for initial setup is ~2750 mm wheelbase / ~1485 mm front track / ~1400 mm rear track.

## Mass-sensitive FGear behavior

Increasing total mass remains one continuous FGear profile family:

- acceleration modestly worsens;
- braking modestly worsens;
- roll/pitch/settling increase;
- direction changes feel heavier;
- Tatra Expression intensity may increase coherently;
- matched wallride/fence abrupt separation becomes only modestly less likely.

Acceleration/braking response should be smooth/near-linear. Roll/settling expression may become mildly stronger toward 1900–2107 kg. Exact low-level FGear constants are **CALIBRATION**, not owner-open questions.

## Electronics

Physical Tatra has **NO ABS / NO ESP**. Any bounded C+ yaw edge guard is non-diegetic, separately telemetered, and may not auto-countersteer, auto-throttle, auto-brake or perform ESP individual-wheel braking.

## Physical vs expression feedback

FGear owns physical suspension/tire state. Tatra Expression Layer may amplify presentation without changing force:

`FGEAR SUSPENSION/LOAD → PHYSICAL CHASSIS → EXPRESSION ROLL/PITCH/HEAVE/SHAKE → STEERING/CABIN/CAMERA → AUDIO/VFX`

Reference expression seeds: physical roll ~4.5–5.5° @~0.8g; visible roll ~1.35×; pitch ~1.25×; heave ~1.20×; visual settle ~0.9–1.2 s. These are calibration values inside the locked owner intent.

## Wallride / fence

At matched approach geometry/speed/contact, heavier load gives only a **modest single-digit-percent direction** of improved abrupt-separation resistance. It never selects a different easy-mode physics preset. Exact force/contact curve remains `CD-592/CD-701` calibration.

## Level1 residual magnetism — LOCKED LINEAR FORMULA

| Total mass | Residual window |
| --- | ---: |
| **1657 kg** | **5.0 s** |
| **2107 kg** | **4.0 s** |

Rules:

- legal lighter-than-reference states cap at 5.0 s;
- between 1657 and 2107 kg, timeout is **linear by authoritative total mass**;
- equivalent implementation formula for in-range mass `m`:
  `timeout_s = 5.0 - (m - 1657) / 450`;
- clamp upper timeout to 5.0 s for legal lighter states;
- reacquisition before expiry restores stable contact;
- expiry enters normal detach/failure.

The old fixed-five-second rule and old `interpolation OPEN` wording are both **SUPERSEDED**. Magnetic force/damping/contact/reacquisition remain calibration under `CD-592/CD-701`.

## Persistence

Persist current fuel, crew presence, passenger identities/masses/seats, deterministic total-mass inputs, and version IDs for FGear mass-response/magnetic profiles. Save/reload must not duplicate or reroll mass.

## Remaining calibration, not owner questions

- exact CG XYZ from final packaging;
- low-level FGear mass→accel/brake/roll/pitch/yaw values;
- exact wallride contact-force curve within the modest advantage direction;
- final expression vibration frequencies/amplitudes and camera accessibility;
- any later luggage/cargo system requires a new decision.

The interpolation formula and 1657 benchmark fixture are **closed choices**.

## Verification

`CD-701` proves mass/vertical behavior; `CD-738` proves integrated FGear stack. Required fixtures include 1657 kg, 2107 kg and at least one intermediate mass that matches the linear timeout formula, plus no-ABS/ESP, load trends, wallride direction and physical-vs-expression telemetry separation.

Runtime remains **NOT IMPLEMENTED / NOT VERIFIED** until executable evidence exists.
