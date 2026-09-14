# CD-828 · Canonical Playable Cockpit Runtime Evidence

Branch: `feat/CD-823-cockpit-playable-code-health`

## Contract

- possessed `APinkCabChaosTatraPawn` in `L_PinkCab_ChaosWeave`
- active first-person driver camera
- ignition and handbrake actuated through physical cockpit interaction events
- throttle produced through canonical semantic `E` -> `FPinkCabVehicleInputFrame`
- Space gaze suppresses steering; release restores steering ownership
- transient cleanup does not teleport or reset the vehicle
- no player-facing `R` semantic action

## RED

Initial runtime test failed to compile because `APinkCabChaosTatraPawn` had no shared production `ApplyVehicleInputFrame` boundary.
After the boundary was added, the first physical run exposed a spawn/road-contact race rather than a drivetrain defect.
The test now starts its bounded drive window only after at least two wheels report road contact.

## GREEN

- `PinkCab.Cockpit.Playable.Runtime`: 1/1 PASS
- stability repeats: 3/3 PASS
- `PinkCab.Cockpit.Input.Compliance`: 12/12 PASS
- `PinkCab.Core.CodeComplete.Runtime`: 1/1 PASS
- representative runtime: 1112.8 cm travel, 33.26 km/h, gear 1, 2771.6 rpm, throttle 1.00, handbrake 0.00

## Task 9 release verification

- default map corrected from obsolete `L_PinkCab_ContractZero` to verified `L_PinkCab_ChaosWeave`
- post-config `PinkCab.Cockpit.Playable.Runtime`: PASS; representative run 1666.3 cm / 41.61 km/h
- post-config `PinkCab.Core.CodeComplete.Runtime`: PASS
- clean `PinkCabEditor Win64 Development`: 27/27 build actions, `Result: Succeeded`
- final regression: Cockpit 26/26; Interaction 8/8; Vehicle 34/34; Taxi 50/50; Persistence 32/32; Service 32/32; World 26/26; Vertical 31/31; Core.CodeComplete 6/6; Core 17/17; all exit 0 / fail 0
- `scripts/build.ps1 -Package`: `BUILD SUCCESSFUL`, AutomationTool `ExitCode=0`; 511 packages cooked
- packaged exe launched with no map override and loaded `/Game/Dev/Maps/L_PinkCab_ChaosWeave`
- packaged process remained responsive; no fatal/assert/unhandled exception observed
- accepted package mirrored to stable `PINK-CAB/Artifacts/Package/Windows`; 51/51 files match by relative path/size and key exe/pak/utoc SHA-256 hashes match
- `Desktop/PinkCab.lnk` resolves to stable accepted bootstrap exe and was smoke-launched successfully
