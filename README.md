# PINK CAB

First-person Tatra taxi / arcade-sim project built in Unreal Engine 5.8.

## Current development authority

- Repository: `CheshirskyCat63/PINK-CAB`
- Integration branch: `main`
- Owner-accepted fallback: `accepted/p4-rig06-20261007` / `6edea77`
- Pre-model vehicle / asset handoff gate: `CD-856` — DONE; internal RIG06 presentation accepted, public provenance/release disposition remains under `CD-855`
- Immediate execution priority: `Vehicle Feel 90` before broader FIRST EURO world/gameplay work
- Runtime vehicle authority: stock Unreal Chaos Vehicles
- Game-specific vehicle layer: PINKCAB cockpit, physical H-pattern, controls, health/state and Tatra presentation
- Current world target: `/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight`

The old P00–P04 custom driveline, causality/calibration laboratory and road-authoring CI are historical evidence, not current production architecture.

## Build

```powershell
pwsh -NoProfile -File scripts/build.ps1
```

Package:

```powershell
pwsh -NoProfile -File scripts/build.ps1 -Package
```

## Verify

GitHub: `.github/workflows/verify.yml`

Local focused runtime:

```powershell
pwsh -NoProfile -File scripts/verify-runtime.ps1
```

The focused gate covers the Tatra asset/presentation contract, stock-Chaos forward/reverse driving, H-pattern, cockpit, live vehicle state and endless-road continuity.

## Deliver

GitHub manual workflow: `.github/workflows/deliver.yml`

Implementation: `scripts/deliver.ps1`

Delivery requires:

1. clean exact source HEAD;
2. editor build;
3. focused runtime acceptance;
4. BuildCookRun package;
5. packaged smoke;
6. atomic `PINCKCAB_BUILD` publication;
7. update of `PINCKCAB.lnk` only after all gates pass.

## Road content

The P4 gate does not require a road-authoring framework.

MetaRoad 3.2.0 is retained only as frozen free content needed by the baked straight-road presentation. MetaRoad source/editor modules and StructUtils are not active P4 dependencies.

## History

Previous vehicle research and delivery evidence remains recoverable from Git and the `archive/20261006/*` tags. PR #64 is deliberately historical and unmerged.

Historical evidence is not an instruction to restore retired systems.

## Start here

- [Project setup](docs/PROJECT_SETUP.md)
- [Active baseline](docs/PINK_CAB_ACTIVE_BASELINE.md)
- [Control plane](docs/CONTROL_PLANE.md)
- [Engineering start](docs/ENGINEERING_START_HERE.md)
- [Program roadmap](docs/PROGRAM_ROADMAP.md)
- [Vehicle Feel 90](docs/PINK_CAB_VEHICLE_FEEL_90.md)
