# PINK CAB · Level 1 Corridor World Bible

Implementation-facing mirror of Confluence `8388630`.
Delivery/proof: `CD-678/CD-679`; vehicle technology `CD-730`; handling `CD-729`; magnetic/contact `CD-592`; QA `CD-700..703` + vehicle stack `CD-738`.

## Authority rule

Level1 world/gameplay consumes the authoritative hero-Tatra state from **FGear Vehicle Physics**. It may add explicit vertical magnetic/contact constraints but may not instantiate a parallel tire/suspension/drivetrain solver. Presentation uses the Tatra Expression Layer and cannot independently change physical grip/collision.

## Playable boundary / road flow

Level1 is a bounded longitudinal corridor with five express lanes, right-side accel/decel exits, local/access edge, micromobility/pedestrian/service strips, residential/civic frontage, giant-poplar center ribbon, debris transition, seven-band wallride, five-lane freight ceiling, poplar side-swap windows and bounded ServiceNodes.

No traffic lights/red-green phases. No systemic standing traffic-jam state. Dense traffic can compress/slow/merge around incidents while retaining moving bypass. Lower baseline traffic = passenger cars + motorcycles/sidecars; buses belong Level2 and ordinary heavy freight belongs overhead.

## Hero Tatra / road behavior

FGear profile uses page `10977288` / `CD-729`: E34-like calm/progressive/readable control with rear-engine/rear-heavy old-Tatra mass and visible body drama. No ABS/ESP. Current 180 hp / 240 Nm and 195 km/h target remain.

Wet behavior: rapid lane change above ~160 km/h under maintained/excess throttle can progressively saturate rear combined grip and escalate to spin; easing excessive throttle restores rear lateral reserve and allows player-steering recovery. No scripted speed-threshold drift.

Speed presentation: <=100 comfortable/lively; 100–140 confident; 140–170 visibly busier/floatier; 170–195 power remains available while chassis/suspension/road workload rises.

## Road-surface / expression chain

Ruts, patches, cracks and water feed:

`SURFACE → FGEAR WHEEL/SUSPENSION → PHYSICAL CHASSIS → STEERING TREMOR → TATRA EXPRESSION/CABIN/CAMERA → AUDIO/VFX`

Expression may amplify visual roll/pitch/heave/shake but cannot alter FGear tire force. Ruts give moderate physical steering influence plus stronger visual tremor. Large potholes may physically perturb wheel load/yaw; ordinary defects should not inject random steering.

## Giant-poplar / frontage identity

Near-identical leafless giant poplars, clipped hedge, artificial bright grass/dandelions and P-6V-like center edge define the corridor. Selected deterministic poplar gaps are legal ceiling cross-carriageway windows. Thin civic/residential frontage and dedicated micromobility/courier strip remain current direction.

## Debris / wallride

Outer debris strip provides progressive drag and transition bias into/out of the wallride with bounded pooled effects.

Wallride uses **7 longitudinal bands**, Tatra roughly 3. Rear-first/front-first loss are distinct; recoverable/terminal states exist; no cloth simulation is gameplay authority.

Mass effect is actual FGear/load state: more load increases roll/pitch/settling and modestly worsens accel/brake, while at matched wallride entry it gives only a **modest single-digit-percent direction** of improved abrupt-separation resistance. Exact contact-force calibration remains `CD-592/CD-701`.

## Five-lane freight ceiling

Exactly five upper freight lanes aligned above the five lower express lanes. Upper freight always moves **opposite** lower-road direction. Single units and 3–5-unit convoys valid; exact traffic numerics remain open/calibration.

## Magnetic ceiling contact — LINEAR LOAD LOCK

The old `5.0 seconds at every load` rule is SUPERSEDED.

- **1657 kg → 5.0 s**;
- **2107 kg → 4.0 s**;
- legal lighter states cap at 5.0 s;
- **linear interpolation by authoritative total mass** between endpoints;
- in-range formula: `timeout_s = 5.0 - (total_mass_kg - 1657) / 450`;
- reacquisition before expiry returns valid/stable contact;
- expiry enters normal detach/failure.

Interpolation choice is closed. **Whether reacquisition immediately restores the entire residual budget or follows another bounded reset/recharge rule remains owner question `M05` and is not implementation calibration.** Magnetic force/damping/contact-angle, strip/gap geometry and failure tolerances remain `CD-592/CD-701` calibration.

## Cross-carriageway jump

Only fast direct side swap:
`ROAD → DEBRIS/WALLRIDE → CEILING → FREIGHT GAP → POPLAR WINDOW → OPPOSITE LANDING/REACQUISITION → ROAD`.

Ground lanes offer no equivalent instant shortcut. Static CityCode preserves approach/aperture/landing; dynamic traffic and load may still make execution dangerous.

## Tatra load / chassis fixture

Base 1450 kg; full fuel 1550 kg; heroine 58 kg; daughter 49 kg; actual passenger masses; benchmark **1657 kg**; declared max **2107 kg**. Reference static balance target **45/55 F/R @1657 kg**. Historical initial chassis baseline ~2750 mm wheelbase / ~1485 mm front track / ~1400 mm rear track / 185R15-like tall sidewall. Final rear body stretch remains `CD-561` model-sheet work.

## Practice Hangar / CityCode

Practice Hangar remains bounded ServiceNode for floor→wall→ceiling→wall→floor practice, clean reset and exact same-Tatra/CityCode return. Passenger mass must not reroll through streaming/save because it affects FGear dynamics and magnetic timeout.

## Remaining owner-open vs calibration

Owner-open/code-facing: exact cross-section geometry, final wallride geometry/oscillation/recovery architecture values, freight traffic values needed by traffic code, road-defect generation values, legal tolerance/fines, incident persistence/frequency where stateful, Practice Hangar geometry where required by collision/route code, poplar/median geometry, final traffic roster, and `M05` reacquisition reset semantics.

Presentation-only art density/budget questions do not lower BASE-100 unless they create runtime/API/performance constraints.

Calibration: FGear steering/tire/suspension/brake values, exact wallride contact force and magnetic force/contact tolerances. The **linear 5.0→4.0 timeout**, 45/55 reference and 1657 benchmark are closed.

## Corridor Zero

`CD-679` proof must include no signal/jam dependency, FGear-backed road defects, seven-band wallride, five-lane opposite freight, 1657/2107/intermediate **linear** timer cases, reacquisition under the final M05 rule, moving incident bypass, Practice Hangar round-trip, reverse CityCode and bounded counters. Integrated hero-vehicle stack evidence comes from `CD-738`.

Runtime remains NOT VERIFIED until executable evidence exists.
