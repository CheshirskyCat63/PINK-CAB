# Master execution plan P00–P11

**Integration owner:** CD-921 / Confluence 22413517.

## Global execution rules

- One coefficient family per A/B iteration where possible; never change engine+tires+suspension simultaneously and then guess the cause.
- Every candidate has a rollback target and profile hash.
- Automated pass proves invariants; owner gate proves handling feel.
- No implementation work in R4 world branches.
- TDD first where a deterministic automation test is technically possible.
- After final change re-run relevant tests on exact HEAD; then independent review; then human gate where required.

## P00 — Exact baseline / causal telemetry

1. Fresh local and remote Git preflight: repo root, branch, exact HEAD, remote/upstream, status, untracked, active PR, latest commits.
2. Freeze executable path/hash, source SHA, UE version, active vehicle profile id/version/hash and accepted rollback R3.
3. Build writer→consumer map for engine state/RPM/throttle/clutch/gear/drive torque/brake/steering/mass/CoM/wheel contact/suspension.
4. Enumerate direct force/torque/velocity writes and classify as authoritative, bounded gameplay contact, presentation-only or forbidden.
5. Add ring-buffer telemetry: time/dt, raw+semantic input, ignition, RPM, requested/engaged gear, clutch command/coupling/capacity, final drive torque, brake torque, wheel contact/load/slip/travel, speed/accel/yaw/sideslip/slope/profile hash.
6. Create deterministic fixtures: flat standstill/launch, grade, coast, slalom, constant-radius turn, brake strip, single bump/curb/pothole, wet patch.
7. Capture same fixtures at 30/60/120 FPS where input transfer is involved.
8. Gate: no tuning starts until the trace can explain where vehicle energy came from.

## P01 — Engine state / Off propulsion / carburetor-like warm idle

1. Run state matrix Off/Stalled/Running × N/1/R × clutch open/partial/full × throttle 0/25/50/100.
2. Assert Off/Stalled -> zero positive combustion-generated drive torque at wheels. Do not assert zero velocity.
3. After engine stop at speed preserve linear/angular velocity and physically valid coast/back-drive/engine drag policy.
4. On slope allow gravity roll; classify wheel rotation caused by chassis motion separately from propulsion.
5. Trace Start/Running/Stall/restart transitions and clear stale propulsion exactly once without hidden throttle.
6. Move current warmed-idle target from observed 750 to owner range 900–950 through the actual engine profile. Use 925 only as first candidate.
7. Warm-idle fixture: healthy engine, N, declared base electrical load, 60 s after stabilization; record mean/min/max/variance.
8. Throttle blip returns to 900–950 without sustained hunting; normal load can droop and a bad clutch launch can still stall.
9. Do not model “carburetor feel” as random RPM noise; any hunting/choke/cold-start behavior belongs to explicit future engine-state design.

## P02 — Clutch / drivetrain continuity / gearbox

1. Separate clutch command, actual coupling, torque capacity and relative shaft speed in telemetry/state.
2. Audit the branch where partial clutch uses external rear torque and full coupling delegates mechanical gear to Chaos.
3. Apply the same engine-running, torque sign, health, limiter and profile bounds to both paths.
4. Sweep coupling through branch boundaries at fixed RPM/load; reject torque discontinuity/impulse.
5. Q+wheel remains release-time setting only; Q press during release interrupts deterministically.
6. All gear requests converge on one engagement validator using clutch/load/speed mismatch; no path writes engaged gear directly.
7. Reconcile all RPM fields: idle, red-zone warning, limiter/max, damage overspeed. Remove stale thresholds only after finding every consumer.
8. Preserve accepted physical H geometry and enlarged throws; no diagonal teleport, no hidden old reverse plane.
9. Gate: torque is continuous/bounded and requested-vs-engaged truth remains deterministic.

## P03 — Steering / pedal response without mechanic change

1. Instrument raw mouse delta -> semantic target -> steering controller -> physical road-wheel angle -> yaw response.
2. Tune center exponent and response rates rather than inventing assists.
3. Standstill “heavy” means slower/more effortful response while preserving every delta; it must not feel dead or lose motion.
4. At rolling city speed steering becomes noticeably easier/quicker; at high speed small-signal gain calms without removing deliberate countersteer authority.
5. Tune current pedal press/release smoothing to shorten intention-to-torque/brake lag while preserving wheel dosing.
6. Validate wheel burst acceleration: sustained same-direction faster; pause/reversal resets; one detent stays precise.
7. Replay at 30/60/120 FPS and irregular dt; compare road-wheel trace and final state.
8. Exercise Space, menu/focus, direct LMB lever manipulation and RMB retain transitions; no accumulated/residual mouse delta.
9. HUMAN gate: parking and 20–80 km/h city corrections must feel more immediate but still physical.

## P04 — Acceleration / engine character / gearing

1. Change order is fixed: correctness -> torque curve/rev inertia -> gear/final ratios + rolling radius -> clutch/traction -> aero/drag.
2. Never obtain acceleration by silently reducing authoritative mass or injecting AddForce/AddTorque boost.
3. Reconcile old 180/240/~6200 references with current 250/260/8500 code as provenance choices, not averages.
4. Build gear-speed-RPM tables from actual ratios/radius for 1–5/R and validate against runtime slip.
5. Tune low/mid-range torque shape and engine inertia for faster urban pickup while maintaining controllable rev behavior.
6. Validate launches at 25/50/100% throttle, several clutch-release settings and dry/wet surfaces.
7. Reverse uses the same drivetrain truth; any speed moderation must be physically explainable torque/ratio/drag, not teleport clamp.
8. A/B exact fixtures with same mass/fuel/surface/input; record 0–30, 0–60, 30–60, 60–100, wheelspin, jerk and RPM.
9. Numeric acceleration target is accepted only after HUMAN A/B; “faster” does not override mechanical skill.

## P05 — Mass / CoM / inertia / wheel-contact geometry

1. Separate base vehicle mass, fuel mass, heroine/daughter/passengers; verify the current 1657 kg reference fixture composition.
2. Move from longitudinal-only CoM adjustment to profile-owned local XYZ where Chaos integration allows.
3. Version inertia data per model instead of using one generic body response.
4. Validate PhysicsAsset/chassis collision vs Chaos wheel centers/radii on current 613 donor runtime.
5. Presentation wheels remain presentation; they may not silently become physics contact sources during model import.
6. Test empty/reference/intermediate/max load and verify monotonic effects on acceleration/braking/roll/settling.
7. Create model import validator for wheelbase/track/radius/pivots/scales and profile mapping.
8. Gate: no wheel buried/floating contact regression and no hidden seat-count preset switching.

## P06 — Suspension / weight transfer / old-car workload

1. Measure static sag per wheel and available bump/droop on reference load before adjusting spring/damping.
2. Confirm Chaos units and solve an initial spring/damper estimate from actual mass/corner load; tune from measured traces.
3. Current ~160 mm front/~180 mm rear travel is a seed, not a reason to fake visible travel if contact geometry disagrees.
4. Single-bump fixture separates spring/damper response from tire grip.
5. Constant-radius turn measures steady-state load transfer/roll balance.
6. Slalom measures transition, overshoot and settling; reject growing “boat” oscillation.
7. Brake-over-bump checks wheel contact and no-ABS lock behavior.
8. Curb/pothole/landing checks compression, rebound, chassis collision and wheel contact recovery.
9. Only after physical behavior is stable may expression/camera/audio amplify readable body movement.
10. HUMAN gate: visible old Tatra workload without mushy delayed steering or endless oscillation.

## P07 — Tires / brakes / wet RWD

1. Build wheel-load/slip sweeps for longitudinal, lateral and combined demand.
2. A/B rear nominal friction 0.50 against physically balanced candidates; keep it only if evidence says it is needed and coherent across load/surfaces.
3. One continuous grip -> slide -> deep slide -> spin model; no drift tire swap or speed-threshold script.
4. Power oversteer must build progressively and be catchable; committed bad input can still full-spin.
5. Wet/storm reduce available combined grip progressively and preserve throttle-ease recovery.
6. Service brake remains analog/no ABS; hard demand can lock wheels.
7. Handbrake 25/50/100% produces ordered rear-brake effects and clean release.
8. Brake+throttle coexistence remains player-owned; no hidden emergency braking/ESP.
9. HUMAN gate: dry city grip predictable, wet car alive but readable, drift never feels like mode switch.

## P08 — Mechanical health / persistence

1. Clutch slip -> heat -> warning -> reduced capacity/wear; no binary instant damage.
2. Gear conflict/forced mismatch -> grind/refusal/wear; gear-specific worsening where authored.
3. Brake heat/fade/cooling distinguishes reversible fade from permanent damage.
4. Tire/wheel/alignment/suspension consequences come from explicit hit zones/events.
5. No presentation dent changes handling unless Vehicle Health maps it explicitly.
6. Save stores schema/profile versions and stable health state; load reconstructs exactly once.
7. Loading may not heal or duplicate damage; repair is an explicit service transformation.
8. 30-minute normal urban drive should not force service; abuse must still have consequences.

## P09 — Tatra 613 / 603 Series 1 / 77 profiles

1. Introduce one versioned physical profile interface; keep shared input/state/telemetry/health implementation.
2. Stabilize Tatra 613 first because current donor/runtime geometry already exposes wheelbase/track/tire source data.
3. Build Tatra 603 Series 1 profile from approved 603-family design/model authority; never inherit 613 geometry silently.
4. Build Tatra 77 profile only from verified source/design inputs; unknown historical values stay UNKNOWN/CANDIDATE.
5. Each profile declares mass/CoM/inertia/wheels/suspension/engine/RPM/torque/transmission/brakes/thermal/electrical capabilities.
6. Each parameter has provenance; no “looks plausible” value is promoted to owner lock.
7. Run the same invariant engine-off/clutch/steering/brake tests for every model profile.
8. Changing selected profile must not alter accepted input grammar.
9. Hero identity remains current 603-family canon until an explicit product decision says otherwise.

## P10 — Future cabin-system readiness: windows / heater / electrical

1. Do not implement a new control grammar in this phase; define state/capability contracts only.
2. Windows: per-window position 0..1, movement direction, jam/broken state, optional motor/power requirement, persistence and semantic target binding.
3. Heater: explicit source type, fan setting, duct direction, cabin temperature, residual heat and power availability.
4. Air-cooled V8 means no generic water-coolant heater loop assumption; 613/603/77/retrofit profiles may differ.
5. Electrical/thermal accessory load may influence idle only through an explicit budget/state, never arbitrary "heater_on => -hp".
6. Prepare telemetry so later rain/noise/fogging systems can consume window/heater state without writing physics directly.
7. Vehicle Health may jam/break a window/heater component only through explicit states and repair mapping.
8. Gate: future systems can be added without changing engine/vehicle core interfaces again.

## P11 — Integrated acceptance / delivery / admin closure

1. Run full accepted input-contract regression before and after each final profile candidate.
2. Run engine-off/idle, clutch/gear abuse, steering FPS, acceleration A/B, mass/load, suspension, dry/wet/brake/HB and save/load suites.
3. Perform minimum 30-minute mixed normal urban soak; scan fatal/assert/ensure/error plus NaN/Inf/stuck ownership/energy gain.
4. Owner feel route: parking, repeated launches, stall/restart, all gears, 0–city acceleration, lane changes, hard brake, curb/bump, wet corner, reverse parking.
5. Capture exact SHA/run/exe hash/profile hash/test results and owner decision.
6. Only accepted candidate supersedes R3 rollback baseline; rejected candidate remains evidence but not authority.
7. Reconcile Git docs, Jira CD-921/current owners and Confluence page 22413517; do not mark unrelated cards DONE.
8. Independent code review before merge; verification-before-completion on exact final HEAD.
