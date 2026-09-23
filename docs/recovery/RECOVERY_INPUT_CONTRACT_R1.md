# PINK CAB · Recovery Input Contract R1

**Contract revision:** RECOVERY-R1-INPUT-2026-09-23  
**Owner umbrella:** CD-848  
**Status:** **CURRENT · OWNER-ACCEPTED WORKING BASELINE**  
**Accepted runtime SHA:** `8168d72406af6934ab20eace583c2b895f0620b7`  
**Accepted run:** `35809749568`  
**Integration:** PR #7 merged to `main` as `865e8f77dde4af1f5c5bee8d49754628b6494db4`

This contract records the interaction behavior accepted by the owner on 2026-09-23. It supersedes contradictory earlier R1 wording that made RMB a mandatory prefix for lever motion.

## 1. Mouse ownership

| State | Contract |
|---|---|
| Default | Mouse XY owns steering. |
| Space held | Mouse owns gaze/look. Releasing Space returns it to steering. |
| Quick 1/2/3/4 held | Temporary quick target/prompt is visible. Recall alone never actuates. |
| Quick key released | The quick-access prompt disappears immediately. |
| RMB held | RMB may capture/retain one valid contextual/selected control. RMB alone never actuates it. |
| LMB contextual action | LMB may directly execute the authored action on the current/contextual target without RMB first. |
| Wheel contextual action | Wheel may directly execute the authored incremental action without RMB first. |
| Lever manipulation | While Gearbox/Handbrake manipulation is active, mouse XY belongs only to that lever; steering holds its current command. |
| Manipulation ends | Mouse XY returns to steering. An optional RMB retain may remain until RMB release. |

One physical mouse sample must never steer and manipulate a lever at the same time.

## 2. Quick access

Keys:

- 1 = TurnSignals
- 2 = Horn
- 3 = Gearbox
- 4 = Handbrake

Rules:

- quick access is selection/staging, never hidden actuation;
- its visible prompt is key-held only;
- releasing the number key removes the quick prompt;
- a control already captured/manipulated may finish its active interaction without a ghost quick prompt;
- release order must not create duplicate commits or delayed actions.

## 3. RMB / LMB / wheel

RMB is **optional capture/retain**, not a global permission key.

- RMB alone never presses a button, moves a lever or changes a rotary value.
- LMB can directly activate momentary/contextual controls such as Horn/Ignition/Taximeter where authored.
- LMB can directly begin Gearbox/Handbrake manipulation when that lever is the active/contextual target.
- RMB can retain Gearbox/Handbrake so the player can hold that target across interaction phases.
- wheel can directly operate a target that declares wheel/incremental support.
- Gearbox itself remains mouse-XY H-gate manipulation rather than wheel shifting where the current control spec disables wheel.
- pedal wheel routing is separate and single-recipient with priority **E → W → Q**.

## 4. Wheel response

Isolated wheel detents remain precise.

A sustained same-direction burst accelerates progressively so long adjustments do not require excessive repetitive scrolling.

Acceleration resets when:

- the burst pauses beyond the configured window; or
- direction reverses.

The response must never duplicate one physical notch through both analog and discrete UE wheel representations.

## 5. Steering character

The car has **no hydraulic or electric power steering**.

Required transfer character:

- standstill: heavy and slow; large mouse travel is required;
- initial/low-speed rolling: substantially lighter than standstill;
- rising speed: response becomes calmer/stabler rather than more nervous;
- high speed must not gain extra target sensitivity or response rate;
- mouse right means physical steering/right trajectory direction; no recurring inversion;
- gaze or lever ownership holds existing steering rather than resetting it.

## 6. Gearbox

H-gate topology:

| Position | Left | Center | Right |
|---|---:|---:|---:|
| top | 1 | 3 | 5 |
| cross-gate | N | N | N |
| bottom | 2 | 4 | R |

Rules:

- changes physically traverse the neutral corridor;
- requested gear and engaged gear are separate;
- actual engagement goes through the common mechanical validator;
- clutch/load/speed mismatch may refuse or damage a shift;
- release edges never perform a second commit;
- Q may stage Gearbox while clutch remains independently held;
- Q does not steal another active retained control.

## 7. Pedals / launch

- Q = clutch.
- W = brake.
- E = throttle.
- E+wheel doses throttle.
- W+wheel doses brake.
- Q+wheel adjusts clutch-release timing.
- If E/W/Q overlap, one wheel step has one recipient: E, then W, then Q.
- A genuine new launch from standstill requires a fresh throttle dose.
- One held launch attempt is not repeatedly reset by speed jitter.
- No hidden auto-throttle or auto-rev-match.

## 8. Cleanup / cancellation

Focus loss, menu transition or explicit transient reset must:

- end temporary target/capture/action ownership;
- release held momentary actions;
- prevent delayed/duplicate gear commits;
- preserve truthful persistent mechanical state such as engaged gear or handbrake position.

## 9. No hidden assists

The accepted control layer does not introduce:

- auto-countersteer;
- yaw rescue;
- ESP;
- ABS;
- trajectory correction;
- auto-throttle;
- auto-rev-match.

## 10. Baseline evidence

Owner-accepted working build:

- source SHA: `8168d72406af6934ab20eace583c2b895f0620b7`;
- run: `35809749568`;
- successful job: `PINK-CAB code-only standalone human build`;
- delivery: `E:\CHESHIRE_DIVISION\Builds\PINKCAB\CODEONLY_8168d72_RUN35809749568`;
- desktop shortcut: `PINKCAB Latest.lnk`.

This acceptance freezes the current working interaction baseline. It does not assert that every later FIRST EURO feature or QA item is finished.
