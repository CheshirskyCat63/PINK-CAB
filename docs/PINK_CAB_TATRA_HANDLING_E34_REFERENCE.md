# PINK CAB - Tatra handling reference

**Status:** CURRENT handling goals; reconciled 2026-10-07.
**Owners:** CD-648 handling, CD-641 drivetrain, CD-748 physical vehicle, CD-855 presentation.
**Precedence:** owner-approved Tatra Ready plan, PINK_CAB_VEHICLE_FEEL_90.md, PINK_CAB_CONTROL_MECHANICS_RELEASE_CONTRACT.md, then this companion.

## Current checkpoint

Accepted P4 fallback is accepted/p4-rig06-20261007 at 6edea77. Tasks 0-4 integration is main@c8459ba / PR70. Task5 baseline is RED at 0b395c9; no production physical correction is implied. Historical acceptance is retained; complete appearance, authored cabin motion and enjoyable handling requested on 2026-10-07 are not yet accepted.

Native Unreal Chaos Vehicles is the sole road-dynamics solver. FGear/VDS references in the previous revision are historical research, not implementation instructions.

The current calibration/presentation subject is the accepted Tatra 613 donor. Do not substitute a different body during this work. The separately documented bespoke 603-family future hero identity is not changed by accepting this donor.

## Driving goal

The owner's Forza-like quality goal means readable, enjoyable manual driving: calm straight-line control, progressive response, useful throttle and braking, understandable weight transfer, and rear-wheel-drive slides the player can catch. E34 535i remains a readability reference, not a source of front-engine packaging or donor hardware.

No ABS, TC, ESP, automatic countersteer, yaw rescue, velocity overwrite or force boost may manufacture this behavior. Presentation consumes physical state and cannot change trajectory.

## Parameters and evidence

The executable physical profile and measured runtime state own current values. PinkCabChaosPhysicalProfile.cpp records targets of 250 hp / 260 Nm, 8500 max RPM and 925 idle RPM. These are gameplay profile values, not measured performance or historical factory claims. The reference crew/fuel fixture is 1657 kg; the declared maximum fixture is 2107 kg.

Previous tables of 180 hp, 240 Nm, 2750 mm wheelbase and expression gains are retained in Git history only. Separately, the current project reference-load target remains 45/55 F/R at 1657 kg under the mass/load contract; it is a gameplay design target requiring measured geometry/load justification, not a factory specification or licence to force a test green. They must not override the current 613 measurements or executable profile. Wheel geometry, CoM, inertia and suspension need evidence before handling acceptance.

## Open requirements

- Measure visible wheel centers against physical contacts through suspension travel; reconcile dimensions without silently stretching the authored body.
- Establish rear-engine base CoM, vertical/lateral CoM, inertia and exactly-once fuel/crew/passenger load contributions.
- Measure mouse input, authored command, Chaos wheel angle and visible steering end-to-end, including native speed-dependent steering configuration.
- Close analog clutch/handbrake requirements deliberately within the single Chaos dynamics ownership. Accepted P4 currently uses a neutral/gear clutch threshold and boolean handbrake; this is not proof of continuous actuation.
- Verify useful launch, engine braking, gear/RPM continuity and upper-RPM behavior without repeated unexplained torque interruption.
- Calibrate settling, progressive dry breakaway, steerable braking and wet/load variants only on verified road and physical geometry.
- Validate 30/60/120 FPS and a sustained packaged drive; report requested controls separately from actual actuation.

## Acceptance

Stable road rendering, Blender-to-package appearance parity and visible cabin interaction are required alongside driving behavior. One exact-source packaged candidate must pass technical checks and the owner's visual/driving verdict before world/taxi/traffic/Neural expansion resumes. See docs/superpowers/plans/2026-10-07-tatra-ready.md.
