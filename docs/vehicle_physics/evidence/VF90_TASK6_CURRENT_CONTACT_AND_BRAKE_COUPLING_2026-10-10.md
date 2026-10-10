# VF90 Task 6 - Current native wheel contact and unresolved brake/clutch boundary

Status: IN PROGRESS / NOT TASK 6 DONE / DO NOT MERGE. Date: 2026-10-10, Europe/Sofia.
Branch: feat/vf90-task6-actuation, draft GitHub PR #73, base main ed18a57be5c03e35acee252f74c8f7e8ae51d1e8.
Accepted installed build 6edea7747d3a8433188c9fb394b98ae9c320d49b remains untouched.

## Review corrections with bounded proof
- P1: copied native wheel response uses this-frame contact; native suspension force with WheelLoadRatio and RestingForce; current physical material; preserves previous surface friction if material is absent; zero contact load at liftoff. No force or velocity is applied to the real chassis or wheel during candidates.
- P2: gearbox power-direction prediction, wheel work and final actual native torque use identical efficiency factors; a zero-power crossing is evaluated with a passive factor 1.0. No new tyre/engine/yaw solver.
- UE5.8.3 compiler PASS; focused contact/direction Unreal tests 2/2 PASS; code health 0 baseline regressions (Saved/VF90/t6-matfix-*). No change to clutch 25-RPM tolerance, friction, braking or tuned mass.

## Reproduced remaining Task 6 blocker
The actual native wheel brake torque remains nonzero immediately after a command requesting brake=0 and handbrake=0 during the manual reverse release/acceleration transition. Recorded at Saved/VF90/t6-brake-branch-cockpit-1.log, from real current Chaos state:
- Reverse ratio -12.80000019, transmission efficiency approx 0.9; engine 96.865776 rad/s; clutch capacity 390 Nm; dt 0.01666670 s.
- Each rear wheel had 18,333,326 native centimetre torque units of brake (~1833.33 Nm), contact true. This may be an ordinary smoothed native brake transition; the observed prepared input command zero is NOT proof of a stale or unowned brake actuator.
- For clutch candidates 286.458181143 and 286.458227634 Nm (difference 0.000046491 Nm), the native gearbox shaft jumped from -3.64841657 to 241.05488640 rad/s. The native Chaos joint reaction jumped from +389.99999504 to -389.99990272 Nm. Residuals +103.5418 and -676.4581 Nm straddle a discontinuity: no accepted zero of the current coupled equation was found.
- Epic Chaos Wheeled native code uses brake-vs-drive selection inside FSimpleWheelSim. Historical public source mirrors illustrate it but are not byte-for-byte UE5.8.3 verification.

A diagnostic one-pass native-joint prototype failed 6/9 targeted physical tests and produced +2077.8 J energy gain. Reverted exactly from preserved backup Saved/VF90/t6-before-native-single-step-prototype-20261010.cpp.txt (SHA256 224811580b2e8023e7144c2c2c093291527046f5626a94e164a3e065ba99e9e9). Temporary diagnostics were removed. These failures cannot be hidden by relaxing tolerances, force clamps, grip edits or brake-zeroing.

## Exact current required runtime result
Saved/VF90/t6-matfix-required45-20261010.log: mandatory Task 6 test selection 45 total, **44 PASS / 1 FAIL**. The only failure is PinkCab.Cockpit.Playable.Runtime after real brake-to-reverse transition. This is a product failure, not a missing automation test. Targeted two-review-case run Saved/VF90/t6-matfix-focus2-20261010.log: **2/2 PASS** on this same source. UnrealBuildTool Saved/VF90/t6-matfix-build-20261010.log: PASS. Code health Saved/VF90/t6-matfix-code-health.log: zero baseline regressions.
## No administrative closure yet
Review P1/P2 corrections are narrower than the whole Task 6. The nonconvergent mixed brake/clutch scenario blocks technical closure and PR merge even if independently selected green tests pass. Full mandatory and combined controls suites must be reported with their exact source and failures, never summed across builds. No Tasks 7-9, new accepted package or owner verdict. Jira CD-659/CD-648 and Confluence program 22413538 remain accountable for the blocker.
## Combined and administration checks on this candidate
Saved/VF90/t6-matfix-combined-20261010.log: combined engine/controls/actuation matrix **106/106 PASS**. It is distinct from, and does not supersede, the required **44/45 FAIL**.
Saved/VF90/t6-matfix-final-scripts.log: Python test discovery **58/58 PASS**.
Saved/VF90/t6-matfix-final-code-health.log: repository hygiene PASS, baseline regressions **0**.
Saved/VF90/t6-matfix-final-scope.log: vehicle-feel scope **PASS**.
git diff --check PASS. No rejected native one-pass experimental coupling or temporary diagnostic marker is staged.

### Exact local proof hashes (SHA256)- Saved\VF90\t6-matfix-required45-20261010.log : 2b292c1b0ab09485f307877529f2e6e9cd563c4ca61ccb3ff86716a29d64f2fe
- Saved\VF90\t6-matfix-combined-20261010.log : 78e46b609f386715775e699711c4954abbcced6ddcb6c11bd13bed8342613074
- Saved\VF90\t6-matfix-focus2-20261010.log : c4bb7fe20c10703af45483ca823f08916377374bee64e47eaf6195bca67cc6d4
- Saved\VF90\t6-matfix-final-scripts.log : 8e56bbd25af8e728c25cd38af5a85d42f1669dee59ad3fa0d9b24b8961cdef3a
- Saved\VF90\t6-matfix-final-code-health.log : 5d5d10e92099465c444a74f2604242322f8fac8083ad289bd5c278cd6180c883
- Saved\VF90\t6-matfix-final-scope.log : 8f3875e44d400b2fc27b2a7b6c9ac31313bffbc2f8dc51d66983d7f955c4a27e
- Saved\VF90\t6-matfix-build-20261010.log : 646c111f7b1b0f57331dbd6379f61c3f4d096bf2915727067ce0264e96fcbc2a
