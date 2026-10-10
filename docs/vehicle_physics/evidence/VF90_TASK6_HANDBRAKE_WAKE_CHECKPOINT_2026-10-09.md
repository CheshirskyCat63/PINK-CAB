# VF90 Task6 - analog handbrake wake checkpoint / 2026-10-09

Status: PARTIAL IMPLEMENTATION; Task6 remains RED. Not merged, not packaged, not owner accepted.
Base: Task5 main ed18a57be5c03e35acee252f74c8f7e8ae51d1e8; resumed WIP dd0e806179c478f3f917b5471fa1b31391d8afe1. Installed accepted source 6edea7747d3a8433188c9fb394b98ae9c320d49b is unchanged.

## Actual defect and production correction

Read-only diagnostics in task6-sleep-probe-2.log reproduced commands reaching native HandbrakeInput 0.25/0.5/1/0 while IsInstanceAwake was false and the rear brake output stayed425 Nm after the first0.25 command. Waking only in the command setter did not fix it (task6-wake-runtime.log).

The implemented correction carries an analog-command transition into the native ProcessSleeping hook, resets its inactivity counter and wakes once before permitting normal sleep logic again. Release is a transition too; repeated unchanged commands do not keep a parked car awake. No velocity/force/position reset, steering adjustment or second dynamics solver was introduced.

The native boolean input is explicitly kept false by PinkCabChaosVehicleDynamicsProvider, the existing raw-input owner. The analog float is populated in the native movement UpdateState hook. Code-health ownership rules were NOT widened to permit another raw-control writer.

## Measured output

In task6-native-final-runtime.log the production provider/movement on the real Tatra produced rear per-wheel425/850/1700 Nm for25/50/100 percent handbrake, and returned to the pre-existing138.75 Nm neutral-engine-drag output when released. Front brake output remained0. The former stale425 Nm output is resolved, not replaced by another latch.

A separately named AnalogHandbrakeIsolated fixture temporarily sets EngineBrakeEffect=0 on its own instance, recreates native physics, then restores the setting at completion. Its8-command sequence0/0.25/0.5/1/0/0.5/0.5/0 PASSED: zero release, proportional rear-only torque and stable repeated half input. This is brake-path isolation, NOT acceptance of the normal drivetrain. Native mechanical simulation remains enabled. The production profile was not changed to zero engine braking.

The original AnalogHandbrakeTorque test with the ordinary engine profile remains FAILED at zero-command rear outputs138.75 versus expected0. Its acceptance thresholds are retained. This identifies a separate neutral/disengaged drivetrain coupling problem; it does not yet prove a particular on-road deceleration magnitude.

## Explicit limitations / unsuccessful attempts

- The initial focus-cleanup extension to the provider-only fixture was invalid: it bypassed cockpit state, then expected cockpit reset to clear its independent override. That extension was removed. Parked lever state and transient interaction capture remain distinct; complete focus/menu/parking semantics are NOT verified here.
- A test-only NativeClutchCandidate probe attempted the installed Chaos Modular Engine/Clutch/Transmission API, first with passive shafts then with actual native node types. Neither probe demonstrated transmitted torque. The typed run also logged native condition failures. This is an inconclusive integration experiment, NOT evidence that the native engine is universally broken or that a replacement has been adopted. No Modular Vehicle runtime/plugin/pawn was connected to the game. Its source and logs are retained as RED research WIP, not mandatory green acceptance.
- Production clutch still maps coupling below0.95 to neutral. Continuous clutch, back-drive and neutral torque isolation remain CD-659 work.
- No final canonical20-test rerun completed in this continuation: the requested command was rejected by the tool service. Task5's old20/20 result does NOT certify the new branch.
- The attempted relocation of the exploratory source and its test-only dependency cleanup was rejected and did not execute. The probe still exists in the worktree; no cleanup success is claimed.

## Verification provenance

Editor compilation passed for the corrected handbrake and typed candidate probe. Final handbrake8-step result: AnalogHandbrakeIsolated PASS; AnalogHandbrakeTorque FAIL. NativeClutchCandidate FAIL / unadopted. Logs: task6-native-final-runtime.log and task6-clutch-typed.log under Saved/VF90.

The separately executed script suite56/56 passed; code health0, hygiene0, authority guardPASS, scope guardPASS in task6-script-regression.log / terminal output. That static run preceded the last test-only typed-node probe edit; it is not mislabelled an exact final-head runtime pass.

No new package or desktop delivery was attempted. Task9 still requires fixing deliver.ps1 rollback retention before replacing the owner's installed copy. Tasks7-9 remain held behind complete Task6 acceptance. Preserve Task5 main, known RED results, and existing scope; do not merge this checkpoint as completed controls.
