# PINK CAB · Gate 1 Road Handling · Acceptance and Test Overlay · 2026-09-28

Source: `PINK-CAB_Gate-1_Road_Handling_v1.0.zip` SHA-256 `3eed0b832eedc7411971c0010c75ce2d5c9bfc9bc453d84fc9037dbf41c20f43`.

All numeric criteria below are **PROPOSED**, imported from the source package. They are not measured achievements and remain **NOT RUN** until executed on an exact candidate.

Definitions: `W` = lane width. `V_ref` = measured/documented maximum/reference speed of the accepted baseline from T00. `V_low=0.50 V_ref`, `V_mid=0.75 V_ref`, `V_high=0.90 V_ref`. `beta` = chassis heading vs velocity angle on the road plane; do not treat beta below 5 m/s as a reliable slip metric. `Footprint` is the full physical vehicle projection, not the actor center.

## Acceptance A01–A14

| ID | Imported condition | Canonical mapping | Status |
|---|---|---|---|
| A01 Traceability | zero unidentified mandatory build/profile/level/engine/input/physics inputs; baseline acceptance linked | G1-001 / PHY-001 | NOT RUN for Gate-1 candidate |
| A02 Repeatability | >=5 valid repeats/condition; where median !=0, IQR/abs(median)<=5%, otherwise predeclared absolute tolerance; else INCONCLUSIVE | G1-004 / PHY-003/004 | NOT RUN |
| A03 Precision preservation | latency/RMS/settling candidate <= baseline median + max(5% baseline, 2×baseline IQR, epsilon); epsilon one physics step for time, 0.01W lateral | G1-003 / CD-649 | NOT RUN |
| A04 Straight | symmetric horizontal zero-steer: lateral deviation <=0.05W over 10 s after speed stabilization, no growing yaw; manual T01 footprint stays in lane | G1-012 / CD-658 | NOT RUN |
| A05 Lane change | 5/5 per side/speed: no collision/spin/corridor exit; center reaches ±0.10W target within 1 s and holds 1 s; abs(beta)<=3°; overshoot <=0.15W | G1-013 / CD-658 | NOT RUN |
| A06 Recovery | T04 moderate disturbance with 0.30 s correction delay: >=4/5 success per side at V_mid/V_high; no spin/collision/corridor exit; abs(beta)<=3° for >=1 s within 2 s of correction | G1-014 / CD-654/CD-658 | NOT RUN |
| A07 Throttle/brake | no spontaneous spin in T05/T11; straight braking footprint stays in lane; stopping distance does not regress beyond A03 formula with epsilon=0.5 m; no >=3 alternating shifts of one pair during 10 s stable command | G1-010/011 | NOT RUN |
| A08 Suspension | no continuous travel-limit contact on flat; after standard small bump deviation reaches <=10% first peak within 2.0 s and stays there >=0.5 s; no growing resonance on double bump | G1-008 / PHY-025..028 | NOT RUN |
| A09 Infinite road | 10 min continuous drive AND >=100 seams (continue if needed); no fall-through, removal under car, repeatable wheel detach on flat seam or unexplained pose jump | G1-005 / CD-869 | NOT RUN |
| A10 Optional drift | chosen speed range: >=4/5 each side no collision/spin/corridor exit; exit abs(beta)<=3° for 1 s; exploratory target beta 5–12° for 0.3–1.0 s; A03/A04/A05 remain green | G1-018 / CD-654 | OPTIONAL / NOT RUN |
| A11 Human | owner explicitly accepts precision and fun of exact build/profile; suggested 1–7 precision/predictability/fun >=5 and irritation <=3; one-owner gate allowed with limited generalization | G1-019 / CD-658 | NOT RUN |
| A12 FPS/state | supported 30/60/120 FPS with unchanged physics config pass A04/A05; median settling/RMS delta <=max(5%,2×IQR,epsilon); after 10 reset/restart no input/override/config-hash leak | G1-017 / CD-657/CD-658 | NOT RUN |
| A13 Cost/duration | physics/frame p95/p99 fit measured gate budgets; logging overhead measured; no unbounded road-section/memory growth; physics/frame time no >5% regression beyond measured variance | G1-017 / CD-657/CD-658 | NOT RUN |
| A14 Final | every mandatory criterion has evidence; final profile reproducible; post-integration smoke green; HUMAN_ACCEPTED points to exact build/profile; optional drift explicit | G1-020 / PHY-047/048 | NOT RUN |

Decision vocabulary: PASS / FAIL / INCONCLUSIVE / N/A. INCONCLUSIVE is never PASS. Critical single faults (fall-through, NaN, loss of control) are recorded independently and are not hidden by medians.

## Protocols T00–T11

### T00 — baseline / initial conditions
Record build, profile, tire/surface, input, hardware, road seed and physics dt. Warm 60 s. Resolve V_ref from a stable plateau or documented accepted maximum; do not use last frame as Vmax. Dynamic start: speed ±2%, center ±0.02W, abs(beta)<1°, known gear/RPM.

### T01 — straight / small corrections
At V_low/mid/high: 10 s zero steering after stabilization plus separate 60 s manual lane hold. Fixed small-correction impulse both directions. Five repeats/condition.

### T02 — single lane change
Center-to-center shift by W using fixed smooth trajectory `y=W(10s^3-15s^4+6s^5)`. Fix duration before candidate comparison. Check full footprint. Manual and fixed-input replay are separate series. V_low/mid/high, both sides, five repeats.

### T03 — double lane change / abort
T02 out, 1 s pause, T02 back. Separate mid-manoeuvre throttle-lift series; braking is T05. Keep duration/corridor fixed. Five repeats per side/speed.

### T04 — moderate error / recovery
Create a fixed steering impulse producing baseline abs(beta) 5–8° at V_mid without contact loss. Reuse waveform; define a separate moderate V_high impulse only if necessary and freeze it before A/B. Test correction delays 0.15/0.30/0.45 s; A06 uses 0.30 s. Five repeats/side/speed.

### T05 — braking / lift
Straight brake commands 0.3 and 0.7 plus separate maximum braking. During lane change use 0.3 brake at s=0.5 and separate full throttle lift. Record gear, ABS, TC and engine braking. V_mid/high, five repeats.

### T06 — small bump / damping
Reuse a representative level bump if it exists. If level is flat, diagnostic seed is a smooth 20 mm high, 2 m long bump (not a vertical step). Double bump uses the same shape, centers separated by 0.5 s at selected speed. This is suspension diagnosis, not off-road scope.

### T07 — infinite road
Fixed seed on real Level 1. Run A09 at V_mid and V_high if applicable. Log seam/stream/recycle/rebase and distance independent of world origin. Run a matching monolithic-plane control. Show height/normal/contact profile at boundary seams.

### T08 — optional short slide
Only after base acceptance. Begin V_mid. Freeze the input sequence and road corridor. Record peak/mean beta, duration, speed loss, recovery time and swept footprint. Five repeats each side. Negative result is retained; do not widen road or destabilize base handling to force success.

### T09 — FPS / focus / reset / isolation
Freeze physics timestep/substep/async mode. Run short T01/T02 at supported 30/60/120 FPS; unsupported modes declared before series. Verify focus/pause does not accumulate mouse delta/throttle. Ten reset/restart cycles. Separate packaged/editor evidence.

### T10 — human
Same device/camera except explicit camera A/B. 3 min familiarization per candidate, counterbalanced A/B order. Then five ordinary lane changes/side, three doubles, three braking runs and five recovery attempts. Minimum first gate is the owner; broader pilot 6–8 if available. Do not imply group results when only owner tested.

### T11 — throttle / gears / tire response
Compare throttle 0.10/0.25/0.50/1.00 at fixed initial speed/gear/RPM. Check 10 s speed hold and gear changes. Repeat representative shift during T02. Tire force series require a controlled load/slip/torque bench; if the channel/bench is unavailable, do not claim a measured tire curve.

## Project-specific conflict rulings

- G1-015 conditional assist is **N/A / prohibited by current no-assist canon**, not an implementation requirement.
- R6 remains a diagnostic A/B candidate, not an accepted tire result.
- WheelLoadRatio=1.0 is a physical-reference A/B, not an automatic final target.
- Contact/CoM/inertia/suspension evidence precedes fine tire acceptance.
- Optional drift cannot block base Gate-1 acceptance.
