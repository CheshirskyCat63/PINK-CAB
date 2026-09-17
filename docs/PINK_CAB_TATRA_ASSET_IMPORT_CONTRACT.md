# PINK CAB В· Tatra Asset Import Contract

Date: 2026-09-17
Gate: pre-model handoff for `CD-855`
State: SPECIFIED вЂ” presentation/import contract only; no final model claim

## Purpose

This contract is the last technical boundary before hero-Tatra model integration. Final art may replace presentation assets, but must not rewrite native Chaos physics, input semantics, cockpit StableIds, Vehicle Health, load/CG, fare/economy or persistence authority.

The canonical hero identity remains the bespoke early / Gen-1 Tatra 603-family interpretation in `docs/AUTHORITY.yaml`. A Tatra 613 asset may be used only as a licensed donor / placeholder presentation source and must not silently redefine the canonical silhouette or mechanics.

## Unreal coordinate and scale contract

- Unreal units: `1 UU = 1 cm`.
- Source asset must be converted to centimeters before final import; runtime actor-scale compensation is forbidden for production art.
- Forward axis: `+X`.
- Right axis: `+Y`.
- Up axis: `+Z`.
- Vehicle root origin: stable chassis datum on the longitudinal centerline; do not use an arbitrary DCC scene origin.
- Final root transform in the vehicle pawn is identity unless an explicit presentation profile records a donor-only corrective transform.
- Negative scale / mirrored hierarchy is forbidden. Fix handedness in DCC or import conversion, not in gameplay transforms.

## Locked chassis references

- Wheelbase reference: `2750 mm` / `275 cm`.
- Front track reference: `1485 mm` / `148.5 cm`.
- Rear track reference: `1400 mm` / `140 cm`.
- Tire visual family: tall-sidewall `185R15`-like.
- Steering inner-lock target: `41 В± 2 deg`.
- Rear-engine / RWD identity and current Chaos wheel roles remain authoritative.

## Required presentation decomposition

At minimum the imported presentation must be separable/bindable as follows, whether by separate meshes, bones, or deterministic presentation bindings:

- exterior/body shell;
- cabin/interior shell;
- four road wheels with correct individual centers;
- steering wheel;
- clutch, brake and throttle pedals;
- gearbox lever;
- handbrake lever;
- ignition control;
- turn-signal control;
- horn control;
- lights / wipers / washer controls where visible;
- taximeter and dashboard/warning presentation;
- passenger-door presentation and physical cabin lever;
- rear-view, left and right mirrors.

The existing 22 `EPinkCabCockpitSlot` StableIds are semantic authority. Art binds to those anchors; it does not rename, reorder or replace them.

## Pivots and moving parts

- Wheel pivots: exactly at each wheel rotation center; local rotation axis consistent across all four wheels.
- Steering wheel pivot: physical column center; steering animation must not orbit around an arbitrary mesh origin.
- Pedal pivots: hinge/rocker location appropriate to the visible pedal mechanism.
- Gear lever pivot/path: authored around the visible lever base; presentation path may change without changing selected-gear semantics.
- Handbrake pivot: physical hinge at the base.
- Passenger doors: pivots at their actual hinge axes. Opposed-opening passenger-side geometry must preserve the canonical pillarless continuous aperture.
- Mirrors and dashboard controls may use static presentation bindings where no moving geometry is required.

## Physics and collision separation

- The visible donor/final mesh is presentation-only.
- Native Chaos skeletal chassis / movement component remains sole trajectory, grip, suspension and drivetrain authority.
- Do not make imported body geometry the new gameplay physics owner merely because it contains collision.
- Production collision is authored explicitly and kept simple/bounded; decorative cavities, trim and cabin detail must not create dense physics collision.
- Presentation profile changes must not alter authoritative mass, COM, wheel locations, suspension tuning, health state or snapshot schema.

## Materials, normals and visibility

- Import with stable smoothing/normals; repair inverted normals and mirrored text in source/DCC, never by negative actor scale.
- Keep material slots finite and mergeable; avoid per-trim unique material proliferation.
- First-person owner visibility may use separate cabin/exterior visibility intent through the presentation profile.
- Physics chassis may be hidden when a valid exterior presentation is present, but it remains the underlying physics authority.

## Import acceptance before binding

1. Root orientation and scale are correct with actor scale `1,1,1`.
2. Wheelbase/track and four wheel centers are measured and documented.
3. No mirrored handedness, reversed markings, inverted steering presentation or negative scale remains.
4. Required moving-part pivots are correct.
5. 22/22 cockpit StableIds remain registered after presentation binding.
6. Driver camera remains active and cockpit controls remain reachable/readable from first person.
7. `PinkCab.Vehicle`, `PinkCab.Cockpit` and relevant persistence tests remain green.
8. Clean UE 5.8.2 build and packaged Win64 smoke remain green.
9. Donor/final visual swap requires no gameplay/Chaos rewrite.

Only after these checks may the asset be treated as an integrated presentation candidate. Visual quality approval and final hero-model approval remain separate art gates.
