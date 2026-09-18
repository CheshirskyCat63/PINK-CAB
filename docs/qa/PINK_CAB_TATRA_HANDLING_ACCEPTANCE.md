# PINK CAB QA · Tatra Handling Acceptance

**Status:** QA SPECIFIED — RUNTIME NOT VERIFIED
**Authority:** `docs/PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md` + `docs/PINK_CAB_TATRA_HANDLING_E34_REFERENCE.md`
**Vehicle stack:** `docs/PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md`
**Jira:** owner `CD-729`; FGear profile `CD-732`; wet calibration `CD-734`; expression `CD-735`; integrated QA `CD-738`; telemetry `CD-657`; acceptance `CD-658`; mass/vertical `CD-701`

Every result records exact build/commit, UE 5.8.x / Chaos profile versions as relevant, Tatra physics/tire/surface/expression profile versions and exact load fixture.

## PC-T-HND-000 · Sole solver

Pass: hero Tatra road forces come from native Chaos Vehicles through the PINK CAB provider; no parallel second tire/suspension/drivetrain solver contributes forces. Presentation/expression stays separate from road-dynamics authority.

## PC-T-HND-001 · Low-speed steering precision

Prove continuous steering, deliberate fine/degree-level road-wheel command, no binary steps, no auto-countersteer, no mouse center dead-zone beyond jitter filtering. Mechanical lock target = **41°±2° inner wheel** unless final geometry validates a tiny correction through authority update.

## PC-T-HND-002 · 100 km/h placement

Repeat lane-change/long-bend traces. Steering progressive/repeatable; body motion visible but non-destabilizing.

## PC-T-HND-003 · 140 km/h calm weave

Small steering commands place car without nervous constant micro-correction while Tatra body/suspension workload visibly increases.

## PC-T-HND-004 · 170 km/h chassis workload

Lane change + long bend remain readable/recoverable while float/roll/pitch/rut sensitivity and expression are clearly stronger than at 100–140.

## PC-T-HND-005 · 195 km/h authority

Pass: stable small-signal steering; full deliberate player countersteer authority remains; engine not artificially power-starved; difficulty reads through chassis/suspension/road; no hidden velocity clamp as normal terminal mechanism.

## PC-T-HND-006 · Steering gain baseline

Measure speed-gain target family: `1.00 <=40 / ~0.72 @100 / ~0.52 @160 / ~0.42 @195`. Values may be finely calibrated only if owner behavior remains green and change is recorded in profile version.

## PC-T-GRP-001 · Progressive grip release

Run dry/wet/storm slip sweeps in the native Chaos/PINK CAB profile. Reject tire-force cliffs, hidden drift-tire swaps or binary grip modes. Dry/wet/storm μ seeds are calibration inputs, not acceptance law by themselves.

## PC-T-WET-001 · Wet >160 maintained-throttle escalation

At 160/170/195 km/h wet fixtures, perform rapid lane change while maintaining/exceeding throttle demand.

Pass: rear combined grip saturates progressively; rear slip/yaw can build to deep slide/spin; there is no 159→160 discontinuity and no scripted `speed>160` force injection.

## PC-T-WET-002 · Wet >160 throttle-ease recovery

Repeat same onset, then deliberately reduce excessive throttle without hidden brake/countersteer.

Pass: rear longitudinal demand drops, lateral reserve recovers, and player steering can settle/straighten car. Full abrupt lift may show normal load transfer but controlled throttle reduction must not perversely worsen ordinary recovery.

## PC-T-DRIFT-001 · Entry families

Throttle, weight-transfer/lift and analog handbrake can each produce rear-slip entry through one continuous Chaos/PINK CAB tire model. Useful sustained sideslip target = **18–40°**.

## PC-T-DRIFT-002 · Recovery

Competent trace recovers representative slide through player steering/throttle timing. Reject automatic countersteer/automatic brake rescue.

## PC-T-DRIFT-003 · Full spin remains possible

Committed bad input can exceed 90° sideslip/complete spin. No C+ yaw edge guard/yaw rescue may contribute.

## PC-T-ELEC-001 · ABS absent

Hard braking: no ABS hardware/state, no pulse intervention, wheel lock remains possible.

## PC-T-ELEC-002 · ESP absent

Yaw/spin: no ESP equipment, no individual-wheel ESP braking, no automatic steering correction, deliberate spin possible.

## PC-T-BRK-001 · Analog brake command

Sweep 0/10/25/50/75/90/100% plus intermediate targets. Command/delivered braking monotonic inside versioned profile. Initial bias target **60/40 F/R**.

## PC-T-BRK-002 · Load-normalized stop

Measure 100–0 dry/wet at **1657 kg benchmark**, at least one intermediate load and 2107 kg. Historical 38–42m dry / 48–55m wet remain comparison references, not silent pass criteria if final profile law explicitly differs.

## PC-T-HB-001 · Analog handbrake

Sweep 0/10/25/50/75/90/100%. Intermediate pull creates intermediate rear brake command; progressive lever curve; release returns cleanly; ~1700 Nm/rear-wheel is starting calibration seed, not binary drift button.

## PC-T-CL-001 · Continuous clutch

Clutch physical state remains continuous/deterministic through Q press/release.

## PC-T-CL-002 · Release-speed fine control

Contextual wheel changes **release-speed setting**, never instantaneous clutch pressure. Verify ordered durations across at least 10 representative positions and verify monotonic player-adjustable release timing across representative settings. The final numeric range is a reconciliation item and is **not a locked pass criterion** until explicitly accepted.

## PC-T-LAUNCH-001 · Start outcomes

Deterministic throttle + release-speed matrix demonstrates real stall possibility, reproducible clean urban launch and aggressive harsh/wheelspin launch; no launch control/auto-throttle.

## PC-T-BODY-001 · Physical vs expression channels

Changing only expression profile may change visible/camera/audio roll/pitch/heave/shake, never Chaos tire forces/collision/trajectory beyond numerical noise.

## PC-T-BODY-002 · Speed expression ladder

Run matched road family at 100/140/170/195. Expression shows increasing old-Tatra workload while physical control remains bounded.

## PC-T-BODY-003 · Roll / settle targets

At ~0.8g target physical roll roughly **4.5–5.5°**. Nominal visible roll begins around **1.35×** physical, pitch ~1.25×, heave ~1.20×. Sharp lane-change visual settling target ~0.9–1.2 s with no growing oscillation. Exact low-level coefficients may be calibrated within owner intent.

## PC-T-SURF-001 · Rut

Verify `RUT → FGEAR WHEEL/SUSPENSION → PHYSICAL BODY → STEERING TREMOR → EXPRESSION → AUDIO/VFX`. No random shake without owning physical/surface state.

## PC-T-SURF-002 · Pothole / patch / crack

Matched fixtures create synchronized suspension/body/cabin response. Large pothole may perturb wheel load/yaw physically; ordinary defects must not get scripted random heading changes. Expression can amplify readability but cannot invent grip loss.

## PC-T-MASS-001 · Load trend

Use legal lighter state, 1657 kg, intermediate load and 2107 kg.

Pass: more mass modestly worsens acceleration/braking, increases roll/pitch/settling/expression, no loaded/unloaded preset, actual passenger mass persists.

## PC-T-WALL-001 · Wallride load direction

At matched geometry/speed compare reference/heavier fixture. Heavier car has only modest/single-digit-percent direction of better abrupt-separation retention plus greater body-motion penalty. No easy-mode transition.

## PC-T-MAG-001 · Residual reference

1657 kg → effective **5.0 s**. Verify 4.9 / boundary / >5.0 detach.

## PC-T-MAG-002 · Residual max

2107 kg → effective **4.0 s**. Verify 3.9 / boundary / >4.0 detach.

## PC-T-MAG-003 · Locked linear interpolation

For `1657 < m < 2107`, expected `timeout_s = 5.0 - (m - 1657)/450`. Test at least three intermediate masses, including **1882 kg = 4.5 s**. Legal lighter state never exceeds 5.0 s. No nonlinear hidden curve/seat preset.

## PC-T-DMG-001 · Damage does not bypass handling ownership

Presentation/deformation alone must not mutate Chaos handling. Only authored Vehicle Health consequences from configured hit zones may alter approved runtime parameters. Compare cosmetic dent vs wheel-corner damage.

## PC-T-DET-001 · Cross-FPS replay

Replay identical controls/surface sequence at 30/60/90/120 FPS. Ownership/gear/outcome class matches and continuous channels remain within tolerance; no physical/expression/deformation desync.

## PC-T-SOAK-001 · 30-minute mixed handling soak

Mix express driving, 140–195 work, lane changes, long bends, ruts/potholes, braking, oversteer/recovery, shifts, analog control and representative damage states.

Fail on NaN/inf, stuck controls, unexplained energy gain, growing oscillation, hidden ABS/ESP, expression changing physical trajectory, VDS silently changing dynamics without authored bridge, or persistent-state leak.

## Required telemetry

Build/commit; UE/Chaos/PINK CAB profile versions; total mass/fuel/occupants; speed; raw/filtered steering/gain/road-wheel angles; throttle/brake/clutch/handbrake; clutch release-speed; gear/rpm/torque; wheel loads/slip/forces; yaw/sideslip; suspension travel/velocity; physical roll/pitch/heave; expression channels separately; surface id; camera/cabin/trim; yaw-rescue contribution (must be zero); damage/hit-zone consequence id; `ABS=false`; `ESP=false`; evidence artifact.

## 2026-09-18 release-matrix overlay

The canonical control/mechanics release checks `QA-01..QA-20` are defined in [`PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md`](../PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md). They include steering sign/FPS, per-launch E+wheel reset, wheel-recipient routing, continuous half-clutch, H-gate/N topology, requested-vs-engaged refusal behavior, analog handbrake, no hidden assist, stall/restart, drivetrain-damage persistence and the 30-minute normal-driving owner gate.

All are **NOT RUN** until exact executable evidence is captured.

## Maturity

Passing document review = SPECIFIED, not VERIFIED. Only exact-build executable evidence plus owner feel review can mark handling/stack VERIFIED.
