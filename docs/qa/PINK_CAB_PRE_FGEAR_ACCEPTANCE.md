# PINK CAB PRE-FGear Acceptance — PF-15

Date: 2026-09-12
Jira: CD-770
Certified implementation commit: `4b0dc3a6d6d8ae112475b866ce602b180c3115e8`
Certified base main before PF-15: `e50cec3106a6ce0db43417e80264cbcfe3bd6ba5`

## Verdict

**PASS — vendor-independent PRE-FGEAR foundation is complete.**

PF-00 through PF-15 are executable and verified without a paid plugin dependency. The next production road-dynamics blocker is the FGear provider slot; no temporary Chaos/custom hero solver is present.

## Environment

- Engine: Unreal Engine 5.8.2
- Engine CL: 56702186
- Compatible CL: 55116800
- Branch: `++UE5+Release-5.8`
- Platform: Win64 / Windows SDK 10.0.22621.0
- Build identity schema: `bootstrap-v1`
- Build channel: `pre-fgear`
- Runtime version: `0.0.1-pre-fgear`
- Determinism fixture identity: `CityCode=EURO-63`, `GeneratorVersion=gen-1`, `ContentSetVersion=content-1`
## Fresh verification

A clean GitHub clone at `e50cec3106a6ce0db43417e80264cbcfe3bd6ba5` was created under a new temporary path and built using only the canonical entrypoint `scripts/build.ps1`: **PASS**.

After the PF-15 packaging fix, the certified feature tree was rebuilt and verified again:

- canonical build: PASS / exit 0
- all `PinkCab.*` automation: **79 discovered / 79 Success / 0 Fail / exit 0**
- authority guard: PASS
- `git diff --check`: PASS
- full automation leaves the canonical L1 map unchanged; the generator is now idempotent when the saved map already exists
- Development package via `scripts/build.ps1 -Package`: PASS / AutomationTool exit 0
- packaged executable: `Artifacts/Package/Windows/PinkCab.exe`
- packaged executable SHA-256: `7098E1E497CF1D1536C557318DE923155A9773502539CEAC3ED8960479C5CF9B`

## Canonical map evidence

Fresh-clone map loads were executed for:

- `/Game/Dev/Maps/L_PinkCab_ContractZero`
- `/Game/Dev/Maps/L_PinkCab_L1_GreyboxZero`

Both emitted **MapCheck 0 Error(s), 0 Warning(s)**. The command-line editor required external termination after the successful `MAP CHECK; Quit` log marker; this is recorded as a runner-exit quirk, not a map failure. The automation suite independently loads the L1 map and executes MapCheck successfully.
## Paid dependency / vendor gate

- project `Plugins/` directory: absent
- runtime `PinkCab.Build.cs` dependencies: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`
- production Build.cs vendor-module hits for FGear / VDS / ChaosVehicle: **0**
- production source hero-provider implementation: **0**
- `docs/ASSET_LICENSE_LEDGER.csv`: header only, **0 acquired asset rows**
- PRE-PURCHASE spend: **0**

The strings `pre-fgear` in build identity are lifecycle labels, not vendor dependencies.

## PF-15 defect found and closed

The first packaging smoke exposed a real build-graph defect: `PinkCabTests` was declared as a `DeveloperTool` project module, so the non-editor game target attempted to instantiate `UnrealEd` through the test module and packaging failed with `RulesError`.

PF-15 changed `PinkCabTests` to an Editor-only project module. A second QA defect was also closed: `PinkCab.Editor.GenerateL1GreyboxZero` no longer re-saves an already-present canonical map, preventing full-suite CI runs from dirtying the repository.

After both fixes, canonical build, 79/79 automation, package, authority guard and diff hygiene all pass.

## Gate result

PRE-FGEAR package count: **16 / 16 complete = 100%**.

This document certifies the free/vendor-independent foundation only. It does not certify FGear or Vehicle Damage System integration, because neither paid plugin is installed. PF-16 must keep the provider slot empty until an official evaluation or owner-approved purchase is available.
