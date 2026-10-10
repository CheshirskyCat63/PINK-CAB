# VF90 Task6 - native clutch integration, loaded-lock blocker / 2026-10-10

> **Subsequent review qualification:** this is the historical900b5e4 implementation/run record. Its claims that a newest complete command guarantees native-frame coherence and that constant ClutchCapability proves availability were disproved by review. Exact-packet selection and runtime/enabled/configuration capability validation are corrected in9fdc7f0 plus cfd9bfe. Full loaded lock is still RED. Current truth and fresh38/99-test evidence: VF90_TASK6_REVIEW_CORRECTIONS_2026-10-10.md. Original log identities/counts below are retained, not upgraded to current acceptance.

**WIP / Task6 NOT accepted / PR73 DRAFT.** Implementation and tests: `900b5e4992b99ad55fc0251e94a84796d1df84fa`, based on `a3110a448b1edeb2a632a23d54ec7abbc09be32d`. Verification ran on exact working contents captured by900b5e4; subsequent documentation commits do not relabel executable source. MainTask5 remains `ed18a57`; installed accepted `6edea7747d3a8433188c9fb394b98ae9c320d49b` is untouched. No package, desktop replacement or owner verdict.

## What is now implemented

The previously standalone native angular-joint primitive is connected to the real Tatra inside its existing wheeled mechanical override. Actual validated gear stays selected while continuous clutch pressure reaches the physics step; the old0.95 neutral/gear emulation is removed. Former PartialClutchTransfer and ClutchCapability failures now pass with actual native rear-wheel torque. This is real integration, but not complete clutch acceptance.

The implementation is an explicit project adapter around exported ImmediatePhysics_Chaos, NOT a claim of a stock clutch option on the existing vehicle. Native torque-limited angular velocity drive operates on two transient gathered rotor states, not hidden world actors, additional chassis mass or a second road vehicle. Original native Chaos owns tyre contact/friction, suspension, steering and chassis integration. The retired homemade clutch equation/integrator and Modular migration remain absent. Required command transport, kinematic reflection, unit conversion and direction-aware gearbox losses are visible adapter code for review, not disguised coefficient tuning.

One bounded synchronized complete command captures prepared steering/throttle/brake/handbrake and actual gear together with coupling, configuration and ignition/health permission. The existing TickVehicle copies that frame once and uses it throughout the step. No spare roll/pitch/yaw channel, independently sampled clutch with stale throttle, or physics-thread UObject read. Shared value-only lifetime; local single-player semantics, no networking/resimulation claim. Reset/focus/parking still require full end-to-end acceptance.

Native engine free-rev/drag and transmission ratios are retained. All clutch positions use one native connection-to-wheel torque path. No direct chassis force, native wheel/chassis velocity overwrite or competing full/partial propulsion owner. Actual gear, post-joint engine RPM, native wheel torque/omega, command sequence and connection energy are recorded on the same physical step. Capability says NativeConstraintExtension, not stock Native or full acceptance. Physical profile calibration8, engine/gear values, mass/CoM/inertia, springs, grip, Content, Config and workflows were not changed. New connection behavior still needs calibration and owner acceptance; unchanged parameters do not imply unchanged handling.

## Demonstrated fixes inside this integration

1. Initial SI-to-native wheel torque conversion was missing. Actual Chaos::TorqueMToCm fixed tiny wheel torque; no engine/friction boost was used.
2. Native position-based angular drive aliased high shaft rotation. The production-wrapper test failed, then passed after the native bUsePositionBasedDrives=false setting.126 cases cover3step sizes,7couplings,3shaft rates and2slip directions plus open release. Maximum test-oracle error0.000031Nm. The test-only implicit viscous-drive oracle is not a copied product solver; this verifies a bounded native primitive, not full dry-clutch lock.
3. Clutch-only input did not wake a sleeping body. RED then GREEN through the existing one-shot changed-control sleep hook; steady commands do not force continued wake.
4. Forward-only gearbox-efficiency scaling created energy in backdrive. The real-road test first recorded65 gain samples/max3.895J. Corrected direction-aware reflected load and once-only native wheel torque then gave zero gain samples in the same native-acceleration/open-coast/key-off partial reconnect. Final mandatory/combined runs had94/95 real backdrive samples respectively with nonnegative transmission losses. This is tested connection accounting, not whole-game energy/thermal acceptance.
5. Two tests still expected neutral on an open clutch or named the retired custom torque path. They now assert retained selected gear plus zero coupling and the actual NativeConstraintClutch path. Physical disconnect/no-torque/coast checks were retained.

## Actual car envelope

ClutchEnvelopeRuntime passes28 sequential Tatra configurations: first/reverse, nominal/reduced0.4 capacity input and0/.25/.5/.949/.95/.999/1coupling. Each has multiple distinct physical outputs and validates one prepared command frame, real gear/pressure/capacity, torque bound, zero front propulsion, zero open-clutch rear torque, nonzero partial rear torque and passive connection energy. Healthy quarter coupling produced561.6Nm per rear wheel; reduced-capacity quarter/half produced224.64/449.28Nm. These sequential transient values are NOT an otherwise-identical torque-response curve. Capacity-input tests do NOT certify the Vehicle Health heat producer.

## Blocking full-engagement result

ClutchLoadedLock is RED and mandatory. A normal native first-gear drive at full healthy coupling and0.4 throttle runs6s; after the initial3s, the same physical output compares post-clutch engine speed with actual rear-wheel speed through the actual gear ratio. No synthetic vehicle velocity or force.

Final mandatory run:134 samples, mean1424.245RPM slip, maximum1656.631RPM, speed707.799cm/s. Final combined run:145 samples, mean1384.752RPM, maximum1615.060RPM. Existing diagnostic LockedSlipRpm remains25RPM. It was not raised, used to snap RPM or turned into another full/partial threshold.

The finite native velocity-drive connection transfers bounded torque but does not yet achieve full healthy synchronization under actual wheel/road load. Resolve the connection/load boundary rather than inflate inertia, damping or grip, snap RPM or bypass the test. Basic positive partial transfer and earlier29/29 or92/92 subsets do not close Task6.

## Fresh final verification of900b5e4 contents

| Scope | Result |
|---|---|
| Final Editor compilation | PASS |
| Mandatory existing runner, original29 plus5 new required tests | 34 executed:33PASS/1FAIL |
| Combined controls/cockpit/interaction/engine/provider/actuation | 95 executed:94PASS/1FAIL |
| Failed test in both suites | ClutchLoadedLock |
| Production-wrapper NativeJointCapacity | 126 cases plus open-release checks PASS |
| Real-vehicle ClutchEnvelopeRuntime | 28 configurations PASS |
| Script unit tests | 58/58PASS |
| Code health/hygiene/authority/scope/whitespace | PASS;0 code-health violations |

Main suites overlap: never sum them or use their green fraction as car-readiness. Selection regression first failed for5 missing tests, then passed when NativeJointCapacity, ClutchCommandWake, ClutchBackDrivePower, ClutchEnvelopeRuntime AND ClutchLoadedLock entered the same verify-runtime.ps1. No excluded red requirement, new workflow or protection bypass.

## Ordered remainder and administration

Task6 still needs correct full engagement, then complete real idle/stall/limiter/terminal-health, save/reinit/focus/parking, work-based heat/wear and rendered-FPS proof. Existing terminal-health code disabling all mechanics and heat proxy paths are NOT certified or silently rewritten. Prepared-frame equality is not full lifetime/replay validation. The installer rollback-retention defect remains Task9 work. No Tasks7-9 or whole-car acceptance.

Tasks0-5 remain6/10 scoped closed stages (60%), not realism or remaining effort. Prior handbrake, N/open/coast and held-steering reporting blocks remain verified3/6 (50%); new partial-clutch implementation cannot close its whole block while loaded lock fails. No criteria were relaxed to increase percentage.

Review requested for900b5e4 versus a3110a4 in PR73 comment6097107557. Reviewer/hosted-CI results remain separately observable, not assumed in this receipt. Keep the PR draft until technical and administrative closure. Earlier exact-scope receipts remain history, not competing current status.

## SHA256 logs

Paths relative to C:/workspace/pinkcab-vf90-runtime-20261007. Failed intermediate runs are retained evidence, not final regression counts.

- `Saved/VF90/t6-integration-red-20261010.log`: `623c5ff88c3bc1af6c669cc0202ee11a375bae26ba3523b604928d920c0ff0df`
- `Saved/VF90/t6-native-units-20261010.log`: `df56c251a9fb0b53caeafed675999b0a4faf7b2c99082b76ff46fc328122fbb3`
- `Saved/VF90/t6-native-joint-rate-red-20261010.log`: `cd6a38c58e35cab2e1dbf0c8acdee53e158b9e9184680bd5ffdec2fc7a29caaa`
- `Saved/VF90/t6-native-joint-velocity-20261010.log`: `e30b1d60f83188450e68b681a2a4c0a714c284ab5965866e01dec4e762b757b5`
- `Saved/VF90/t6-native-controls-lifecycle-20261010.log`: `9d7c4d97b9ab4378c4361e5d9b90063c44e72c7c8cbf4625b0249c35e749355d`
- `Saved/VF90/t6-native-backdrive-red-20261010.log`: `4ebe71c088b03b0accb227a15b315f181f109eec2d9c0fa1b7fecaaeb55d32d3`
- `Saved/VF90/t6-native-backdrive-green-20261010.log`: `6361d7853a889630456ea5a29eb807094185ecf67883973f15d0f7bb7e4493fa`
- `Saved/VF90/t6-native-envelope-20261010.log`: `1c8d7c2ff49e19d06d743a1f169ebd532d9687858fc2403b19c118e5286beba7`
- `Saved/VF90/t6-loaded-lock-20261010.log`: `e5d57ca68407ebd816daaeada323208ba81799241e0c399bb910c7cc5c5a7710`
- `Saved/VF90/t6-loaded-lock-build-20261010.log`: `2f1d8833ecd78a40b95c913a84a96199e28af841b183222fb577292835e7a18f`
- `Saved/VF90/t6-native-final34-20261010.log`: `3150d471c16c8e3e57370f8d4af2e299f7629d5a725bbc6f1584052831a29698`
- `Saved/VF90/t6-native-final-joint-20261010.log`: `78954f875dc9edd1901ae0c027157878ccebe6503726336c8ce4fc8e591f42c4`
- `Saved/VF90/t6-native-final-scripts-20261010.log`: `ba82dff2723ff75ae3e68fcf6217e797bbcbba35abcab70c893332dfe4d1dab6`
