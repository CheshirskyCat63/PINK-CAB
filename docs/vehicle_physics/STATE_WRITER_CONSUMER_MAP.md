# PINK CAB — PHY-002 State Writer / Consumer Map — 2026-09-26

**Task:** PHY-002 · State-writer inventory  
**Audit base:** `main@7fcb7a13e8c0121edcc36f35066711ab7dbb15a6`  
**Machine guard:** `scripts/ci/verify_vehicle_physics_writers.py`  
**Manifest:** `docs/vehicle_physics/STATE_WRITER_INVENTORY.csv`

## Authority chain

| Domain | Semantic/store authority | Physical/Chaos writer | Primary consumers | Classification |
|---|---|---|---|---|
| Ignition / engine state | `FPinkCabCockpitState` owns Off/Running/Stalled; router requests Start/Stop; `VehicleControlRuntime` requests Stall | `FPinkCabChaosCockpitBridge::Apply` is the only writer allowed to enable mechanical sim | Control runtime, drivetrain condition, Chaos cockpit bridge, UI/telemetry | One authority plus two **disable-only** health safety cuts |
| Engine RPM | Chaos mechanical simulation owns physical RPM; no production direct RPM setter was found | none in project code | DynamicsProvider telemetry → ControlRuntime `LastEngineRpm` → gearbox/drivetrain/UI | Read-only project-side observation |
| Engine torque/config | `FPinkCabChaosPhysicalProfile` owns versioned MaxTorque/RPM/idle/brake/rev-inertia and ratios | Chaos engine via final throttle; partial-clutch external torque currently derived in `FPinkCabChaosCockpitBridge` and applied only by DynamicsProvider | drivetrain condition, gearbox validation, telemetry | P02 must unify partial/full torque path; inventory prevents a third path |
| Steering | `FPinkCabVehicleControlRuntime` composes steering target; health service may only clamp unavailable capability | `FPinkCabChaosVehicleDynamicsProvider::ApplyControls` → `SetSteeringInput` | Chaos, telemetry/UI | Single final physical writer |
| Throttle / service brake | input frame → control runtime → health/drivetrain composition | DynamicsProvider → `SetThrottleInput` / `SetBrakeInput` | Chaos, telemetry | Single final physical writer |
| Handbrake | control runtime / handbrake actuator owns analog command | native Chaos handbrake bool is explicitly held false; rear wheel brake torque is written by DynamicsProvider | Chaos rear wheels, telemetry | Analog torque path is explicit; no parallel native handbrake actuation |
| Gear request / engagement | `FPinkCabGearboxController` owns requested/engaged state; runtime routes requests and controlled restore | `FPinkCabChaosCockpitBridge` alone writes `SetTargetGear` | Chaos transmission, UI/telemetry | Request != engagement preserved |
| Partial-clutch rear drive torque | Cockpit bridge derives value into `FPinkCabVehicleControlState` | DynamicsProvider alone writes per-wheel `SetDriveTorque` | rear Chaos wheels / telemetry | Known P02 blocker, fully inventoried |
| Brake torque | analog handbrake command in control state | DynamicsProvider alone writes rear per-wheel `SetBrakeTorque` | rear Chaos wheels / telemetry | No second per-wheel brake writer found |
| Mass / CoM | `FPinkCabVehicleLoadState` owns logical fuel/crew/passenger load | `FPinkCabChaosLoadBridge` owns runtime `Movement.Mass`, CoM override and `UpdateMassProperties` | Chaos chassis; snapshots/service consumers | PhysicalProfile mass is initialization seed only; load bridge overwrites it in constructor/BeginPlay and on load changes |
| Health / wear / thermal | `FPinkCabVehicleHealthState` owns bounded state | no direct Chaos health writer; HealthService/DrivetrainCondition/RepairService are sanctioned mutation clients | capability clamps, UI, persistence, drivetrain | Existing heat model remains P07 work; inventory only |
| Persistence restore | snapshot codec reconstructs logical health/load; pawn state applies with rollback on failed load sync | load restore reaches Chaos only via `SyncLoadToChaos`; health effects reach Chaos through normal capability sync | runtime state / UI | No direct persisted input/force/velocity writer |

## Multiple-writer cases resolved

### Mechanical simulation

`FPinkCabChaosCockpitBridge` is the authoritative writer and may set mechanical simulation from semantic engine Running state.  
`PinkCabChaosTatraPawnCockpit.cpp` and `PinkCabChaosTatraPawnState.cpp` contain only **post-authority safety-disable** calls when engine capability is lost; neither path can enable mechanical simulation.

### Mass

`FPinkCabChaosPhysicalProfile::ApplyToMovement` seeds `Movement.Mass` during construction. Immediately afterward the pawn initializes logical load and calls `SyncLoadToChaos`; `FPinkCabChaosLoadBridge` becomes the runtime authority for total mass and longitudinal CoM. BeginPlay and subsequent load/persistence changes re-enter only through the same load bridge.

## Side-path audit

PHY-002 RED initially found one dormant private method, `AlignInitialPresentationToGround()`, that:
- teleported the whole vehicle with `AddActorWorldOffset(...TeleportPhysics)`;
- zeroed chassis linear velocity;
- zeroed chassis angular velocity;
- had no production call site.

The method and declaration were removed as dead recovery code. The machine guard now fails on any scoped:
- `AddForce*`;
- `AddImpulse*`;
- `AddTorque*`;
- direct linear/angular velocity setter;
- actor teleport/location writer.

The same guard also fails if Chaos `TorqueControl`, `TargetRotationControl`, or `StabilizeControl` is assigned anything other than `false`.

## Consumer-only surfaces

These are intentionally not manifest writers:
- `PinkCabChaosTatraPawnGateTelemetry.cpp` reads current/target gear, engine RPM, wheel drive/brake torque and vehicle velocity;
- `PinkCabChaosTatraPawnDriver.cpp` projects RPM, clutch, gear and health temperatures to presentation/UI;
- `FPinkCabChaosVehicleDynamicsProvider::ReadTelemetry` reads Chaos wheel/engine state into `FPinkCabVehicleTelemetry`;
- persistence archive/codec reads and serializes durable state but physical reapplication is routed through the authorities above.

## PHY-002 acceptance

PHY-002 is acceptable only when the exact-head guard reports:
- no unknown writer groups;
- no stale manifest groups;
- no writer count mismatch;
- no forbidden force/impulse/torque/velocity/teleport side path;
- no non-false Chaos assist control;
- exact Windows build and focused physics tests remain green.

PHY-002 does **not** tune the vehicle. It freezes the writer topology so PHY-003 telemetry and P01/P02 bug fixes can identify the first causal writer instead of masking symptoms.
