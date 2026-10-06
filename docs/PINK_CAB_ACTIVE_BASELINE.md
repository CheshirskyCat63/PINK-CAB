# PINK CAB · Active Baseline

## Purpose

The active baseline is deliberately small: a playable P4 Tatra on a good straight road with a maintainable build and delivery path.

## Runtime

- UE 5.8
- stock Chaos wheeled-vehicle simulation
- Tatra 613 visual shell retained
- physical cockpit retained
- H-pattern retained as PINKCAB control/gameplay logic
- steering, throttle, brake and parking brake retained
- taxi/game modules retained
- default map: `/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight`

The custom P00–P04 driveline research stack is frozen history, not production runtime authority.

## Road

The project does not require a road-authoring framework for the current P4 gate.

MetaRoad 3.2.0 may provide frozen free content for the baked road, but:

- no MetaRoad source framework is required by PINKCAB runtime code;
- no MetaRoadEditor dependency is required by PINKCAB tests;
- no StructUtils dependency is part of the active project contract;
- R2/R3/R4/R5 road-authoring workflows are retired.

## Verification

Canonical workflow: `.github/workflows/verify.yml`.

Required layers:

- script tests;
- code-health;
- repo hygiene;
- `PinkCabEditor` build;
- focused runtime acceptance covering Tatra assets/presentation, forward/reverse Chaos motion, H-gate, cockpit, live state and endless-road continuity.

## Delivery

Canonical workflow: `.github/workflows/deliver.yml`.

Canonical implementation: `scripts/deliver.ps1`.

Delivery is exact-head, smoke-tested and atomically published to the desktop `PINCKCAB` entry.

## Repository policy

- `main` is the only integration truth.
- One active feature/cleanup branch at a time is preferred.
- Historical experiments are tags/closed PRs, not living architecture.
- Old evidence remains recoverable from `archive/20261006/*`.
