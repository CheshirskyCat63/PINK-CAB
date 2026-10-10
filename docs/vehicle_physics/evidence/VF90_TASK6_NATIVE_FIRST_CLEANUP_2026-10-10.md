# VF90 Task6 - native-first cleanup and enforced blockers / 2026-10-10

Status: cleanup implemented and verified; actual partial clutch remains OPEN/RED. This is NOT Task6 completion, a new physical solver, or permission to migrate the Tatra silently.

## Exact identity and change boundary

Implementation/config-reference/test-selection commit: `be80247f468b91ae6df3551b0c29181d01e85744`, based on `46dd3ce542b04a6af892b9f440be7bd19a5f7d28`, in draft PR73. The editor and tests were executed on the working contents subsequently captured by that commit. Main remains Task5 `ed18a57`; accepted installed source `6edea7747d3a8433188c9fb394b98ae9c320d49b` remains untouched. No package or installation was produced.

Removed the uncalled FPinkCabClutchDrivelineModel and its analytical integration implementation from compiled Source (four obsolete files). Search found no production/test caller of the solver. The FPinkCabClutchDrivelineConfig declaration and validation body were extracted to config-only files and compared verbatim against the prior commit. All calibration fields, values, validation, physical profile, mass, collision, steering, tyre and suspension parameters are preserved. The architecture registry now points to the real provider contract/implementation and actual actuation test family; the engineering entry explicitly marks partial actuation missing. Historical source remains in Git, not copied back into another active solver.

Both known failures now belong to the SAME mandatory verify-runtime.ps1 list: PartialClutchTransfer and ClutchCapability. The original23 tests are retained. A small regression test first failed when these were excluded, then passed after inclusion. A separate cleanup check first failed while the retired solver remained compiled, then passed. No workflow, protection bypass or allowlist expansion was introduced.

## Native capability evidence, not a new architecture decision

The installed standard Chaos FSimpleEngineSim::SetEngineRPM documents that it matches engine/wheel RPM without clutch simulation. This agrees with Epic's API documentation:
https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/ChaosVehiclesCore/FSimpleEngineSim/SetEngineRPM

Epic documents a native Clutch module in the separate Chaos Modular Vehicle plugin; its ClutchStrength parameter controls shaft-velocity matching. It is not a hidden setting on UChaosWheeledVehicleMovementComponent:
https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-modular-vehicles-quickstart

The installed UE5.8 distribution also contains AModularVehicleSkeletalPawn and the official BP_ModularVehicleSimplifiedSkeletalSetup example. Thus a native skeletal path exists; claiming that Modular invariably requires replacing the car with geometry collections would be incorrect. Existence is not compatibility or complete-car acceptance.

Two isolated SDK characterizations were run before considering adoption:
- Native core engine/clutch/transmission/wheel chain: neighboring native modules and a real gear were exercised. Engine torque reached the clutch but downstream transfer was not demonstrated in this fixture. Its finite-output instrumentation PASS is NOT a physical-clutch PASS; native startup condition messages were retained.
- Official skeletal sample/map: the first run lacked the Clutch input and logged a physics-prediction prerequisite warning. Session-local input/prediction configuration and the original possessed sample pawn made it drive. These settings were never saved to project config/assets. However full-disengagement torque, bounded partial capacity and wheel-to-engine energy response were NOT demonstrated. Sequential movement alone, including motion at an open-clutch input, is insufficient proof. Raw sample EngineTorque units were not validated and are NOT labelled Nm.

No Modular dependency, pawn, plugin enablement, force/velocity write or torque multiplier was integrated into PINKCAB. Both research sources were removed from Source and retained with logs under ignored Saved/VF90. They are not acceptance tests and are not shipped. The earlier archived research remains historical evidence. No new self-written clutch equation was added.

## Executed verification

| Check | Actual result |
|---|---|
| Editor build after solver removal | PASS |
| Canonical complete current selection | 25 executed:23 PASS,2 FAIL |
| Exact failed test names | PinkCab.Vehicle.Actuation.PartialClutchTransfer; PinkCab.Vehicle.Input.ClutchCapability |
| Previously passing physical/cabin/road/load/handbrake/neutral/coast checks | All original23 PASS in the same run |
| Script suite after cleanup and ownership sync | 58/58 PASS |
| Code health / hygiene / authority / scope / whitespace | PASS, zero code-health violations |
| Retained config declaration and validation vs46dd3ce | Exact comparison PASS |

The canonical RED is deliberate truthful enforcement of an existing missing mechanic, not a new runtime regression or a waived failure. No readiness percentage is computed from these counts. Previous hosted run37995986320 on46dd3ce finished SUCCESS, but excluded the two clutch blockers and does not certify this change or complete Task6. Final-head hosted status/review remains separately observable on draft PR73.

## Integrity / local resumable evidence

Paths below are relative to C:/workspace/pinkcab-vf90-runtime-20261007. SHA256:
- Saved/VF90/t6-required25-20261010.log: a24347da2d5ce12601c0570ecb0f62b75a889a4f12b9ae3b9e738b5920d05b11
- Saved/VF90/t6-clean-native-base-build-20261010.log: 20c03a2dd1575e7aad6df079a4a7fb015ec29d02ffb6bd18b1316d9b87151d88
- Saved/VF90/t6-epic-configured-20261010.log: b1e8a5b4020da9ec9f8765908cd2533df63dda27fdd4ac5b3cfec4f58da55fa3
- Saved/VF90/t6-native-chain-20261010.log: 2e54778536fc7f6239058e498c3f211da901b20b6c594d02df7f885a73dd058c
- Saved/VF90/epic-skeletal-clutch-probe-20261010.cpp.txt: 568bfc5888af809dfa74b1b308a6d4198f9f870035be672f7d32bc9a1ca08210
- Saved/VF90/native-clutch-chain-probe-20261010.cpp.txt: 5c2c463309144b654d4dc57f35d5bd47dcd880fb6645d48c78dbdd748f5e1c8f
- Final static output: Saved/VF90/t6-clean-native-static-final-20261010.log.

## Remaining ordered work

First prove the native clutch in its intended complete setup: fully open isolation, actual partial torque, shaft reaction/energy, manual gear and no-assist behavior. Only then may a coherent integration proposal identify every changed provider/physics/persistence boundary. Using Modular is a separate runtime choice, NOT coefficient tuning; it has not been adopted. Do not restore the retired solver, fake coupling with throttle scaling, remove the two failures or mix independent propulsion owners.

Task6 remains incomplete including combined thermal/focus/parking/FPS requirements. Tasks7-9 stay held. Existing handbrake/neutral/coast fixes and Task5 remain preserved. No fresh owner build or completed administrative/technical Task6 gate is claimed.
