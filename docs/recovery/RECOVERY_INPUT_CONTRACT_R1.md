# PINK CAB · Recovery Input Contract R1

**Contract revision:** RECOVERY-R1-INPUT-2026-09-20  
**Owner umbrella:** CD-848  
**Source baseline:** `34a1937f9ac66b771f8eba813619c01f9ac706e5`  
**Recovery authority:** `PINKCAB_RECOVERY_TO_WORKING_100_SOL56.md`, REC-002 / REC-011…REC-017  
**Status:** IMPLEMENTATION CANDIDATE · automated verification required · human feel remains pending

This amendment resolves the RMB/LMB conflict for the current recovery lane. It supersedes only contradictory wording that allowed RMB+XY alone to manipulate the gearbox or handbrake.

## 1. Mouse ownership phases

| Phase | Gearbox / Handbrake contract |
|---|---|
| Default | Mouse XY owns steering. |
| Select / stage | 3 or 4 recalls the physical control. Q may stage Gearbox while operating clutch. Recall/staging does not actuate the control. |
| RMB grip | RMB acquires/retains exactly one selected physical control. RMB alone does not move the lever and does not take mouse XY away from steering. |
| RMB + LMB manipulation | LMB begins explicit lever manipulation while RMB remains held. Mouse XY now belongs only to the captured lever. Steering receives no new mouse delta; the existing steering command is held, not reset. |
| LMB release with RMB still held | Manipulation ends immediately. The grip may remain, but mouse XY returns to steering and cannot continue moving the lever. |
| RMB release | Grip ends. A consumed quick-recall target becomes ineligible for later ghost input. Persistent mechanical state (gear request/engagement, handbrake position) is not cleared merely to hide UI. |

One mouse sample must never steer and manipulate a lever at the same time.

## 2. Gearbox commit contract

- H-gate topology remains 1/3/5 top, 2/4/R bottom, through the neutral corridor.
- Lever/request movement exists only during RMB+LMB manipulation.
- Reaching a detent updates the requested gear through the existing gearbox controller.
- Actual engagement remains owned by the existing common mechanical validator.
- LMB release and RMB release do **not** perform a second gear request/commit.
- Standstill N→gear without valid clutch/load synchronization remains refused by the validator; no shortcut bypass is added.
- Q stages Gearbox but never grips/manipulates it and must not steal another active grip.

## 3. Handbrake contract

- 4 stages Handbrake.
- RMB retains the handbrake without moving it.
- RMB+LMB+mouse manipulates the existing analog handbrake value.
- LMB release stops further lever movement; the mechanical handbrake position remains where the player left it.
- Stationary parking-latch and moving hydraulic behavior remain owned by the existing handbrake actuator; this contract changes only input ownership.

## 4. Momentary and other controls

- LMB remains the authored action button for true momentary controls such as Horn/Ignition.
- LMB without RMB does not manipulate Gearbox/Handbrake.
- RMB grip alone never fires a momentary action.
- Wheel routing remains single-recipient with pedal priority E > W > Q.
- Space gaze remains independent; focus/menu cancellation clears transient grip/action state without committing a gear.

## 5. Target / prompt lifecycle

Selection, recalled target, active grip, active manipulation, momentary action, and displayed prompt are separate concepts.

After a recalled target has actually been consumed by grip/action/wheel and the interaction session ends:
- it must no longer remain eligible for a later ghost click;
- Gearbox/Handbrake prompt must disappear after all related input is released;
- actual gear/handbrake mechanical state remains truthful and separately displayed.

A quick recall that was selected but not yet consumed may remain available for the intended tap-recall behavior.

## 6. Mandatory regression scenarios

- RV-011: RMB-only grip does not move Gearbox/Handbrake; steering remains responsive.
- RV-012: LMB without RMB does not manipulate a lever; RMB+LMB does.
- RV-013: simultaneous/reordered releases create one session and no duplicate commit.
- RV-015: another target cannot steal an active session.
- RV-016/RV-017: consumed Gearbox/Handbrake target and prompt clear after complete release.
- RV-021: Q+E+wheel keeps clutch down and doses throttle.
- RV-023: Q does not steal an active handbrake/other grip.
- RV-041: no-clutch mismatch is refused by the common validator.

No later code or test may reinterpret RMB as direct lever manipulation without a new explicit owner amendment.
