# PINK CAB FGear Entry Gate

Date: 2026-09-12
Jira: CD-771 / PF-16
Base SHA: `cfc8992f4b8c9825bef9b52b248d840dcca03d5e`
Decision state: `READY_FOR_OWNER_PURCHASE`

## Proven prerequisite

PF-00..PF-15 are complete. Exact-SHA fresh checkout builds, packages, and passes all 79 `PinkCab.*` tests. The project has zero paid plugin dependencies and no production FGear/VDS/ChaosVehicle module dependency.

The runtime adapter already exists as `IPinkCabVehicleDynamicsProvider`. With no approved provider installed it explicitly reports `NoProvider`, refuses control application, and refuses to fabricate telemetry. No fallback production solver is allowed.

## Sole next road-dynamics blocker

Authority assigns hero Tatra production road dynamics to FGear Vehicle Physics. All vendor-independent input, mass/load/CG, telemetry, Vehicle Health, wallride request, taxi, persistence and world boundaries are ready. The remaining executable road-dynamics step requires an approved FGear provider implementation.
## Legal zero-cost evaluation path

Official FGear Fab listing provides demo links, documentation, videos and an Example Project for evaluation:
`https://www.fab.com/listings/8904fcf9-debf-416c-aefa-4b2d125e46b7`

The developer's Unreal forum guidance states that the UE5 Example Project requires the installed `FGearPlugin` folder to be copied into `FGearExample/Plugins`; therefore the Example Project is not a free plugin distribution:
`https://forums.unrealengine.com/t/fgear-vehicle-physics-v1-4/128793?page=62`

Free/legal work before purchase is limited to the official packaged demos, documentation, videos and Example Project inspection that does not extract or redistribute plugin binaries. No public installable trial plugin was verified by this gate.

VDS remains deferred. Its official Fab listing offers a playable demo and states tested FGear compatibility, but VDS is not the current blocker:
`https://www.fab.com/listings/1c629d9e-044e-439b-873c-0b27f8e65902`

## Purchase boundary

Do not purchase automatically. Owner approval is required before any paid transaction. On approval: acquire FGear through Fab, install the official UE-compatible plugin, implement only the adapter, rerun the full suite and start Tatra dynamics calibration. VDS remains unpurchased until permanent deformation is independently proven as the next blocker.
