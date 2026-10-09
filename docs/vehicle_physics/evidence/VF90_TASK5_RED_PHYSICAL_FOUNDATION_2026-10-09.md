# VF90 Task 5 — RED baseline / physical foundation — 2026-10-09

**Status: RED. Implementation not accepted. Owner-accepted P4 fallback not modified.**

## Source and evidence
- Branch: feat/vf90-task5-physical-foundation.
- Exact source baseline: c8459ba125ee70093f0ae4c82017e9c8c849199b (merged PR #70, Tasks 1–4).
- Unreal 5.8.3 Native Chaos; profile PINKCAB_TATRA613_CHAOS.
- Engine acceptance command: Automation RunTests PinkCab.Vehicle.PhysicalFoundation.BaselineAudit, PIE L_PinkCab_L1_EndlessStraight.
- RED test: Source/PinkCabTests/Private/Vehicle/PinkCabTatraPhysicalFoundationRuntimeTests.cpp.
- Log: Saved/VF90/task5-red-baseline-20261009.log, SHA256 40EC67F3E8CF0E5C1F4AFA996BFCF2B207861420BB21FB3CA5EEA931984E47FE.
- Editor Win64 Development compilation: PASS.
- Runtime: Automation Result={Fail}, 4 physical acceptance assertions fail. Shell exit code 0 despite test failure; NEVER count shell exit as test PASS.

## Measured deficiencies
- Physics skeletal mesh: /Game/Vehicles/SportsCar/SKM_SportsCar, NOT an authored Tatra physics mesh.
- Physics collision: /Game/Vehicles/SportsCar/PA_SportsCar, 5 bodies and 4 constraints, NOT a Tatra body.
- Root and Movement mass: 1657 kg; aggregate mesh mass 1924.377 kg is not the chassis mass.
- Live CoM in component space: (0, 0, 0), not rear-engine/rear-heavy.
- Four wheels show contact; static rear spring share 0.451691, front 0.548309; reference accepted balance 45/55 F/R at 1657 kg.
- Inertia exists but arises from the template geometry, not measured/validated Tatra collision.
- V24 Blender source wheelbase 310.70129 cm is intentionally transformed to effective 298.0 cm by approved X presentation scaling; four visible/physical wheel contacts had already passed Task 1. Do NOT rescale donor/model to force physics acceptance.

## Strict exit
1. Native Chaos remains sole solver; project-owned authored Tatra physical skeletal rig and PhysicsAsset envelope; actual four wheel contacts.
2. Versioned base chassis/rear engine/fuel/heroine/daughter/passenger mass moments, applied exactly once, including real load variation.
3. Measured reference 45/55 F/R from physically defensible mass distribution (never alter tire friction or add yaw control for a static-load test).
4. Physics-derived inertia and static sag under empty, reference and loaded fixtures; determinism and package evidence.
5. RED tests must become GREEN on same exact-source head; link tested code, build logs, asset provenance and Jira/Confluence before Task5 closure.

Jira CD-648 owns Task5, CD-855 visual/provenance; CD-559 owns CI. Task6 steering/actuation remains held until Task5 passes. The 6edea774 installed fallback, old working trees and 32-file backup are preserved.
