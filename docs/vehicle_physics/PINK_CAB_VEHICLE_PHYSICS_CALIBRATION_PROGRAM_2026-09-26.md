# PINK CAB — Vehicle Physics / Suspension / Handling Calibration Program

**Program mirror:** 2026-09-26  
**Scope:** canonical program + execution evidence; accepted control grammar and world remain frozen unless a stage explicitly says otherwise.  
**Original audit baseline:** `main@55ee8173af3c627cf26a06b95ec8628f5077179c`.  
**Current canonical main / PHY-001 frozen baseline:** `0e1a8bce8e29e56a1c16c28c9945467aed986048`.  
**Runtime owner:** Unreal Engine **5.8.3** Native Chaos Vehicles behind `IPinkCabVehicleDynamicsProvider` (exact Windows runner `Engine/Build/Build.version` evidence from run 36213319172).  
**Current execution point:** **PHY-003 · Causal drivetrain telemetry**.  
**Primary Jira owners reused:** CD-848, CD-648, CD-612, CD-643..645, CD-649..659, CD-670, CD-722, CD-740, CD-855/856. No duplicate implementation epic is created.

## Non-negotiable player-mechanic locks

- Mouse steers by default; Space is gaze/free-look and releases back to steering cleanly.
- Q clutch, W brake, E throttle. Wheel recipient priority when overlapped: E → W → Q.
- A new launch from standstill still requires fresh E+wheel throttle dosing.
- RMB is optional acquire/retain only; it never actuates by itself. LMB/wheel may perform an authored contextual action without RMB where the current recovery contract allows it.
- H gate remains 1/3/5 top, 2/4/R bottom, real neutral corridor; request != actual engagement.
- Continuous clutch and analog handbrake stay continuous. No ABS, ESP, auto-countersteer, yaw rescue, autothrottle, auto-rev-match or hidden speed/trajectory correction.
- City, MetaRoad, fare/economy, save transaction semantics and accepted R1/R2/R3 world work are out of scope.

## Driving-physics intent — causal simulation without hidden helpers

The target is **simulation-leaning but not tedious**. Assetto Corsa is used only as a reference for connected physical causality: driver input → engine/drivetrain → tire forces → load transfer/suspension → chassis response. PINK CAB does not import racing-car parameters, assists or maintenance burden from another game.

Accessibility comes from the car and the input mapping, not from trajectory rescue:

- predictable tire build-up, breakaway and recovery; the player gets warning before full loss of grip;
- fast manual countersteer remains available at speed;
- small pedal/steering mistakes are survivable because the physical tune is readable, not because software secretly corrects them;
- large mistakes remain real mistakes: wheelspin, lockup, stall and spin are valid outcomes;
- no auto-countersteer, yaw rescue, hidden brake/throttle/clutch/gear choice, velocity overwrite, hidden speed wall or direct-force boost;
- speed-dependent mouse shaping may change **device sensitivity/integration**, but changing vehicle speed alone must not retroactively shrink an already-authored steering target;
- normal 30-minute driving must not require service; wear/heat must be caused by actual work/slip, not arbitrary command-time accumulation.

This is the acceptance doctrine for P00–P11; it does not change the accepted control grammar.

## Verified current-code facts that force this program

At exact audited `main`:

- `FPinkCabChaosPhysicalProfile` uses **IdleRPM 750**, **MaxRPM 8500**, **MaxTorque 260 Nm**, ~**250 hp** design test, final drive **3.2**, forward ratios **4.6/2.2/1.5/1.1/0.85**, reverse **4.6**.
- `FPinkCabGearboxControllerConfig` still uses **IdleRpm 750** and **MaxSafeEngineRpm 6500**, so the engine/gearbox RPM envelope is internally inconsistent with the 8500-rpm physical profile.
- Current wheel/profile seed: radius **32.13 cm**, width **20.5 cm**, wheelbase **2980 mm**, tracks **1520/1520 mm**, reference service mass **1657 kg**; these are Tatra-613 donor/profile values and must not silently define final 603-I or 77 geometry.
- Nominal suspension seed is spring **170**, damping ratio **0.38**, travel **160 mm front / 180 mm rear**; current values are calibration seeds, not accepted final ride targets.
- `WheelLoadRatio = 0.38` on both axles deliberately weakens tire-force dependence on wheel load; it must be tested as a calibration choice, not treated as an invisible stability requirement.
- Nominal front/rear friction-force multipliers are **2.00 / 0.50**; the rear reduction was authored to make wheelspin easier and is therefore a high-priority shortcut candidate.
- Current steering config is explicitly speed-shaped (1400 counts base; stationary travel scale 3.60; moving 1.35→2.20; response 2.5/s stationary, 10.5→6.0/s moving; high-speed target gain 0.55 at 120 km/h). The current implementation multiplies the held steering target by this speed gain, so speed alone can change the final command.
- Throttle response currently uses `pow(driver, 0.55)`; 25% driver input becomes roughly 47% engine command and 50% becomes roughly 68%, so dosability must be checked independently from power changes.
- Partial-clutch drive torque is authored separately in `FPinkCabChaosCockpitBridge`; it is gated by Running state, while `FPinkCabChaosVehicleDynamicsProvider` still receives throttle commands independently. Existing live engine-restore test proves mechanical-sim enable/disable, **not** the stronger invariant “engine Off can never generate positive wheel drive torque.” This is why P00/P01 starts with telemetry/root-cause proof rather than a blind throttle-zero patch.

## Current authority conflicts to resolve, not paper over

1. Confluence drivetrain page 27 still carries historical **180 hp / 240 Nm / idle 850 / 6000 redline / 6200 limiter** plus old ratios, while executable code/tests currently lock a boosted **250 hp / 260 Nm / 8500** profile.
2. Code has **750 idle**, while the owner requirement for the next calibration is a warm carbureted **900–950 RPM**; **925 RPM** is the proposed center for A/B, not a claim that the final tune is already accepted.
3. Gearbox **6500 MaxSafeEngineRpm** conflicts with the 8500-rpm profile and can misclassify legitimate operating range as dangerous overrev.
4. Some Confluence page banners still say RMB+LMB is mandatory for lever movement although the current page-47/recovery authority allows direct authored LMB contextual manipulation. Physics work must follow page 47 / recovery contract, not stale banners.
5. `WheelLoadRatio=0.38` and front/rear friction multipliers `2.00/0.50` are current tuning shortcuts/candidates, not accepted physics doctrine. P06/P07 must re-derive load transfer, combined grip and rear breakaway from evidence.
6. The speed steering gain currently changes the final held target; P03 must separate device sensitivity from physical steering authority so speed alone cannot steer the car for or against the player.
7. `pow(driver,0.55)` materially amplifies small throttle inputs; P03/P04 must A/B dosability before calling quicker acceleration successful.
8. Hero canon remains bespoke early/Gen-1 603-family. A 613 may remain a donor/reference profile; supporting 613/603-I/77 profiles does **not** silently redefine the hero identity.

## Execution rule

One stage at a time. Every runtime-changing stage uses: RED/reproduction → minimal change → focused automation → full relevant regression → package/runtime check → human feel gate where specified → evidence attached to exact SHA. Do not tune the next stage on top of a rejected feel candidate. Parameter deltas must be data/profile changes unless a causal code defect is proven.

## P00 — Baseline + observability

**Reuse owners:** CD-848 / CD-645.  
### PHY-001 — Freeze executable baseline

**Status:** **DONE · exact-main verified 2026-09-26**.  
**Change:** Pin main SHA, accepted vehicle SHA, active candidate, profile version, executable/hash and rollback build.  
**Acceptance:** Exact executable ↔ manifest SHA; rollback launches prior accepted physics.  
**Evidence:** canonical `main@0e1a8bce8e29e56a1c16c28c9945467aed986048`, Windows run **36213319172**, UE **5.8.3**, profile `PINKCAB_TATRA613_CHAOS` schema/calibration **1/1**, deterministic profile hash `E83EAC2B6FA5D39F`, candidate exe SHA-256 `bb95af6903f3ad18140da2b54d366a981e839d9f35e97e1a7e27599ebd1d6084`. Immutable baseline root: `E:\CHESHIRE_DIVISION\Builds\PINKCAB\PHYSICS_BASELINE_0e1a8bc_RUN36213319172`. Prior accepted rollback: `8168d72406af6934ab20eace583c2b895f0620b7`, run **35809749568**, exe SHA-256 `994a66da0f62e56cdf65f4fdf4625b98d43ecd7366787075aa3c19a674df8469`; its historical unsigned Authenticode state is recorded, while identity is enforced by BUILD_SHA + GATE_MANIFEST + SHA-256. Original rollback run is successful launch evidence. No owner feel gate was required because PHY-001 changes no driving behavior.

### PHY-002 — State-writer inventory

**Status:** **DONE · exact-main verified 2026-09-26**.  
**Change:** Map every writer/consumer for ignition, engine RPM/torque, throttle, clutch, gear request/engagement, steering, wheel drive/brake torque, mass and health.  
**Acceptance:** **PASS.** Machine manifest contains **68 classified writer groups / 80 executable occurrences**. Exact-main guard reports **0 unknown, 0 stale, 0 count mismatch, 0 forbidden side paths**.  
**Evidence:** canonical runtime merge `main@e5077942ee93499247e61d358c55f0b2c222cb72`; exact-main Windows run **36230537265**; repo/code-health **32/32 PASS**, zero-debt **0**, `PinkCab.Vehicle.Physics` **3/3 PASS**, Editor/Game build **PASS**. Authority artifacts: `STATE_WRITER_INVENTORY.csv` and `STATE_WRITER_CONSUMER_MAP.md`. RED audit exposed dormant `AlignInitialPresentationToGround()`, which could teleport the physical pawn and zero linear/angular velocity; repository audit found no production call site, so the dead method was removed rather than allowlisted. Guard now rejects direct force/impulse/torque/velocity/actor-teleport writers and non-false Chaos assist-control assignments. No owner feel gate was required because PHY-002 did not change accepted driving tuning or controls.

### PHY-003 — Causal drivetrain telemetry

**Change:** Add bounded timestamped trace of raw+semantic input, ignition, final throttle, combustion/partial-clutch torque, gear, coupling, wheel torque/contact/slip, speed, slope, profile id/hash.  
**Acceptance:** Engine-off incident can be reconstructed frame by frame; trace does not lose first fault frames.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-004 — Repeatable calibration fixtures

**Change:** Define flat, slope, bump/curb, slalom, brake and launch fixtures and deterministic input traces at 30/60/120 FPS.  
**Acceptance:** Five repeated runs are comparable; physics dt/FPS/profile/load fixture recorded.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## P01 — Engine state, idle and no self-propulsion

**Reuse owners:** CD-612 / CD-644 / CD-659.  
### PHY-005 — Single combustion permission

**Change:** All positive propulsion paths require authoritative Running state; stop/stall clears stale propulsion commands without freezing chassis.  
**Acceptance:** Off + 100% throttle + 1st and Off + partial clutch + R produce zero positive engine drive torque.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-006 — Coast is not propulsion

**Change:** Preserve inertia, gravity, tire/contact drag and rolling resistance while engine is Off; never zero world velocity to fake a fix.  
**Acceptance:** Off in N rolls on slope; after key-off on flat, vehicle coasts/decelerates physically.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-007 — Start/stall/restart state machine

**Change:** Verify Off/Running/Stalled transitions, low-RPM stall causality and restart cleanup without new controls.  
**Acceptance:** Low-RPM coupled overload can stall; restart restores torque exactly once and no hidden throttle persists.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-008 — Warm carb idle 900–950 RPM

**Change:** Move idle target into versioned vehicle profile; use 925 RPM as calibration center until owner feel gate, and align tach/audio/tests with profile.  
**Acceptance:** Warm healthy N idle remains 900–950 after settling and returns there after a blip.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## P02 — Clutch + transmission energy continuity

**Reuse owners:** CD-643 / CD-644 / CD-645 / CD-659.  
### PHY-009 — Unify final torque path

**Change:** Partial-clutch external rear torque and fully-coupled Chaos torque must consume one authoritative available-engine-torque result containing ignition, limiter, health/damage, torque-capacity, sign and ratio rules. Include wheel→engine reaction/load rather than a one-way torque shortcut.  
**Acceptance:** Coupling `0.5 → 0.999 → 1.0` has no discontinuity in delivered torque/RPM/energy, including limiter approach, 1st and R; repeat under hill load and hot/worn clutch.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-010 — Central RPM envelope

**Change:** Replace independent idle/redline/overrev constants with per-profile idle, red-zone, limiter and damage-overspeed fields.  
**Acceptance:** 8500 operating profile cannot trip a stale 6500 damage check; dangerous connected downshift still can.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-011 — Continuous bite/release

**Change:** Calibrate coupling curve/torque capacity while preserving Q semantics and player-selected release time.  
**Acceptance:** Distinct partial-coupling states are smooth; pressing Q mid-release interrupts cleanly.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-012 — One gear engagement validator

**Change:** Every request path uses the same load/speed/clutch validator and accepted H topology; focus loss cancels pending gestures.  
**Acceptance:** No software gate skipping; focus/menu loss cannot cause delayed engagement.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## P03 — Steering + pedal feel without mechanic changes

**Reuse owners:** CD-649 / CD-611 / CD-825.  
### PHY-013 — Steering transfer calibration

**Change:** Separate mouse-device integration from physical steering target. Heavy standstill feel and calmer high-speed mouse sensitivity may change how quickly the driver moves the target, but vehicle speed alone may not multiply/shrink a held target or create auto-centering.  
**Acceptance:** Small right input steers right; a fixed authored target remains fixed while speed changes without new mouse input; deliberate countersteer can reach the mechanically available target at speed; no post-input steering motion exists without a declared physical/self-aligning source.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-014 — Frame-rate independent mouse trace

**Change:** Audit mouse-delta units/integration, fast-motion loss and stale-delta tails at 30/60/120 FPS.  
**Acceptance:** Equivalent timed traces yield comparable road-wheel angle; no post-input ghost steering.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-015 — Pedal wheel dosing response

**Change:** Keep E→W→Q wheel priority and per-launch throttle reset, but explicitly A/B the throttle transfer curve (`pow(driver,0.55)` baseline) for low-input dosability. Tune response/curve only; never add throttle based on slip, RPM rescue or launch outcome.  
**Acceptance:** One wheel detent is precise; 25/50/100% commands remain clearly distinct and monotonic; reversal/pause resets burst acceleration; no double recipient and no hidden throttle.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-016 — Ownership regression

**Change:** Freeze Space/RMB/LMB/quick-access behavior while physics changes land.  
**Acceptance:** Direct authored LMB action works without RMB where contract allows; released quick key removes prompt without actuation.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## P04 — Acceleration + gearing

**Reuse owners:** CD-641..647 / CD-648.  
### PHY-017 — Torque curve + engine inertia

**Change:** Measure wheel torque and acceleration; tune profile torque curve, rev inertia and engine braking only—no hidden AddForce boost.  
**Acceptance:** 0–30/0–60 improve on selected A/B candidate; lift-off is causal and has no residual boost.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-018 — Ratios + rolling radius

**Change:** Generate speed/RPM table for each gear from actual profile radius, ratios and final drive; reconcile old docs vs executable profile.  
**Acceptance:** Steady-state speed/RPM matches profile within slip tolerance; 5th gear cannot receive anti-stall cheat.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-019 — Reverse traction

**Change:** Keep R right-bottom and same clutch/engine rules; calibrate low/medium/full throttle traction.  
**Acceptance:** 25/50/100% reverse throttle are distinct; R→N removes drive while preserving inertia.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-020 — Controlled A/B launch and acceleration

**Change:** Compare baseline/A/B with identical mass, fuel, surface and input; record medians/spread, wheelspin and jerk.  
**Acceptance:** Chosen faster response is not achieved by secretly reducing mass or changing controls.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## P05 — Mass, wheel geometry and collision

**Reuse owners:** CD-748 / CD-855 / CD-856.  
### PHY-021 — Physical↔visual wheel contract

**Change:** Validate 4 wheel centers, radius/width, suspension axes, scale, bones/sockets and body-road clearance.  
**Acceptance:** Four Chaos contacts align with four visible wheels; full travel does not detach/clip visibly.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-022 — Mass/COM accumulation

**Change:** Audit base/fuel/crew/passenger units, one-time application and CG movement under loads.  
**Acceptance:** 1657/2107 kg fixtures are reproducible; passenger restore does not double-count mass.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-023 — Inertia tensor

**Change:** Calibrate roll/pitch/yaw inertia from physical dimensions/pivots, not hidden yaw damping.  
**Acceptance:** Small steering does not rotate car unrealistically fast; release has no external straightening impulse.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-024 — Three-model import/profile validation

**Change:** Separate render mesh from physics profile; validate model id, scale, wheel transforms, collision and material grouping for 613/603-I/77.  
**Acceptance:** Mesh swap never changes input ownership; invalid wheel/profile binding fails loudly.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## P06 — Suspension

**Reuse owners:** CD-652 / CD-648 / CD-855.  
### PHY-025 — Static sag + preload

**Change:** Measure empty/crew/full-load ride height and usable bump/droop; derive stiffness/preload in Chaos units.  
**Acceptance:** Nominal load keeps bump/droop reserve; full load is not permanently on bump stops.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-026 — Damping

**Change:** Tune bounded damping for single bump and repeated roughness; do not fake chassis softness with camera only.  
**Acceptance:** Oscillation decays after one disturbance and does not grow through repeated bumps.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-027 — Roll/pitch balance

**Change:** Tune roll stiffness/load transfer while keeping visible old-car body motion and tire authority.  
**Acceptance:** Braking produces bounded pitch with contact retained; slalom roll reverses smoothly.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-028 — Curb/pothole/landing

**Change:** Separate normal road compliance from damaging impacts and verify contact continuity.  
**Acceptance:** Ordinary curb does not teleport/drop all contacts; hard landing gives causal bounded damage.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## P07 — Tires + brakes + thermal causality

**Reuse owners:** CD-650 / CD-653 / CD-656 / CD-740.  
### PHY-029 — Longitudinal/lateral grip

**Change:** Calibrate slip curves, load sensitivity, axle balance, differential behavior and dry/wet surfaces as one continuous model. Use `WheelLoadRatio=1.0` as the physical reference A/B before accepting any reduced value; remove the `2.00/0.50` front/rear friction asymmetry unless telemetry proves a vehicle-specific reason rather than a wheelspin shortcut.  
**Acceptance:** combined braking+cornering and throttle+cornering consume one grip budget; throttle and lift can produce progressive rear breakaway; split-µ/differential behavior is causal; recovery does not snap without corresponding load transfer; deliberate bad input can still spin the car.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-030 — Service brake without ABS

**Change:** Tune brake torque/bias/dosing while preserving W+E coexistence and explicit no-ABS rule.  
**Acceptance:** 25/50/100% braking differs; full lock has no hidden ABS pulses.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-031 — Analog handbrake 0–1

**Change:** Preserve moving hydraulic behavior vs stationary parking latch and tune rear torque curve.  
**Acceptance:** 25/50/100% handbrake are distinct; parking hold does not turn throttle into scripted stall.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-032 — Work-based heat/wear

**Change:** Compute clutch heat from transmitted torque × relative shaft speed and brake heat from actual brake work at each rotating wheel; command value or body speed alone is insufficient. Normal 30-min driving must remain serviceable.  
**Acceptance:** Free-rev with clutch open cannot heat rear brakes; a locked stationary/near-stationary wheel does not accumulate fictitious brake work from body-speed formulas; prolonged clutch slip heats clutch causally and proportionally.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## P08 — Persistence, health and boundary safety

**Reuse owners:** CD-670 / CD-722 / CD-740.  
### PHY-033 — Focus/menu/load boundaries

**Change:** Test active pedals, lever and handbrake through focus loss/menu/restart.  
**Acceptance:** Transient inputs clear; no stuck throttle or delayed shift.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-034 — Save schema versioning

**Change:** Persist durable engine/health/wear/profile identity; never persist transient input queues/captures.  
**Acceptance:** Wear survives load; old save migrates without free repair or duplicates.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-035 — Repair diagnostics

**Change:** Map measured failure → indication → symptom → bounded repair; preserve exactly-once economics.  
**Acceptance:** Repair one subsystem does not heal unrelated state; transaction is idempotent.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-036 — Long-run soak

**Change:** Run 30-min normal active drive and terminal 2-hour shift soak on final candidate.  
**Acceptance:** No forced repair under normal use; no accumulating input/memory/unexplained wear.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## P09 — Vehicle profile architecture: 613 / 603 Series 1 / 77

**Reuse owners:** CD-605 / CD-855 / CD-856.  
### PHY-037 — Versioned profile schema

**Change:** Per model: id/version, provenance, geometry, mass/COM/inertia, engine/RPM, ratios, wheel/tire, suspension, brakes, thermal/cabin capabilities.  
**Acceptance:** Every parameter has unit/provenance/owner; incomplete profile cannot silently inherit another model geometry.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-038 — Tatra 613 profile

**Change:** Freeze current donor-derived 2980/1520/1520 and wheel 205/70R14 values only in 613 profile; separate gameplay engine tune.  
**Acceptance:** 613 reproduces current physical geometry; engine tune changes do not mutate wheel/H-gate data.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-039 — Tatra 603 Series 1 profile

**Change:** Measure/model-specific wheel centers, geometry and physical capability without copying 613 values; preserve shared semantic controls.  
**Acceptance:** 603-I wheels/suspension align to its asset; no model-specific input branches.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-040 — Tatra 77 profile

**Change:** Create third independent profile and migration id; historical claims require provenance, gameplay tune can remain explicit design.  
**Acceptance:** 613↔77 save/profile ids never mix; all three pass Off/idle/clutch invariants.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## P10 — Cabin extension seam: windows/heater/noise/defog

**Reuse owners:** CD-604 / CD-722 / CD-670.  
### PHY-041 — Semantic cabin ports

**Change:** Future cabin systems consume semantic commands + readonly vehicle telemetry, never raw keys or Chaos writes.  
**Acceptance:** Disabled cabin module cannot alter traction/steering; command routing does not steal physics ownership.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-042 — Windows contract

**Change:** Opening 0–1 per side, interruption, damage/jam and persistence; implementation later, after base-physics gate.  
**Acceptance:** Partial opening persists; each side restores independently.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-043 — Heater/power contract

**Change:** Model heat source, blower and residual cabin heat explicitly for air-cooled V8—no invented water radiator.  
**Acceptance:** Blower ≠ free heat; engine-off retains decaying stored heat rather than instant zero.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-044 — Noise/rain/defog hooks

**Change:** Bound window opening to allowed acoustic/air-exchange effects and heater to temperature/defog; no control-camera takeover.  
**Acceptance:** Effects stay bounded and cannot obscure/steal accepted driving controls.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## P11 — Regression, human gate, admin convergence and rollback

**Reuse owners:** CD-848 / CD-645.  
### PHY-045 — Full regression matrix

**Change:** Combine QA-01..20 with 96 PHY scenarios and exact-head package/runtime evidence.  
**Acceptance:** Every required row records SHA/profile/load/result/evidence; old green run cannot certify new executable.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-046 — Owner city human gate

**Change:** One candidate, short real-city route and baseline/A/B values: launch, brake, slalom, parking, reverse, roughness.  
**Acceptance:** Owner feel finding remains open even when automation is green.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-047 — Git/Jira/Confluence convergence

**Change:** Reconcile current profile, idle target, engine-off invariant, profile ids, accepted SHA/run and superseded historical values.  
**Acceptance:** All three sources agree on current authority; history remains traceable but not live truth.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

### PHY-048 — Single delivery + rollback

**Change:** Point only canonical launcher to accepted build, retain previous accepted package, verify executable hash/PID and rollback.  
**Acceptance:** Shortcut hash/SHA matches accepted evidence; rollback does not mutate city/save.  
**Evidence:** exact SHA + profile id/version + fixture/load + telemetry/log/test result; human-gate note if feel changes.

## File/symbol map from the 2026-09-26 audit

- `Source/PinkCabVehicle/Private/Vehicle/PinkCabChaosPhysicalProfile.cpp` — authoritative Chaos calibration application.
- `Source/PinkCabVehicle/Public/Vehicle/PinkCabGearboxTypes.h` — current 750 idle / 6500 safe-RPM defaults to eliminate as independent magic numbers.
- `Source/PinkCabVehicle/Private/Vehicle/PinkCabChaosCockpitBridge.cpp` — full/partial clutch handoff and external rear-drive torque.
- `Source/PinkCabVehicle/Private/Vehicle/PinkCabChaosVehicleDynamicsProvider.cpp` — final normalized controls, throttle response, drive/brake torque application and telemetry.
- `Source/PinkCab/Private/Vehicle/PinkCabVehicleControlRuntime.cpp` — pedal smoothing, drivetrain condition, gearbox evaluation and steering command composition.
- `Source/PinkCabVehicle/Private/Vehicle/PinkCabSteeringController.cpp` + header — steering transfer function to calibrate without changing semantics.
- `Source/PinkCabVehicle/Private/Vehicle/PinkCabThrottleResponse.cpp` — current low-pedal concave response (`pow(x,0.55)`).
- `Source/PinkCabVehicle/Private/Vehicle/PinkCabDrivetrainCondition.cpp` + header — stall/lug/health logic; must be reconciled with new idle/RPM envelope.
- `Source/PinkCab/Private/Runtime/PinkCabChaosTatraPawnState.cpp` — health-driven mechanical-sim disable/restore and load persistence boundary.
- `Source/PinkCabTests/Private/Vehicle/PinkCabChaosPhysicalProfileTests.cpp` — current boosted-profile assertions.
- `Source/PinkCabTests/Private/Vehicle/PinkCabChaosVehicleProviderTests.cpp` — provider and partial-clutch boundary tests; add engine-Off torque invariant here or adjacent runtime coverage.
- `Source/PinkCabTests/Private/Vehicle/PinkCabVehicleEngineRestoreRuntimeTests.cpp` — current mechanical-sim state proof; extend with no-positive-drive evidence.

## Definition of program completion

The program is done only when the selected vehicle profile has no positive propulsion with combustion disabled, warm idle is owner-accepted inside 900–950 RPM, RPM thresholds come from one versioned profile, acceleration/steering/suspension/brakes pass repeatable fixtures and owner feel, all three model profiles are isolated, future cabin systems have stable semantic/telemetry ports, full project regression is green on exact HEAD, and Git/Jira/Confluence point to the same accepted SHA/profile/evidence. Compilation alone is never acceptance.
