# PINK CAB · C-01…C-16 Recovery Execution Ledger

**Updated:** 2026-09-21  
**Branch:** `fix/CD-848-recovery-r1-input-contract`  
**Rule:** DONE means evidence exists on the exact candidate. OPEN/BLOCKED is not rounded up.

| ID | State | Evidence / next proof |
|---|---|---|
| C-01 | DONE | Jira CD-848 description and Confluence page 47 now explicitly adopt Recovery Input Contract R1: RMB grip only; RMB+LMB lever manipulation. |
| C-02 | TEST_PENDING | `PhysicalControlRouting` now extends through movement without direct gearbox/handbrake actuator calls. Ignition remains an explicitly named fixture, so full-cockpit E2E is not claimed. |
| C-03 | TEST_PENDING | Direct-provider drive/reverse tests are renamed under `ChaosBaseline.PhysicsOnly.*`, and GitHub evidence explicitly forbids using them as gameplay-input acceptance. Exact-run compile/evidence still required. |
| C-04 | TEST_PENDING | Added PlayerController→cockpit→runtime→Chaos→movement drivetrain path. Exact-run evidence still required; ignition fixture is excluded from the claim. |
| C-05 | BLOCKED_HUMAN | UE InputKey/PlayerController automation is not OS/HID mouse evidence. GitHub gate records this as HUMAN_PENDING. |
| C-06 | GREEN_PENDING | RED proven on run `35595470850`: raw X=19/Y=-23 became 0/0. Production fix now makes raw device delta authoritative with processed fallback; exact-head rerun required before DONE. |
| C-07 | TEST_PENDING | Physical PlayerController routing now asserts RequestedGear=1 while EngagedGear remains 0 for a no-clutch first-gear request. |
| C-08 | TEST_PENDING | New drivetrain movement assertion reuses the real RMB/LMB handbrake and gearbox path before launch; existing direct-provider Chaos smoke remains physics-only. |
| C-09 | RED_PENDING | Test `PinkCab.Vehicle.ChaosBaseline.Provider.ZeroThrottleNoSyntheticDriveTorque` added before production change. Current 18% idle-governor throttle floor is expected to fail it. |
| C-10 | RED_PENDING | Added `PinkCab.Vehicle.ChaosBaseline.Provider.FullCouplingBoundary`: 0.999 coupling must remain on continuous partial-clutch torque; only 1.0 may hand off to Chaos gear. Current 0.995 threshold is expected to fail. |
| C-11 | TEST_PENDING | Chaos runtime smoke now asserts signed displacement in the initial vehicle-right axis for positive semantic steering; `abs(lateral)` can no longer hide inversion. |
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
