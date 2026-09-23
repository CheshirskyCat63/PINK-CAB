# PINK CAB · C-01…C-16 Recovery Execution Ledger

**Updated:** 2026-09-22  
**Branch:** `fix/CD-848-recovery-r1-input-contract`  
**Rule:** DONE means evidence exists on the exact candidate. OPEN/BLOCKED is not rounded up.

| ID | State | Evidence / next proof |
|---|---|---|
| C-01 | DONE | Jira CD-848 description and Confluence page 47 now explicitly adopt Recovery Input Contract R1: RMB grip only; RMB+LMB lever manipulation. |
| C-02 | DONE | Run `35611762556` focused matrix PASS: canonical drivetrain acceptance now uses `PhysicalControlRouting` through real quick-recall/RMB+LMB handbrake and gearbox controls to movement; old direct setup test is explicitly fixture-scoped and not acceptance. |
| C-03 | DONE | `ChaosBaseline.PhysicsOnly.DriveSmoke/ReverseDriveSmoke` naming and GitHub evidence scope are active on run `35611762556`; direct-provider smoke is no longer represented as gameplay-input acceptance. |
| C-04 | DONE | Run `35611762556` focused matrix PASS: `PhysicalControlRouting` reaches PlayerController→cockpit→runtime→Chaos→movement with real drivetrain control routing; ignition is the only explicitly excluded fixture. |
| C-05 | BLOCKED_HUMAN | UE InputKey/PlayerController automation is not OS/HID mouse evidence. GitHub gate records this as HUMAN_PENDING. |
| C-06 | DONE | RED on `35595470850` proved raw X=19/Y=-23 became 0/0. GREEN on `35596197869`: `ProcessedZeroPreservesRawPhysicalDelta` PASS after raw-first fix. |
| C-07 | DONE | Run `35596639287` reached the PlayerController gearbox path: requested first with no clutch remained `engaged=0`; real Q then engaged first before launch. |
| C-08 | DONE | Run `35611762556` focused matrix PASS: movement acceptance traverses real RMB/LMB handbrake + gearbox states before Q/E+wheel launch; direct-provider smoke remains physics-only evidence. |
| C-09 | DONE | RED on `35596197869`: zero pedal fabricated 131.751 Nm/wheel. GREEN on `35596639287`: `ZeroThrottleNoSyntheticDriveTorque` PASS after removing the hidden 18% floor. |
| C-10 | DONE | `PinkCab.Vehicle.ChaosBaseline.Provider.FullCouplingBoundary` now enforces the exact boundary: 0.999 remains partial coupling; only 1.0 is fully coupled. Current bridge uses `FullyCoupledThreshold = 1.0f`. |
| C-11 | DONE | RED on `35597592827`: `+right` produced `right=-3.6 cm`. GREEN focused matrix on `35611762556`: signed runtime DriveSmoke PASS after removing the stale Chaos-boundary inversion. |
| C-12 | BLOCKED_HUMAN | Canonical H-gate sign is automated; OS/HID physical MouseY direction remains a human packaged gate. |
| C-13 | DONE | One canonical GitHub control-plane workflow remains. It is manual-only with `fast` (default focused verification) and `human_gate` (full regression/package/OS-input/delivery) modes. Deprecated H-INP/owner-rejection workflows were removed from the active tree. |
| C-14 | DONE | Jira CD-848 + Confluence page 47 + Git recovery docs use the same R1 ownership semantics. |
| C-15 | DONE | Workflow uses separate status contexts for `fast` and `human_gate`; only `human_gate` may produce a packaged HUMAN_PENDING candidate. Automation scope explicitly denies physical HID inference. |
| C-16 | BLOCKED_TOOLING | Exact-head gate/status exists, but branch/ruleset protection cannot be changed by the currently exposed GitHub connector. Do not claim required-check enforcement until GitHub branch/ruleset configuration is verifiably applied. |


## 2026-09-22 current control-plane state

- Administrative owner: Jira CD-868.
- Mechanics/recovery owner: Jira CD-848.
- Single open implementation PR: #7 → `main`.
- Last runtime-affecting candidate before admin-only cleanup: `8bed51e5c47e037eb45a670f9d6c1c2cdd28b712`.
- Run `35746083559`: code-health/build/focused automation/full automation/package/packaged smoke PASS; packaged Windows OS-input route FAIL at throttle dosing; no HUMAN acceptance.
- Heavy CI is manual-only. Normal iteration uses `fast`; package/delivery runs only under `human_gate`.
- Old PRs #3–#6 are closed historical evidence. Unique superseded-branch material is preserved under `docs/archive/`.

## TDD checkpoint

Historical TDD note: C-06/C-09 were introduced tests-first and their expected RED→GREEN sequence is preserved in evidence. This is no longer a pending production gate.

## Acceptance semantics

- Physics-only smoke proves physics-only behavior.
- PlayerController/InputKey tests prove Unreal input routing, not real Windows HID/capture.
- Packaged smoke proves launch/map/runtime health, not handling acceptance.
- Human acceptance is explicit owner evidence only.


## 2026-09-21 completion hardening

- Ruling: the owner's explicit request to replace `PINKCAB Latest.lnk` with the newest verified candidate supersedes the earlier conservative proposal to keep Latest pinned to the last human-accepted build. The workflow now replaces only `PINKCAB Latest.lnk`; it no longer deletes unrelated `PINKCAB*.lnk` shortcuts.
- Ruling: packaged Windows SendInput evidence is an OS/window input-path proof, not physical HID proof. It may close packaged routing evidence but cannot close C-05/C-12 or HUMAN acceptance.
- Added explicit packaged OS-input route, rendered screenshot evidence, 50 interaction-session cleanup cycles, audio-enabled launch, and focused `HealthyThirtyMinutes` simulation evidence to the canonical exact-head workflow.
- C-16 remains BLOCKED_TOOLING until repository ruleset/required-check administration is actually writable and verified.


### 2026-09-21 packaged L3 diagnostic continuation

- Exact packaged Windows input at `a0854d3` proved menu, ignition, handbrake manipulation, Q clutch, H-gate first, E+wheel 25% throttle and engaged gear 1 through the real packaged window, but the vehicle remained effectively stationary.
- The next pass is diagnostic only: trace world pause/delta, rigid-body awake state, actor velocity, wheel contacts, Chaos current/target gear, RPM and rear drive/brake torque before changing gameplay physics.
- Compile-only telemetry qualifier defect from `c8749b1` was corrected at `e199787`; no control/physics behavior was changed by that correction.
