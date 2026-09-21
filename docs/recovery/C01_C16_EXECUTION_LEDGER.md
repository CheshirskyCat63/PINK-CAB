# PINK CAB · C-01…C-16 Recovery Execution Ledger

**Updated:** 2026-09-21  
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
| C-10 | RED_PENDING | Added `PinkCab.Vehicle.ChaosBaseline.Provider.FullCouplingBoundary`: 0.999 coupling must remain on continuous partial-clutch torque; only 1.0 may hand off to Chaos gear. Current 0.995 threshold is expected to fail. |
| C-11 | DONE | RED on `35597592827`: `+right` produced `right=-3.6 cm`. GREEN focused matrix on `35611762556`: signed runtime DriveSmoke PASS after removing the stale Chaos-boundary inversion. |
| C-12 | BLOCKED_HUMAN | Canonical H-gate sign is automated; OS/HID physical MouseY direction remains a human packaged gate. |
| C-13 | DONE | One push-triggered canonical GitHub G1 workflow on the recovery branch; legacy owner/H-INP workflows are manual hard-failing tombstones. |
| C-14 | DONE | Jira CD-848 + Confluence page 47 + Git recovery docs use the same R1 ownership semantics. |
| C-15 | DONE | Canonical GitHub workflow can publish AUTO technical PASS only and always leaves HUMAN_PENDING; automation scope file explicitly denies OS/HID inference. |
| C-16 | BLOCKED_TOOLING | Exact-head gate/status exists, but branch/ruleset protection cannot be changed by the currently exposed GitHub connector. Do not claim required-check enforcement until GitHub branch/ruleset configuration is verifiably applied. |

## TDD checkpoint

This ledger commit is intentionally **tests first** for C-06/C-09. Production behavior must not be changed until the GitHub UE runner executes these tests and the expected failures are observed.

## Acceptance semantics

- Physics-only smoke proves physics-only behavior.
- PlayerController/InputKey tests prove Unreal input routing, not real Windows HID/capture.
- Packaged smoke proves launch/map/runtime health, not handling acceptance.
- Human acceptance is explicit owner evidence only.
