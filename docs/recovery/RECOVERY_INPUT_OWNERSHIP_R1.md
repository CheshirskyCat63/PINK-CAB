# PINK CAB · Recovery R1 Input / Mechanical Ownership Map

**Revision:** RECOVERY-R1-OWNERSHIP-2026-09-20  
**Historical baseline:** 34a1937f9ac66b771f8eba813619c01f9ac706e5  
**Companion contract:** docs/recovery/RECOVERY_INPUT_CONTRACT_R1.md
**Current control plane:** `docs/CONTROL_PLANE.md` · PR #7 → `main` · CD-848 / CD-868

This map closes REC-005 for the current recovery lane. One concern has one authoritative writer. Adapters may translate data but must not create a second state owner.

| Concern | Authoritative owner | Allowed boundary / reader | Forbidden duplicate |
|---|---|---|---|
| Raw mouse/key sampling | PinkCabInteraction · FPinkCabPlayerInputAdapter | APinkCabChaosTatraPawn composition consumes semantic sample | Raw input reads in Vehicle/Taxi/Cockpit domain logic |
| Semantic key binding | PinkCabInteraction · FPinkCabSemanticInputRouter | UI may display the same bindings | Ad-hoc key checks in vehicle mechanics |
| Gaze/quick recall/selected target | UPinkCabCockpitInteractionComponent | Driver UI reads current target for prompt | HUD-owned target memory |
| RMB physical grip | UPinkCabCockpitInteractionComponent | Pawn composition reads active target | Vehicle runtime inferring grip from mouse delta |
| LMB lever manipulation | UPinkCabCockpitInteractionComponent | Pawn passes manipulation-active to vehicle runtime | RMB directly interpreted as mechanical movement |
| Q ephemeral Gearbox stage | UPinkCabCockpitInteractionComponent via bGearboxStageFromClutchHeld | Pawn decides whether staging is allowed | Reusing/latching quick-slot 3 state |
| Momentary LMB action | UPinkCabCockpitInteractionComponent event stream | FPinkCabCockpitInteractionRouter applies authored momentary/toggle controls | Generic LMB = GO path |
| Pedal held state + wheel recipient | FPinkCabVehicleControlRuntime / FPinkCabPedalDosingController | Pawn passes one wheel sample | Cockpit control and pedal both consuming one notch |
| Launch throttle target | FPinkCabLaunchController | Pedal dosing reads/writes through runtime | Immediate hard-coded E throttle shortcut |
| Steering command | FPinkCabSteeringController | Pawn supplies mouse delta; Chaos provider consumes command | Cockpit/Chaos independently changing sign or angle |
| Gear lever/request | FPinkCabGearboxController | VehicleControlRuntime exposes requested gear | UI or release edge writing selected gear directly |
| Actual engaged gear | common gearbox engagement validator/controller | Chaos bridge consumes engaged gear | H-gate detent or mouse release bypassing validator |
| Handbrake analog mechanical state | FPinkCabHandbrakeActuator | Cockpit state/HUD mirror value; Chaos consumes brake command | bool handbrake or separate UI latch |
| Vehicle health / damage | PinkCabVehicle health service/binding | runtime mechanics update through defined service | input layer repairing/resetting damage |
| Chaos input writes | PinkCabChaosVehicleDynamicsProvider | vehicle runtime/bridge calls provider | direct Set*Input outside provider |
| Prompt rendering | UPinkCabDriverUiComponent | reads interaction/runtime snapshot only | prompt text becoming the state owner |

## Mechanical write path

`PlayerInputAdapter → CockpitInteractionComponent → Pawn composition → VehicleControlRuntime → Gearbox/Handbrake/Steering owners → Chaos provider`.

For Gearbox/Handbrake specifically:

`RMB grip state` is **not** a mechanical write permission.  
`RMB + LMB manipulation state` is the only high-level permission that forwards mouse XY to the lever mechanical owner.

## Cancellation invariants

Focus loss, system menu, pawn teardown and explicit transient reset must:
- clear gaze / quick held / Q-stage / grip / manipulation / momentary transient ownership;
- cancel pending gearbox request through the existing runtime reset policy;
- never synthesize a gear commit;
- preserve legitimate persistent mechanical state unless the existing mechanic explicitly defines a moving-control auto-return.

## Analyzer boundary

Config/CodeHealthPolicy.json remains authoritative for structural ownership:
- raw input is confined to PinkCabInteraction;
- Chaos Set* writes are confined to PinkCabChaosVehicleDynamicsProvider;
- module dependency cycles / allowlist debt remain zero-gate conditions.

No recovery fix may create a second input router, second vehicle solver or second save format.
