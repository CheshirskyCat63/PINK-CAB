# PINK CAB · Tatra / Daughter / Conductor Canon

**Status:** CURRENT IMPLEMENTATION-FACING CANON
**Jira:** `CD-561`, `CD-562`, `CD-536`, `CD-537`, `CD-550`, `CD-579`, resolved decision `CD-695`; passenger load `CD-570` / `CD-667`; magnetic load `CD-592`; physical systems `CD-722`; QA `CD-701`
**Confluence:** pages `6619137`, `5701699`, `5931070`, `8388609`, `5832744`, authority index `6586369`

This file mirrors stable hero-vehicle, crew and physical-load decisions that implementation may not silently reopen.

## 1. Hero Tatra identity

The playable taxi is a bespoke retrofuturist **early / Gen-1 Tatra 603-family** interpretation, not a museum reconstruction.

Locked exterior identity:

- neon-pink hero taxi;
- three-headlamp front identity is mandatory;
- center lamp sits on the vehicle longitudinal centerline, visually between the two outer forward lamps and approximately centered between the front wheels;
- the **center lamp is steering-linked / directional**;
- center beam reads somewhat farther than the two outer primary forward lamps;
- rear/tail region is intentionally stretched relative to the historical reference;
- the entire body must not be uniformly scaled to fake the stretch;
- passenger side has **no fixed middle/B-pillar** between front and rear passenger openings;
- front and rear passenger doors open away from the center, exposing one wide continuous right-side passenger aperture;
- passenger doors are controlled by a physical in-cabin lever/handle.

### Still OPEN under `CD-561` / `CD-619` / `CD-626` / `CD-720`

- final exterior/interior dimensional sheet;
- exact wheelbase vs rear-overhang change;
- center-lamp yaw limit;
- steering-to-lamp transfer curve;
- beam cone/intensity/falloff/range delta;
- final lamp housing geometry;
- passenger-door hinge axis/travel;
- latch/striker/reinforcement/sealing solution;
- passenger-door animation timing and damage interaction;
- final rear-seat/service packaging clearances.

These values may be prototyped only as `EXPERIMENTAL / NON-AUTHORITY` until locked.

## 2. Locked mass / occupant contract

The previous provisional `1470 kg reference calibration mass` is **SUPERSEDED**.

Locked values:

- base Tatra vehicle mass = **1450 kg**;
- fully fueled Tatra vehicle mass = **1550 kg**;
- full-tank fuel contribution to runtime mass = **100 kg**;
- heroine body reference = **1.65 m / 58 kg**;
- daughter/conductor body reference = **1.55 m / 49 kg**;
- female passenger authored physical mass = **55–70 kg**;
- male passenger authored physical mass = **65–90 kg**;
- passenger capacity remains max 5;
- luggage/cargo mass is not implied by this lock.

Authoritative runtime total:

`total_mass = 1450 kg base vehicle + current fuel mass + 58 kg heroine + 49 kg daughter + sum(actual boarded passenger masses)`

Reference fixtures:

- full fuel + heroine + daughter + no passengers = **1657 kg**;
- full fuel + heroine + daughter + five 90 kg passengers = **2107 kg theoretical declared maximum passenger-load fixture**.

Passenger mass is identity/state data. It must remain deterministic across stream-out, save/reload and repeat-client reconstruction and may not reroll merely because the NPC is streamed back in.

## 3. Physical electronics boundary

The physical Tatra has:

- **NO ABS**;
- **NO ESP**.

No ABS pulsing, ESP indicator or individual-wheel stability intervention may be presented as an onboard Tatra system.

Existing bounded C+ game-level yaw damping may remain only as non-diegetic handling support under `CD-562`. It is not ESP and may not auto-countersteer, auto-throttle, auto-brake or silently pulse individual wheel brakes.

## 4. Mass-sensitive handling direction

Increasing authoritative total mass must be noticeable but restrained. The Tatra remains one recognizable handling model rather than changing vehicle class by seat count.

As load rises:

- acceleration becomes modestly worse;
- braking becomes modestly worse;
- body roll and pitch increase;
- weight transfer settles more slowly;
- direction changes feel heavier;
- at comparable high-speed wallride/fence entry conditions, the heavier vehicle is somewhat less prone to abrupt separation from the contact surface.

The response must be smooth with actual mass. Exact curves, CG movement and final benchmark load fixtures remain OPEN under `CD-562` / `CD-592`.

## 5. Suspension / body feedback doctrine

Load and road feel are not communicated only through wheel travel. The player should perceive the Tatra through a cheap coherent arcade-sim combination of:

- visible suspension/body compression and extension;
- roll and pitch;
- chassis/cabin shake;
- trim vibration/rattle where appropriate;
- small bounded vehicle/camera vibration where comfortable;
- steering/body tremor on rough surfaces;
- synchronized audio/VFX cues tied to the same physical state.

This is deliberate PS2-style physical expression, not a requirement for expensive structural vibration simulation.

## 6. Level 1 magnetic load consequence

The old global statement `residual magnetism = 5.0 seconds at every load` is **SUPERSEDED**.

Locked endpoints:

- at **1657 kg** reference service load: residual magnetic window = **5.0 s**;
- at **2107 kg** maximum declared passenger-load fixture: residual magnetic window = **4.0 s**;
- legal lighter-than-reference states do not extend the window beyond 5.0 s;
- between 1657 and 2107 kg, timeout decreases smoothly/monotonically with actual total mass;
- exact interpolation curve remains OPEN under `CD-592`.

See `docs/PINK_CAB_VEHICLE_MASS_LOAD_DYNAMICS.md` and `docs/PINK_CAB_VERTICAL_TRANSIT_PARKOUR.md`.

## 7. Center-lamp gameplay boundary

The directional headlamp is a visual/vehicle system, not route guidance or target auto-aim.

Required behavior:

- steering direction changes center-lamp orientation;
- its visible beam response moves with the lamp;
- it remains physically centered and visually distinct from the two outer lamps;
- it may have a subtle mechanical/servo sound if the final design supports it;
- it participates in Level 1 PS2 lighting/presentation evidence.

Acceptance owner: `CD-719` plus Tatra/cabin owners.

## 8. Pillarless passenger-door boundary

The passenger opening is part of hero-car identity, not a generic taxi animation.

Locked:

- no fixed B-pillar divides front/rear passenger access;
- front and rear passenger doors open opposed/away from the aperture center;
- both open doors create one continuous right-side opening;
- heroine physically actuates the passenger-door lever;
- closed state prevents passenger boarding/exiting;
- no HUD-only passenger-door command is authoritative;
- pickup and normal drop-off require full stop under `CD-720` / `CD-522`;
- boarding/exiting adds/removes each passenger's exact physical mass once.

Exact structural/animation numerics remain OPEN.

## 9. Daughter = conductor = later onboard operator

There is **one continuous character identity**:

`17-year-old daughter → physical conductor/service role → later onboard operator`

There is no third permanent conductor NPC.

Physical direction:

- daughter reference body is **1.55 m / 49 kg**;
- heroine/driver reference body is **1.65 m / 58 kg**;
- daughter occupies the rear/service area;
- she sits sideways relative to travel;
- her back is toward the right-side body panel;
- passenger/service packaging must support her work surface without creating a third permanent crew seat.

Exact seat/socket/clearance/passenger-capacity consequences remain implementation locks.

## 10. Conductor/service-stage duties

Where owned by the relevant fare/service contract, she may support:

- fare/ticket/payment workflow;
- handing/printing receipt/check;
- route lookup;
- engine/service-state monitoring;
- onboard-electronics monitoring/support;
- approved physical rear-service interactions.

## 11. Later operator duties

The later operator layer may add:

- traffic-density comparison;
- camera/enforcement warnings;
- toll/access state;
- road closures;
- fuel/service/parts/parking search;
- Neural/repeat-client message work;
- bounded digital bureaucracy / verification tasks.

`OperatorTask` data ownership remains `CD-579`.

## 12. Physical taxi-system doctrine

When an adopted real taxi/car function, gauge, warning lamp, switch, lever, button or indicator matters to gameplay, preferred authority is a physical/diegetic vehicle state rather than a floating abstract icon.

Initial explicit `CD-722` mechanical-health families include:

- engine/coolant overheating;
- brake-fluid / brake-system warning or degradation;
- physical gauges/telltales/warning lamps;
- measurable engine/braking/thermal symptoms;
- audio/handling/VFX cues tied to the same state;
- daughter/conductor service-awareness where appropriate;
- explicit repair/service recovery.

Load/suspension feedback follows the same rule: body motion and vibration are tied to actual physics/load state, not random cosmetic shake or an abstract `+weight` stat.

This is arcade-sim coverage, not a requirement for engineering-depth simulation of every vehicle fluid/component.

## 13. Hard authority limit

The daughter/conductor/operator may inform, prepare and perform approved service/digital work. She must never:

- steer;
- brake;
- operate throttle;
- choose live traffic gaps;
- silently commit the driver to a route;
- become hidden autopilot.

## 14. Current quick-hand / cockpit interaction authority

Canonical high-level grammar:

- default mouse = steering;
- hold `Space` = gaze/free-look;
- `1–4 = START`: reach/hover the saved physical target while mouse can still steer until manipulation ownership changes;
- `LMB = ATTENTION`: keep the right hand physically on the targeted control;
- `RMB = GO`: actively manipulate; RMB takes mouse away from steering;
- release returns according to the current interaction state.

Current quick targets:

- `1 = turn signals`;
- `2 = horn/signal`;
- `3 = gearbox`;
- `4 = handbrake`.

Older Git/Confluence wording that says `RMB=passive grab`, `LMB=manipulate`, or `1/2 OPEN` is superseded.

## 15. Verification IDs

At minimum the verification matrix must carry:

- `PC-T-LAMP-001` — center lamp exists in correct center position and tracks steering;
- `PC-T-LAMP-002` — center beam reads farther than outer pair using current locked/experimental numeric profile;
- `PC-T-MASS-001` — runtime total mass equals exact base + current fuel + crew + boarded-passenger masses;
- `PC-T-MASS-002` — passenger mass survives save/reload/stream reconstruction without reroll;
- `PC-T-ELEC-001` — no ABS/ESP vehicle system or hidden wheel-brake intervention exists;
- `PC-T-LOAD-001` — increased load modestly degrades acceleration/braking and increases roll/pitch/settling without discontinuity;
- `PC-T-WALL-LOAD-001` — matched heavier fixture is not more prone to abrupt wallride separation than reference and follows the locked stability direction within calibrated tolerance;
- `PC-T-MAG-MASS-001` — 1657 kg endpoint honors 5.0 s boundary;
- `PC-T-MAG-MASS-002` — 2107 kg endpoint honors 4.0 s boundary;
- `PC-T-DOOR-001` — passenger side exposes pillarless continuous aperture with opposed door motion;
- `PC-T-DOOR-002` — physical door lever owns authoritative open/closed state;
- `PC-T-DAU-001` — one physical daughter/conductor identity is present; no third permanent conductor path;
- `PC-T-DAU-002` — rear-seat/service placement matches approved cabin sheet;
- `PC-T-SYS-001` — adopted mechanical-health warning is physical/diegetic and maps to the same subsystem state;
- `PC-T-OP-003` — operator authority never acquires driving control.

## 16. Maturity

Mass figures, crew body references, passenger mass ranges, no-ABS/no-ESP identity, mass-sensitive handling direction and magnetic endpoint behavior are locked at canon/spec level.

Exact response curves, CG shifts, vibration calibration and magnetic interpolation remain OPEN. Nothing in this file is **IMPLEMENTED or VERIFIED** until executable evidence passes `CD-701` and related tests.
