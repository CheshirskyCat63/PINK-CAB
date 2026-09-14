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
