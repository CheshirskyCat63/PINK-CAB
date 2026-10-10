# VF90 Task6 - review corrections and remaining full-clutch blocker / 2026-10-10

**WIP / PR73 DRAFT / Task6 NOT accepted.** Current executable source: `cfd9bfeab3c3df986c0d8fe6bc54fb0450c3d858`, including `9fdc7f0efbecf131634d99b8a57fad4d34e89032`. Base of this correction: `900b5e4992b99ad55fc0251e94a84796d1df84fa`. Main remains integrated Task5 `ed18a57`; installed accepted `6edea7747d3a8433188c9fb394b98ae9c320d49b` is unchanged. No package, owner verdict or merge.

## What changed in this iteration

1. **Configured capability is no longer a constant.** It requires a valid project movement, created physical state, enabled mechanical simulation, valid clutch configuration and valid wheel instances matching their setups. Null/uncreated/invalid/stock/disabled-mechanics cases return Unsupported. The live-car test separately verifies available native extension, disable and restore. Availability is never a full-clutch PASS.
2. **Command snapshots follow the native async packet, not the newest game-thread command.** The game thread completes the actual native input packet, including validated selected gear, and publishes its immutable extension under that packet's identity. TickVehicle selects only that native packet's snapshot, with no replacement of old PT NetworkInputs by newer commands. Repeated substeps keep their original command. Reset belongs to the next native input; it cannot rewrite queued inputs. Physics recreation gets a fresh shared channel with no PT UObject access.
3. Both fixes are regression-tested inside the existing mandatory runner. Native clutch maths/settings, torque path, physical profile/calibration8, mass/CoM/inertia, geometry, steering, tyres, springs, engine/gear coefficients, Content, Config, project plugins and workflows are unchanged by these corrections.

## Review and reproduced defects

The review of900b5e4 found three distinct issues: missing fully-coupled holding behavior, newest-command/native-frame mismatch, and capability claimed without a configured provider. The last two are corrected here; full holding remains OPEN.

- Capability RED reproduced four incorrect available reports; GREEN proves the real runtime is required.
- Packet-identity and queued-reset tests were added against the old singleton with ignored packet parameters first. Both failed before implementing exact-packet selection; both then passed.
- Review of9fdc7f0 (comment4237643680 / review5478972266) found that disabled native mechanics still advertised availability. The added live-car disable/restore assertion failed on9fdc7f0, then passed with the one-line enabled-state guard in cfd9bfe. Terminal-damage behavior itself was NOT rewritten or certified.
- Review request6097566334 explains packet lifetime, capacity, reset, local-only limits and the open holding defect. Independent review is not automatic whole-Task6 acceptance.

## Packet lifetime and supported scope

The map key is the exact FChaosVehicleAsyncInput identity, never a dereferenced external pointer. Native ownership prevents reuse of a packet address while that packet remains alive. Under local monotonic consumption, only history older than the last consumed command is reclaimed; its current packet remains available for repeated substeps. The current physics simulation and its channel have shared value-only lifetime, and a recreated simulation gets a different channel.

The explicit 256-packet safety budget is NOT claimed to be a native Chaos queue constant. Exhaustion never evicts an unconsumed command: publication fails, an ensure identifies it, and missing PT input is invalid/no-propulsion with no latest fallback. The tests cover delayed A/B/C consumption, repeated steps, unknown packets, future reset, exhaustion without eviction, recovery and2048 recycled identities with bounded retention. No missing/capacity diagnostic appeared in the full runtime runs. Unbounded lag, out-of-order replay/network resimulation and every possible pause/lifetime scenario are not certified; full lifecycle/soak remains required.

## Fresh final executable-source verification

The following runs used the exact working contents subsequently committed as cfd9bfe, not older subsets relabelled as this source:

| Scope | Result |
|---|---|
| Final Editor build | PASS |
| Mandatory existing runner | 38 executed:37 PASS /1 FAIL |
| Combined engine/control/cockpit/interaction/provider/actuation | 99 executed:98 PASS /1 FAIL |
| Only failed test in both suites | ClutchLoadedLock |
| Unconfigured and live enable/disable/restore capability | PASS in both applicable final scopes |
| Exact packet identity, queued reset, retention/reuse | PASS |
| Existing28-configuration real-car clutch envelope | PASS |
| Existing126-case native joint plus open-release checks | PASS, bounded primitive proof only |
| Scripts | 58/58 PASS |
| Code health, repo hygiene, authority, scope, whitespace | PASS |

The new required tests are UnconfiguredClutchCapability, ClutchAsyncFrameIdentity, ClutchQueuedReset and ClutchFrameRetention. Existing34 tests remain included. The selection-script regression requires all new names. The intermediate file named t6-r12-selection-red contains an OK result and is NOT cited as RED evidence; actual native capability/packet RED logs below establish their regressions. The scopes overlap, so do not sum them or translate their pass ratio into car-readiness.

## Remaining full engagement defect - NOT FIXED

At full healthy coupling, first gear and0.4 throttle for6s, after the initial3s the mandatory final run recorded136 samples, mean1419.688RPM slip, maximum1665.426RPM and709.943cm/s. Combined final run recorded139 samples, mean1415.224RPM and maximum1678.756RPM. The existing25RPM diagnostic limit remains unchanged and failing.

The finite native velocity-drive connection lacks required static holding under load. Its algorithm has NOT been changed by this correction. Do not inflate damping/inertia, change LockedSlipRpm, snap engine/wheel velocities, reduce grip, hide the failure or introduce independent full/partial propulsion to force a PASS. Solve the actual holding/load connection before accepting the clutch. Reduced-capacity inputs do not prove the Vehicle Health heat producer. Existing terminal-engine-damage mechanical-disable and heat proxy behavior remains unaccepted broader work.

## Numbered execution and readiness

Tasks0-5:6/10 scoped stages closed (60% stage closure, not realism/effort). Task6 local reporting remains3/6 blocks (50%): handbrake, neutral/full-open/coast and held steering verified; the full clutch, complete robustness and terminal integration remain open. Correcting two review defects does not close the clutch block while loaded lock fails.

Next within Task6: correct full engagement/holding, then finish actual idle/stall/limiter/terminal-health, reset/save/reinit/focus/parking, work-based heat/wear and rendered-FPS evidence; obtain final review and applicable clean package/admin gates. Task7 dry handling -> Task8 wet/load/FPS/soak -> Task9 safe exact package and owner verdict. Installer rollback-retention defect remains Task9/CD-559. No broader world work or installed replacement.

## Source convergence

The900b5e4 integration receipt retains its exact original33/34 and94/95 results but is explicitly qualified: its earlier command-coherence and null-provider capability claims were disproved by review. This report and the current roadmap take precedence. Git/Jira/Confluence point to the corrected source, with full Task6 RED. Historical accepted sources remain valid only for their accepted scopes.

## SHA256 evidence

Paths are relative to C:/workspace/pinkcab-vf90-runtime-20261007. The validation log with the only failed physical requirement is kept rather than suppressed.

- `Saved/VF90/t6-r1-capability-red-20261010.log`: `4980537d15d08becfd95850187289843509e39efbc78491e112b557e5926b288`

- `Saved/VF90/t6-r1-capability-green-20261010.log`: `0e4b9a00752b82b4c2ad5788fd50f1a18bc4716379f70d0b0b703dfbf119fd69`

- `Saved/VF90/t6-r2-async-red-20261010.log`: `aa221d870fd33a42986fea5c9eda2447f126ea8eed0a2f6a9cc982f65f4179ba`

- `Saved/VF90/t6-r2-async-green-20261010.log`: `aae0171019ef6062c1bda7475481b173acf4ee9f02a0d6fd838b510497dc90bd`

- `Saved/VF90/t6-r1-mechanics-red-20261010.log`: `1ed5f4820ab83626c7248a179a0fb593fd4638a0f7a5dcb7680dd0b1e983ea1e`

- `Saved/VF90/t6-r12-review-final-build-20261010.log`: `ba52cce7665845de0ff791583aa80af98282cec7cc15a49b4c11a232de82eb08`

- `Saved/VF90/t6-r12-review-final-required-20261010.log`: `51de882db02ae684dfe875d98cc758ca909533c3217b25adf59d52f4115f640c`

- `Saved/VF90/t6-r12-review-final-joint-20261010.log`: `702d4b78d50091bdeeb371ad5ffe8d45ca55127ae5fe7aa6b440ec8c150fa771`

- `Saved/VF90/t6-r12-review-final-scripts-20261010.log`: `6598712da0f661a5dee0eccf01b4ac3a98c90917a289cc8aa1634bc142b59120`
