# VF90 Task 4 — authored Tatra cabin parity — 2026-10-08

Status: **TECHNICALLY VERIFIED; administrative convergence pending at creation time.**

Scope is Task 4 from `docs/superpowers/plans/2026-10-07-tatra-ready.md`: inventory authored bones and existing gameplay actions, bind approved RIG06 cabin presentation to authoritative runtime state, and verify neutral/mid/full travel plus normal input paths. This receipt does not close Task 5 mass/CoM/inertia, Task 6 Chaos actuation, handling calibration, commercial provenance, or final owner acceptance.

## Exact identities

- Task-3 administrative close: `f003917`.
- Exact Task-4 implementation: `3efab779901d2355964d4062ee6d30b347426235`.
- Branch: `feat/tatra-ready-runtime`.
- Installed owner-accepted fallback remains `6edea7747d3a8433188c9fb394b98ae9c320d49b`; it was not replaced.
- V24 FBX: `E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\export\TATRA613_RIG24_UE\TATRA613_CHAOS_RIG_06_TEXTURED_OPENABLES.fbx`.
- V24 FBX SHA-256: `6943EEBA8FE70C2B55CAF6FD295A2E1F819132154D0D4A8D5796218A0DFAC1DC`.
- V24 export report SHA-256: `20EC101B441957E796F4B75DA7F68601D0D7D12DB1A6E67FBF48FD355C553320`.

## V24 authoring boundary

V24 derives from the accepted V23 source and adds semantic binding only for four existing source instrument-needle islands:

- `Cabin_SpeedometerNeedle`;
- `Cabin_TachometerNeedle`;
- `Cabin_FuelNeedle`;
- `Cabin_TemperatureNeedle`.

The four bones are authored from the existing `t613_needle_*` source pivots and existing meshes are weighted 100% to their corresponding semantic bone. No replacement needle geometry is generated.

The V24 authoring report proves:

- geometry digest unchanged: **true**;
- all pre-existing bone transforms unchanged: **true**;
- export objects: **213**;
- bones: **78** (V23 74 + four semantic instrument bones);
- required bones missing: **0**.

The active RIG06 profile/test contract now explicitly inventories the relevant authored presentation bones: root, physical wheels, steering wheel, gearbox, three pedals, handbrake, horn, left/right stalks, radio, climate, four doors, four windows, front luggage lid, rear hood, two mirrors and four instrument needles.

## Presentation ownership and no hidden mechanics

Task 4 does not create another vehicle-mechanics owner.

Authoritative values continue to come from the existing control/cockpit/load/telemetry state. RIG06 only consumes those values and moves authored presentation bones.

No Task-4 diff touches `Source/PinkCabVehicle`, the Chaos physical profile, the dynamics provider, vehicle-control runtime, tyre/grip/suspension, mass or CoM.

The RIG06 full-car skeletal asset is the same asset and transform for exterior and cabin. Task 4 reuses one poseable full-car component rather than maintaining two identical pose copies. D3D12 cockpit/exterior/side captures and the unchanged playable gate verify that this optimization preserves presentation while removing duplicated per-frame bone work.

The original `PinkCab.Cockpit.Playable.Runtime` test remains unchanged. Exact Task-4 runtime diagnostics remain healthy:

- forward: travel **59.2 cm**, speed **8.12 km/h**, gear **1**, rpm **3618.4**, throttle **1.00**, handbrake **0.00**;
- reverse: travel **136.9 cm**, speed **-6.92 km/h**, gear **-1**, throttle **1.00**.

## Normal gameplay/state paths verified

The executable Task-4 cabin-parity test verifies existing normal paths rather than direct test-only pose writes:

- handbrake through `ApplyPhysicalControlMouseDelta("Handbrake", ...)`;
- turn signals through the normal cockpit interaction router;
- wiper steps through the normal cockpit interaction router;
- horn press/release through the normal cockpit interaction router;
- passenger-door state through the normal cockpit interaction router;
- H-gate lever through normal physical-control mouse manipulation;
- fuel needle through authoritative vehicle load state;
- pedal presentation through existing displayed clutch/brake/throttle values;
- steering presentation through existing semantic steering state.

Verified authored travel/state:

- handbrake: **0% → 0°**, **50% → 16°**, **100% → 32°**;
- turn-signal stalk: right/left are distinct **±18°** authored states;
- wiper stalk: mode 1 **14°**, mode 2 **26°**;
- passenger door: existing passenger-door state opens/closes both authored right-side donor doors;
- gearbox: physical H-gate manipulation moves the authored lever around its lower shaft pivot;
- fuel needle: empty-to-full span **120°**, 50% fuel at **60°** of that span;
- speedometer, tachometer, fuel and temperature semantic needle bones are all live.

Horn boundary: the normal horn state is verified, but no arbitrary bone motion is invented because the current authored source has no approved horn-travel action. The semantic horn bone remains readable at its authored rest pose.

Radio/climate/mirrors boundary: authored bones are inventoried and required by the RIG06 asset contract, but no new gameplay state or manipulation is invented in Task 4.

## Windows

The canonical cabin parity authority allows side-window winders to remain post-P1 if not required for FIRST EURO input acceptance. Task 4 therefore adds **no new window key/input grammar**.

The plan nevertheless requires authored window presentation. All four authored window bones use the source V22/V23 window motion basis and are executable through the existing bounded RIG06 openable presentation ownership.

Runtime verification covers closed / 50% / full / closed:

- all four windows reach 50%;
- front-window authored exponential half-roll ≈ **4.0661°**;
- rear-window authored exponential half-roll ≈ **0.6532°**;
- all windows travel into the door at half state;
- front full roll ≈ **24.7153°**;
- rear full roll ≈ **25.1639°**;
- full travel exceeds half travel;
- all four return to exact authored rest.

No window collision, grip, road physics or driving command is changed.

## Automated verification

Exact implementation `3efab779901d2355964d4062ee6d30b347426235`:

- repository clean and synchronized with origin before exact verification;
- authoring/tooling suite: **52/52 PASS**;
- vehicle-feel scope guard: **PASS**;
- `git diff --check`: PASS before implementation commit;
- UE Editor build: PASS;
- Task-4 runtime gate: **11/11 PASS**;
- standard focused runtime gate: **9/9 PASS**;
- original unchanged `PinkCab.Cockpit.Playable.Runtime`: PASS;
- fresh D3D12 `PinkCab.Vehicle.Visual.FixedViewCapture`: PASS;
- exact-source package: BUILD SUCCESSFUL / AutomationTool ExitCode 0;
- packaged runtime launch: Exit 0;
- packaged road/material audit: `PINKCAB_ROAD_MATERIAL_AUDIT=PASS materials=PASS textures8k=PASS`.

Evidence hashes:

- `Saved/VF90/task4-exact-script-tests.log`: `CEE8A657362014A9E1BC3F164876D343A68665E56FFCD27AB876710C6E0C7658`;
- `Saved/VF90/task4-exact-scope.log`: `8DDBA7E293792F06EE4EC1DDF6CB48B6EEFCA278106C88491C526913A8C9D2F7`;
- `Saved/VF90/task4-exact-runtime-gate.log`: `98B653B7E18C3B258DC73A6C3390A6D6684A805625065CB4EDB401DF5D596181`;
- `Saved/FocusedAcceptance/focused-runtime.log`: `960DBBD213E113C712223FEFBCD2570C5065A94ABC5087DACB2A2BBAD9FCBED4`;
- `Saved/VF90/task4-exact-d3d12.log`: `59A23562AB399E5F513EA1B48D96BBF27574390F06437F57309468F50A77F9D3`;
- `Saved/VF90/task4-exact-package-build.log`: `90DE45455EE2FBDB2073872B0BD2F50CAB83FD6510DBBAEB5C475D796D184351`;
- package runtime log: `C57FBF24EAA9D392D18C59E223FFCEC661F4861AD38BC4232BB050708E7B8631`.

## D3D12 matched views

Fresh exact-source fixed views:

- cockpit: `2FFA2E1783643B0002121B69608A3852A586FE023218A39AC2158375E2FDD20C`;
- exterior: `3D4103F92BE9F86CA3F19D48F09B745F44A678C0CD1A5AC8857D6CA503C30AC8`;
- side: `078D045FD1BED212EE20867E2699F1C5C13FC427D261422E5AB4189A92FE442D`.

The cockpit, exterior and side views remain correctly composed with the single full-car pose owner; no duplicate presentation is required.

## Exact package

Built from clean exact source `3efab779901d2355964d4062ee6d30b347426235`.

Package hashes:

- `PinkCab.exe`: `94FC74A1C8EF8FF4CC43BF1D50C3ABC2F5977811705D2901080C8E3DC0545DB9`;
- `global.ucas`: `2BB74E1888479E6E8C1C86807771D11D2D75F2B9600685AFDA70C5F181A27ED8`;
- `global.utoc`: `291F607C5212B362BA2D191B7718A380CEB553E6DA482D269ABB6937508017A1`;
- `PinkCab-Windows.pak`: `85ADEFE60BC3D3D995B5C4774392A04B16FBBC250319E0E0F65549A5740BFD2D`;
- `PinkCab-Windows.ucas`: `65E5CC3F44A7256E6516D2043016F74C48BB894FBD5580B6E1E4AB413E84F296`;
- `PinkCab-Windows.utoc`: `22425DEBAA6DAEAD15589C6BBE108348E5886619683059DF53674F05E454F888`.

Package screenshot SHA-256:

`D2BBD674DDD47E4B7351C3D75F688727535BEE6F0DD96F4407A3A4A27FB72C3D`.

The package was launched directly from `Artifacts/Package/Windows`. It was **not** installed over the owner's accepted Desktop fallback.

## Boundary

Task 4 may be marked complete only after Jira/Confluence/Git administrative convergence records this receipt.

Task 5 remains the next stage: wheel geometry/body collision/base+fuel+crew mass/rear-engine CoM/inertia/static sag/contact/load fixtures. Task 6 analog Chaos actuation remains held and must not be applied before Task 5 closes. Commercial donor provenance/public-exposure requirements on CD-855 remain open and are not changed by this internal presentation result.
