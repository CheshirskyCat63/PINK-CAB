# PINK CAB · Vertical Transit & Auto-Parkour Authority

Status: CURRENT FIRST-EURO L1+L2 IMPLEMENTATION-FACING CANON
Product root: `CD-519`
Confluence authority: `8388609`
Vehicle technology: Native Chaos `CD-785..792` / `docs/PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md`
Vertical owners: `CD-523`, `CD-547`, `CD-676`, `CD-572/CD-677`, `CD-589`, `CD-592`; QA `CD-701/CD-738`

## Authority correction

Tier1 = seven-band chain-link wallride + five-lane opposite-flow freight ceiling + ceiling/poplar side swap. Tier2 = suspended bus flow + metro + stations. **Tier3 gameplay is POST-FIRST-EURO**; year one preserves only a compatible tier/route/schema extension boundary. Old generic two-magnetic-lanes-per-tier and any FIRST-EURO Tier3 implementation requirement are superseded.

## Vehicle physics boundary

**Native Unreal Chaos Vehicles behind `IPinkCabVehicleDynamicsProvider` is the sole hero-Tatra road/tire/suspension/drivetrain solver.** Vertical systems may apply explicit magnetic/contact constraints under `CD-592`, but may not instantiate a second hidden vehicle solver or call those constraints ABS/ESP.

Tatra remains rear-engine/RWD, 1450 kg base / 1550 kg full fuel, no ABS/ESP, E34-like control/Tatra-expression contract.

## Load fixtures

Heroine 58 kg, daughter 49 kg, female passenger 55–70 kg, male 65–90 kg actual authored mass. Standard benchmark **1657 kg** full-fuel crew-only; declared max **2107 kg**. Passenger mass persists and cannot reroll.

## Tier1 wallride

Curved chain-link uses **7 longitudinal bands**, Tatra roughly 3. More load increases roll/pitch/settling and modestly worsens accel/braking, but at matched wallride entry gives only a **modest single-digit-percent direction** of improved abrupt-separation resistance. Exact contact-force calibration remains `CD-592/CD-701` work.

## Tier1 freight ceiling

Exactly **five** upper freight lanes, always opposite lower-road flow. Freight actors are moving traffic/geometry; single units and 3–5-unit convoys valid. Exact speed/headway/lane-change/dock values remain open/calibration.

## Tier1 residual magnetism — LOCKED LINEAR FORMULA

The old `5.0 seconds at every load` rule is SUPERSEDED.

- **1657 kg → 5.0 s**;
- **2107 kg → 4.0 s**;
- legal lighter-than-reference states cap at 5.0 s;
- **between endpoints timeout is linear by authoritative total mass**;
- in-range formula: `timeout_s = 5.0 - (total_mass_kg - 1657) / 450`;
- reacquisition before expiry returns the vehicle to valid/stable contact;
- expiry enters ordinary detach/failure.

The interpolation is no longer OPEN. **The exact residual-budget reset/recharge behavior after reacquisition remains owner question `M05` in Technical Owner Pack 01 and must not be silently invented.** Magnetic force/damping/contact angle, strip/gap geometry and reacquisition/failure tolerances remain calibration under `CD-592/CD-701`.

## Tier1 cross-carriageway rule

Only fast direct side swap uses wallride/ceiling and an approved giant-poplar window. Static CityCode geometry preserves approach/aperture/landing; dynamic traffic/load may still make it dangerous.

## Tier2 bus / metro / stations

Suspended bus lane count remains OPEN/CALIBRATION under `CD-705` and the BASE-100 owner pack. Current authoritative Chaos/PINK-CAB vehicle/load state carries into Tier2; Tier2 magnetic/contact values must not silently copy Tier1.

Metro has **two tracks per side**, longitudinal and visible from Level1. Exact elevation, directions, speed/headway/dwell/counter-phase and station geometry remain `CD-706/CD-707` work. Direct Tatra-on-metro driving is non-canon unless separately locked.

## Shared body language

Where approved, ascent rolls clockwise and descent continues clockwise; a full ascent+descent may create one longitudinal 360° rotation. Stable/partial/transfer/descent/abort states remain tier-owned.

## Tier3 — POST-FIRST-EURO

`OD-T3-SUSPENDED-001` is **POST-FIRST-EURO**, not a FIRST-EURO owner blocker. Year one requires only extension compatibility. No Level3 suspended gameplay concept, traffic profile, service gameplay or acceptance test is required for START-90/BASE-100/FIRST EURO. A future Tier3 design may not silently clone Tier1 or Tier2.

## Persistence

CityCode+version reconstructs Tier1 wallride/ceiling sequence, poplar windows, Tier2 bus topology and metro/station sequence. FIRST EURO schema may expose compatible future-tier fields, but it does not generate or persist canonical Tier3 gameplay topology yet. Passenger mass remains stable because it affects authoritative vehicle dynamics and residual timeout.

## Gameplay integration

Vertical traversal feeds ordinary taxi systems: fare time/route efficiency, passenger preferences, reputation/enforcement, damage/repair, daughter/operator warnings and Neural memory. It is not a separate trick-score mode.

## Verification

`CD-701` / `docs/qa/VERTICAL_TRANSIT_TESTS.md` must prove 1657, 2107 and intermediate **linear** timeout fixtures, lighter cap, wallride load direction and clean recovery. Reacquisition tests use the final `M05` owner lock once resolved. Integrated vehicle-stack proof follows current CD-848 recovery/human-gate evidence. Runtime remains NOT VERIFIED until exact-build evidence exists.
