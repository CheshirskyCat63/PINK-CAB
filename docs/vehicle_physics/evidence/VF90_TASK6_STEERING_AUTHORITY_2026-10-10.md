# VF90 Task6 - native steering authority / 2026-10-10

Status: steering sub-scope implemented, locally verified and pushed as `59dcb4dbcd16cfe955831fa72934f1b9b4ea0010` in draft PR73. Whole Task6 is still NOT accepted. Main remains Task5 `ed18a57`; accepted installed `6edea774` is untouched.

## Change and preserved boundaries

The native default speed curve shrank a held steering target as road speed changed. At fixed 0.4 command the native bench returned14.03979 degrees at0MPH and5.45831 at60MPH. The real-road held-angle test failed as well. This violates existing CD-649, which allows shaping NEW device input but forbids rewriting a held physical target from speed alone.

Configure the existing native SteeringCurve as constant1, through versioned SteeringSpeedScaleCurve with DesignTarget provenance and deterministic fingerprint coverage. Calibration7 ->8. Preserve Ackermann, mechanical angle, input grammar/sensitivity, body/mass, springs, tyres and engine/gear parameters. No steering equation, chassis torque or velocity correction was added. ExternalCurve is detached before assigning the profile-local curve; a RED/GREEN regression proves shared assets remain unchanged.

Four required tests are in the existing runtime list: NativeSteeringAuthority, HeldSteeringRuntime, HeldSteeringReverse, NativeSteeringCurveOwnership. The two partial-clutch blockers remain included and failing. No new CI lane or guard bypass.

## Exact proof

Earlier local working contents were subsequently captured unchanged by59dcb4d. On2026-10-10 the resumed Editor build passed and nine steering/profile tests passed in t6-steering-resume-20261010.log before commit. They cover forward/right and reverse/left native travel, held wheel angles, configuration-speed sweep, shared asset ownership and profile identity/hash/provenance. High-speed road/countersteer/handling is NOT accepted from a configuration sweep or low-speed travel.

The same working contents previously completed mandatory29 with27PASS/2FAIL and joint controls82 with80PASS/2FAIL. Exact failures were PartialClutchTransfer and ClutchCapability. These are separate overlapping scopes, not an aggregate game-readiness percentage. Script58 and static checks passed in the prior recorded iteration. Hosted59dcb4d verification/review remains separate from these local results.

## Administrative reconciliation

Prior PR73 comment6096234118 and Jira environment fields correctly recorded UNCOMMITTED at that time. That publication blocker is now resolved by pushed59dcb4d. Task5 checkbox is reconciled to already merged PR72, without creating new acceptance. Current major-stage completion is6/10 scoped stages (60%), not driving realism or effort remaining. Task6 local reporting remains3/6 blocks (50%): handbrake, N/full-open/coast and held steering verified; continuous clutch, combined robustness and terminal integration remain open.

## Native clutch boundary - do not repeat invalid assumptions

Epic UE-181298 records missing manual clutch simulation in standard Chaos Vehicles, currently backlogged with a6.0 target; this is not a promise or a completed fix. Source: https://issues.unrealengine.com/issue/UE-181298 (checked2026-10-10).

Previous Modular core probe omitted SetSimModuleTree and consequently proved no transfer. Correcting the fixture demonstrated distinct torque, but a separate native fully-open test changed engine2000->1900RPM and gearbox1000->1100RPM despite zero drive torque. Reproducer remains outside compiled Source under Saved/VF90/native-clutch-boundary-confirmed-20261010.cpp.txt. No engine binary patch, unapproved Modular migration or restored custom solver is justified by those results.

Any next native constraint characterization is research only until compatible thread, inertia, torque, energy, save and output ownership are proven. Do not call a standalone native joint a shipped clutch, or use a second vehicle/propulsion solver. Owner requirement for real continuous coupling remains unchanged.

## Current-turn final readback and native feasibility

Fresh final clean-source build at59dcb4d passes. After both research sources left Source and the test Build.cs returned to its exact original content, the mandatory29 ran27PASS/2FAIL; the failures remain physical partial-clutch transfer and capability. This is a fresh final run, not the prior29 result relabelled. Independent review of59dcb4d reported no major issues (PR73 comment6096447558); no whole-Task6 approval follows.

Native physics joint feasibility is now demonstrated through the exported ImmediatePhysics_Chaos FSimulation API. No engine binaries, private symbols or copied solver equations were changed.84 setup cases (3 steps1/30,1/60,1/120;7couplings0/.25/.5/.949/.95/.999/1;2torque-capacity scales1/.4;2slip directions) plus an open-release step after each case passed. Checks cover bounded torque, equal/opposite angular momentum, no added energy and independent open shafts. Step-rate checks are NOT rendered-FPS acceptance, and capacity-scale checks are NOT integrated Vehicle Health proof. The standalone rotational fixture uses consistent SI inertia/torque with zero gravity and no colliding geometry; production UE centimetre conversion is not validated by it.

The preliminary world-joint fixture also passed but is kept only under Saved/VF90; no hidden world bodies are adopted. The raw private-kernel trial compiled but failed linking missing engine exports, so it was rejected, not patched. The exported ImmediatePhysics approach resolves that access limitation. Its minimal source is retained as a noncompiled evidence attachment RESEARCH_NATIVE_JOINT_CAPACITY_2026-10-10.cpp.txt. No research dependency or test remains in production compilation.

This native primitive is NOT yet a vehicle clutch. A real connection still needs one coherent physics-thread command snapshot, correct engine/shaft inertia and once-only torque exchange, native wheel/engine telemetry, limiter/ignition/health permission, save/load and reinitialization ownership. Existing wheeled setup does not offer a clutch field in its native input packet. Do not hide the missing integration by using pitch/roll/yaw channels, an arbitrary mailbox with mismatched frames, throttle scaling or duplicate full/partial torque owners. No such runtime architecture was installed in this iteration.

### Final log SHA256

- `Saved/VF90/t6-steering-resume-20261010.log`: `6af19dfd36a7db12197297adc2a239b661d92e2c7dc8cc7d4ece8a35c9bd55c2`

- `Saved/VF90/t6-steering-clean-final-20261010.log`: `72596494ce52130087bbf8e179833ed2499a46d084bc9bfe461e754797b498c1`

- `Saved/VF90/t6-immediate-envelope-20261010.log`: `1b1fb20c5a3ed2a1482dd948cdf23274fa0eb01e4da5db145c975ff22763b050`

- `docs/vehicle_physics/evidence/RESEARCH_NATIVE_JOINT_CAPACITY_2026-10-10.cpp.txt`: `a9ea11d2295f9a019e0f0406b7a7016db52b89d5e0c8d8b2066080118f19d702`

- `Saved/VF90/t6-native-kernel-build-20261010.log`: `cd3019f79a5973c0692d38aa4eb4fa4e59f0279254053d5b3682beead12d3a7d`
