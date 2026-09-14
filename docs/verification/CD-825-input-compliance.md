# CD-825 · FIRST EURO Cockpit/Input Compliance

**Authority:** `PROJECT_SETUP.md`, `PINK_CAB_ACTIVE_BASELINE.md`, `PINK_CAB_BASE100_TECH_OWNER_PACK_01.md` C01-C08, `PINK_CAB_TATRA_DAUGHTER_CONDUCTOR_CANON.md`, `VERIFICATION_MATRIX.md` PC-T-INP-001..008, Confluence 03/30.

**Rule:** current authority wins over historical START/ATTENTION/GO drafts. Unresolved owner numerics fail closed; code may not invent hidden shortcuts.

## Canon-to-code audit at `c006540`

| Contract | Required behavior | Pre-fix status |
| --- | --- | --- |
| C01 | one semantic raw-KBM router | FAIL: pawn also reads raw `R` reset |
| C02 | vehicle solver receives adapter commands, not raw keys | PASS for Q/W/E/mouse; keep |
| C03 | steering/throttle/brake/clutch/handbrake continuous | FAIL: cockpit handbrake is boolean/toggle |
| C04 | controls declare Grip / Press-Hold / Wheel support | PARTIAL: generic model exists; live cockpit bypasses it |
| C05 | physical controls emit semantic events | PARTIAL: router exists, live ATTENTION/GO path is wrong |
| C06 | Space gaze; 1-4 recall; RMB grip; LMB momentary; wheel contextual | FAIL: live bindings are LMB ATTENTION / RMB GO |
| C07 | one bounded target via trace/query; no Actor scan | FAIL: live pawn has no real gaze target query |
| C08 | semantic rebinding + conflicts + Restore Defaults | PASS baseline; bindings need canonical actions |

## Executable rows

| Verification row | Exact required proof | Fix owner |
| --- | --- | --- |
| PC-T-INP-001 | Space transfers steering -> gaze/target-search -> steering cleanly | interaction component + pawn |
| PC-T-INP-002 | gaze/selection/quick recall never actuates | interaction component |
| PC-T-INP-003 | RMB grip only on grip-capable target | control metadata + interaction component |
| PC-T-INP-004 | LMB momentary only; horn short/long hold observable | interaction component + horn state |
| PC-T-INP-005 | wheel only on wheel-capable target | control metadata + interaction component |
| PC-T-INP-006 | 1/2/3/4 recall signals/horn/gearbox/handbrake only | semantic router + interaction component |
| PC-T-INP-007 | one bounded target, no world scan/autopilot | cockpit-local query |
| PC-T-INP-008 | repeated/focus-loss recovery leaves no stuck transient input | explicit reset-transient contract |

## Additional authority gaps in this gate

- Handbrake must be normalized `0..1`; prototype visuals may quantize/interpolate but gameplay truth is continuous.
- `FPinkCabCockpitSlotDefinition` must carry the interaction/presentation metadata promised by the cockpit spec rather than stable ID only.
- No player-facing `R`/teleport/free-reset path is permitted.
- Prototype meshes/assets remain replaceable presentation only; no gameplay branch may depend on SportsCar/Manny identity.
- Runtime acceptance may not bypass canonical player input by directly applying hidden recovery/control shortcuts.

## Post-fix executable evidence

Candidate verification after canonical-input refactor and physical fare-control correction:

- `PinkCab.Cockpit.Input.Compliance` — **12/12 PASS**, ExitCode 0.
- `PinkCab.Cockpit` — **25/25 PASS**, ExitCode 0.
- `PinkCab.Interaction` — **8/8 PASS**, ExitCode 0.
- `PinkCab.Vehicle` — **34/34 PASS**, ExitCode 0.
- `PinkCab.Vehicle.Cockpit.InteractionRouter` — **1/1 PASS**, ExitCode 0.
- `PinkCab.Cockpit.PrototypeVisual` — **2/2 PASS**, ExitCode 0, mannequin dependency load errors = 0.

Implemented authority mapping:

- `Space` owns gaze while held and returns cleanly to steering on release.
- `1..4` recall signals / horn / gearbox / handbrake without actuation.
- RMB owns grip only for grip-capable controls.
- LMB owns momentary press/hold; horn press duration is observable and cleanup emits release.
- wheel acts only on wheel-capable controls and requires active grip where authored.
- handbrake is continuous `0..1` with 1/64 authored wheel increments.
- passenger-door lever uses grip + signed wheel travel; taximeter START/STOP/RESET is a physical momentary surface.
- gaze selection is bounded to registered cockpit slots; no world Actor scan.
- pause/controller-loss cleanup releases transient ownership and momentary state.
- player-facing `R` reset/teleport and universal `ATTENTION/GO` semantics are absent.
- pawn gathers raw input only; cockpit interaction policy is delegated through `FPinkCabCockpitInteractionFrame` / `UPinkCabCockpitInteractionComponent::ProcessFrame`.
